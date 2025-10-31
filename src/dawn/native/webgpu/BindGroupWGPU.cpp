// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/native/webgpu/BindGroupWGPU.h"

#include <vector>

#include "absl/container/inlined_vector.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/native/webgpu/BindGroupLayoutWGPU.h"
#include "dawn/native/webgpu/BufferWGPU.h"
#include "dawn/native/webgpu/CaptureContext.h"
#include "dawn/native/webgpu/ComputePipelineWGPU.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/RenderPipelineWGPU.h"
#include "dawn/native/webgpu/SamplerWGPU.h"
#include "dawn/native/webgpu/TextureWGPU.h"

namespace dawn::native::webgpu {

namespace {

WGPUBindGroupEntry ToWGPU(const BindGroupEntry* entry) {
    return {
        .nextInChain = nullptr,
        .binding = entry->binding,
        .buffer = entry->buffer == nullptr ? nullptr : ToBackend(entry->buffer)->GetInnerHandle(),
        .offset = entry->offset,
        .size = entry->size,
        .sampler =
            entry->sampler == nullptr ? nullptr : ToBackend(entry->sampler)->GetInnerHandle(),
        .textureView = entry->textureView == nullptr
                           ? nullptr
                           : ToBackend(entry->textureView)->GetInnerHandle(),
    };
}

class ComboBindGroupDescriptor {
  public:
    explicit ComboBindGroupDescriptor(const BindGroupDescriptor* desc) {
        mDesc.nextInChain = nullptr;
        mDesc.label = ToOutputStringView(desc->label);
        mDesc.layout = ToBackend(desc->layout->GetInternalBindGroupLayout())->GetInnerHandle();
        mDesc.entryCount = desc->entryCount;
        for (uint32_t i = 0; i < desc->entryCount; ++i) {
            mEntries.push_back(ToWGPU(&desc->entries[i]));
        }
        mDesc.entries = mEntries.data();
    }

    const WGPUBindGroupDescriptor* Get() const { return &mDesc; }

  private:
    WGPUBindGroupDescriptor mDesc;
    absl::InlinedVector<WGPUBindGroupEntry, 8> mEntries;
};

}  // namespace

// static
ResultOrError<Ref<BindGroup>> BindGroup::Create(
    Device* device,
    const UnpackedPtr<BindGroupDescriptor>& descriptor) {
    Ref<BindGroup> bindGroup =
        ToBackend(descriptor->layout->GetInternalBindGroupLayout())->AllocateBindGroup(descriptor);
    DAWN_TRY(bindGroup->Initialize(descriptor));
    return bindGroup;
}

BindGroup::BindGroup(Device* device, const UnpackedPtr<BindGroupDescriptor>& descriptor)
    : BindGroupBase(this, device, descriptor),
      RecordableObject(schema::ObjectType::BindGroup),
      ObjectWGPU(device->wgpu.bindGroupRelease) {
    ComboBindGroupDescriptor desc(*descriptor);
    mInnerHandle =
        ToBackend(GetDevice())
            ->wgpu.deviceCreateBindGroup(ToBackend(GetDevice())->GetInnerHandle(), desc.Get());
    DAWN_ASSERT(mInnerHandle);
}

MaybeError BindGroup::InitializeImpl() {
    return {};
}

void BindGroup::DeleteThis() {
    // This function must first run the destructor and then deallocate memory. Take a reference to
    // the BindGroupLayout+SlabAllocator before running the destructor so this function can access
    // it afterwards and it's not destroyed prematurely.
    Ref<BindGroupLayout> layout = ToBackend(GetLayout());
    BindGroupBase::DeleteThis();
    layout->DeallocateBindGroup(this);
}

MaybeError BindGroup::AddReferenced(CaptureContext& captureContext) {
    // We have to include any referenced bound textures views as the front end does
    // not track texture views.
    //
    // Unfortunately we can't just call `AddResource(layout)` because that would add a call to
    // createBindGroupLayout which we only want if the bindGroupLayout was not implicit. If
    // the bindGroupLayout was implicit then the code that serializes the command buffer makes sure
    // the pipeline is serialized first so its implicit bindGroupLayouts will already be
    // referenced.
    BindGroupLayoutBase* layout = GetFrontendLayout();
    if (layout->IsExplicit()) {
        // TODO(452983510): Handle explicit bind group layouts.
        // DAWN_TRY(captureContext.AddResource(layout));
        DAWN_CHECK(false);
    }

    {
        BindGroupLayoutInternalBase* layout = GetLayout();
        const auto& bindingMap = layout->GetBindingMap();
        for (const auto& [bindingNumbers, apiBindingIndex] : bindingMap) {
            BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
            const auto& bindingInfo = layout->GetAPIBindingInfo(apiBindingIndex);

            DAWN_TRY(MatchVariant(
                bindingInfo.bindingLayout,
                [&](const TextureBindingInfo& info) -> MaybeError {
                    return captureContext.AddResource(GetBindingAsTextureView(bindingIndex));
                },
                [&](const auto& info) -> MaybeError { return {}; }));
        }
    }

    return {};
}

MaybeError BindGroup::CaptureCreationParameters(CaptureContext& captureContext) {
    std::vector<schema::BindGroupEntry> entries;

    BindGroupLayoutInternalBase* layout = GetLayout();
    const auto& bindingMap = layout->GetBindingMap();

    for (const auto& [bindingNumber, apiBindingIndex] : bindingMap) {
        BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
        const auto& bindingInfo = layout->GetAPIBindingInfo(apiBindingIndex);

        MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& info) {
                const auto& entry = GetBindingAsBufferBinding(bindingIndex);
                entries.push_back(schema::BindGroupEntry{{
                    .binding = uint32_t(bindingNumber),
                    .bufferId = captureContext.GetId(entry.buffer),
                    .offset = entry.offset,
                    .size = entry.size,
                    .samplerId = 0,
                    .textureViewId = 0,
                }});
            },
            [&](const auto& info) { DAWN_CHECK(false); });
    }

    // This must use the front end layout as it is not deduplicated.
    schema::BindGroup bg{{
        .layoutId = captureContext.GetId(GetFrontendLayout()),
        .entries = entries,
    }};
    Serialize(captureContext, bg);
    return {};
}

}  // namespace dawn::native::webgpu
