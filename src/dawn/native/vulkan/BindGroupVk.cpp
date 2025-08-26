// Copyright 2018 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/vulkan/BindGroupVk.h"

#include <utility>
#include <vector>

#include "dawn/common/Enumerator.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Range.h"
#include "dawn/common/ityp_stack_vec.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/vulkan/BindGroupLayoutVk.h"
#include "dawn/native/vulkan/BufferVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/SamplerVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

// static
ResultOrError<Ref<BindGroup>> BindGroup::Create(
    Device* device,
    const UnpackedPtr<BindGroupDescriptor>& descriptor) {
    Ref<BindGroup> bindGroup;
    DAWN_TRY_ASSIGN(
        bindGroup,
        ToBackend(descriptor->layout->GetInternalBindGroupLayout())->AllocateBindGroup(descriptor));
    DAWN_TRY(bindGroup->Initialize(descriptor));
    return bindGroup;
}

BindGroup::BindGroup(Device* device,
                     const UnpackedPtr<BindGroupDescriptor>& descriptor,
                     DescriptorSetAllocation descriptorSetAllocation)
    : BindGroupBase(this, device, descriptor), mDescriptorSetAllocation(descriptorSetAllocation) {}

BindGroup::~BindGroup() = default;

MaybeError BindGroup::InitializeImpl() {
    DAWN_TRY(InitializeStaticBindings());

    if (HasDynamicArray()) {
        DAWN_TRY(InitializeDynamicArray());
    }

    SetLabelImpl();
    return {};
}

MaybeError BindGroup::InitializeStaticBindings() {
    const auto* layout = ToBackend(GetLayout());

    // Now do a write of a single descriptor set with all possible chained data allocated on the
    // stack if possible. We need to preallocate the vectors to avoid reallocation that would
    // invalidate the pointers chained in `writes`.
    // TODO(https://crbug.com/438554018): Use Vulkan's descriptor set update template so as to need
    // a single allocation, and one that could be reused at the layout level.
    const uint32_t bindingCount = static_cast<uint32_t>((GetLayout()->GetBindingCount()));
    ityp::stack_vec<uint32_t, VkWriteDescriptorSet, kMaxOptimalBindingsPerGroup> writes(
        bindingCount);
    ityp::stack_vec<uint32_t, VkDescriptorBufferInfo, kMaxOptimalBindingsPerGroup> writeBufferInfo(
        bindingCount);
    ityp::stack_vec<uint32_t, VkDescriptorImageInfo, kMaxOptimalBindingsPerGroup> writeImageInfo(
        bindingCount);

    uint32_t numWrites = 0;
    auto AddWrite = [&](BindingIndex bindingIndex) -> std::pair<size_t, VkWriteDescriptorSet*> {
        const BindingInfo& bindingInfo = layout->GetBindingInfo(bindingIndex);
        size_t writeIndex = numWrites;
        numWrites++;

        auto& write = writes[writeIndex];
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.dstSet = GetHandle();
        // Arrays all have a single binding, so compute the binding index for the array, which is
        // the same as the binding index for the 0th element.
        write.dstBinding = uint32_t(bindingIndex - bindingInfo.indexInArray);
        write.dstArrayElement = uint32_t(bindingInfo.indexInArray);
        write.descriptorCount = 1;
        write.descriptorType = VulkanDescriptorType(bindingInfo);

        return {writeIndex, &write};
    };

    // Loop over bindings for each binding type. Skip over already destroyed handles as it produces
    // a VVL error. The descriptor set will have null entries, which is invalid to use, but we'll
    // never do that since the WebGPU command buffers will be errors.
    // TODO(https://crbug.com/438554019): Instead, consider replacing the handles with placeholder
    // handles, to skip over branches and later allow for the use of Vulkan descriptor update
    // templates.

    for (BindingIndex i : layout->GetBufferIndices()) {
        BufferBinding binding = GetBindingAsBufferBinding(i);

        VkBuffer handle = ToBackend(binding.buffer)->GetHandle();
        if (handle == VK_NULL_HANDLE) {
            continue;
        }

        auto [writeIndex, write] = AddWrite(i);
        writeBufferInfo[writeIndex].buffer = handle;
        writeBufferInfo[writeIndex].offset = binding.offset;
        writeBufferInfo[writeIndex].range = binding.size;
        write->pBufferInfo = &writeBufferInfo[writeIndex];
    }

    for (BindingIndex i : layout->GetNonStaticSamplerIndices()) {
        Sampler* sampler = ToBackend(GetBindingAsSampler(i));

        auto [writeIndex, write] = AddWrite(i);
        writeImageInfo[writeIndex].sampler = sampler->GetHandle();
        write->pImageInfo = &writeImageInfo[writeIndex];
    }

    for (BindingIndex i : layout->GetSampledTextureIndices()) {
        TextureView* view = ToBackend(GetBindingAsTextureView(i));

        VkImageView handle = view->GetHandle();
        if (handle == VK_NULL_HANDLE) {
            continue;
        }

        auto [writeIndex, write] = AddWrite(i);
        // TODO(crbug.com/41488897): Add GetVkDescriptorSet{Index, Type}(BindingIndex) functions to
        // BindGroupLayoutVk that access vectors holding entries for all BGL entries and eliminate
        // this special-case code in favor of calling those functions to assign `dstBinding` and
        // `descriptorType` above.
        // TODO(https://crbug.com/438554018): Alternatively take advantage of the precomputed
        // descriptor update template to do set this up once in the layout and have it be
        // transparent in the BindGroup.
        if (auto samplerIndex = ToBackend(GetLayout())->GetStaticSamplerIndexForTexture(i)) {
            // Write the info of the texture at the binding index for the sampler.
            write->dstBinding = static_cast<uint32_t>(samplerIndex.value());
            write->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }

        writeImageInfo[writeIndex].imageView = handle;
        writeImageInfo[writeIndex].imageLayout =
            VulkanImageLayout(view->GetFormat(), wgpu::TextureUsage::TextureBinding);
        write->pImageInfo = &writeImageInfo[writeIndex];
    }

    for (BindingIndex i : layout->GetStorageTextureIndices()) {
        TextureView* view = ToBackend(GetBindingAsTextureView(i));

        VkImageView handle = VK_NULL_HANDLE;
        if (view->GetFormat().format == wgpu::TextureFormat::BGRA8Unorm) {
            handle = view->GetHandleForBGRA8UnormStorage();
        } else {
            handle = view->GetHandle();
        }
        if (handle == VK_NULL_HANDLE) {
            continue;
        }

        auto [writeIndex, write] = AddWrite(i);
        writeImageInfo[writeIndex].imageView = handle;
        writeImageInfo[writeIndex].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        write->pImageInfo = &writeImageInfo[writeIndex];
    }

    for (BindingIndex i : layout->GetInputAttachmentIndices()) {
        TextureView* view = ToBackend(GetBindingAsTextureView(i));

        VkImageView handle = view->GetHandle();
        if (handle == VK_NULL_HANDLE) {
            continue;
        }

        auto [writeIndex, write] = AddWrite(i);
        writeImageInfo[writeIndex].imageView = handle;
        writeImageInfo[writeIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        write->pImageInfo = &writeImageInfo[writeIndex];
    }

    Device* device = ToBackend(GetDevice());
    // TODO(https://crbug.com/42242088): Batch these updates
    device->fn.UpdateDescriptorSets(device->GetVkDevice(), numWrites, writes.data(), 0, nullptr);

    return {};
}

MaybeError BindGroup::InitializeDynamicArray() {
    // This backend only supports DynamicArrayKind::SampledTexture at the moment.
    DAWN_ASSERT(GetLayout()->GetDynamicArrayKind() == wgpu::DynamicBindingKind::SampledTexture);

    // Write only the entries that have bindings present, the availability buffer will prevent
    // reading entries that aren't written to and may contain garbage.
    // TODO(crbug.com/435251399): Instead of bespoke initialization, handle the creation like any
    // other updates to the dynamic array, so as to have a single code path handling updates.
    std::vector<VkDescriptorImageInfo> imageWrites;
    std::vector<uint32_t> arrayElements;

    auto bindings = GetDynamicArrayBindings();
    for (auto [i, view] : Enumerate(bindings)) {
        if (view != nullptr) {
            VkDescriptorImageInfo imageWrite = {};
            imageWrite.imageView = ToBackend(view)->GetHandle();
            imageWrite.imageLayout =
                VulkanImageLayout(view->GetFormat(), wgpu::TextureUsage::TextureBinding);
            imageWrites.push_back(imageWrite);
            arrayElements.push_back(uint32_t(i));
        }
    }

    std::vector<VkWriteDescriptorSet> writes;
    for (size_t i = 0; i < imageWrites.size(); i++) {
        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = GetHandle(),
            .dstBinding = uint32_t(GetLayout()->GetDynamicArrayStart()),
            .dstArrayElement = arrayElements[i],
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &imageWrites[i],
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr,
        };
        writes.push_back(write);
    }

    Device* device = ToBackend(GetDevice());
    device->fn.UpdateDescriptorSets(device->GetVkDevice(), writes.size(), writes.data(), 0,
                                    nullptr);

    return {};
}

void BindGroup::DestroyImpl() {
    BindGroupBase::DestroyImpl();
    ToBackend(GetLayout())->DeallocateDescriptorSet(&mDescriptorSetAllocation);
}

void BindGroup::DeleteThis() {
    // This function must first run the destructor and then deallocate memory. Take a reference to
    // the BindGroupLayout+SlabAllocator before running the destructor so this function can access
    // it afterwards and it's not destroyed prematurely.
    Ref<BindGroupLayout> layout = ToBackend(GetLayout());
    BindGroupBase::DeleteThis();
    layout->DeallocateBindGroup(this);
}

VkDescriptorSet BindGroup::GetHandle() const {
    return mDescriptorSetAllocation.set;
}

void BindGroup::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mDescriptorSetAllocation.set, "Dawn_BindGroup",
                 GetLabel());
}

}  // namespace dawn::native::vulkan
