// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/native/BindGroup.h"

#include <limits>
#include <variant>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/Assert.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Math.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Device.h"
#include "dawn/native/DynamicArrayState.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/Texture.h"
#include "dawn/native/utils/WGPUHelpers.h"

namespace dawn::native {

namespace {

// Helper functions to perform binding-type specific validation

MaybeError ValidateBufferBinding(const DeviceBase* device,
                                 const BindGroupEntry& entry,
                                 const BufferBindingInfo& layout) {
    DAWN_INVALID_IF(entry.buffer == nullptr, "Binding entry buffer not set.");

    DAWN_INVALID_IF(entry.sampler != nullptr || entry.textureView != nullptr,
                    "Expected only buffer to be set for binding entry.");

    DAWN_INVALID_IF(entry.nextInChain != nullptr, "nextInChain must be nullptr.");

    DAWN_TRY(device->ValidateObject(entry.buffer));

    uint64_t bufferSize = entry.buffer->GetSize();

    // Handle wgpu::WholeSize, avoiding overflows.
    DAWN_INVALID_IF(entry.offset > bufferSize,
                    "Binding offset (%u) is larger than the size (%u) of %s.", entry.offset,
                    bufferSize, entry.buffer);

    uint64_t bindingSize =
        (entry.size == wgpu::kWholeSize) ? bufferSize - entry.offset : entry.size;

    DAWN_INVALID_IF(bindingSize > bufferSize,
                    "Binding size (%u) is larger than the size (%u) of %s.", bindingSize,
                    bufferSize, entry.buffer);

    DAWN_INVALID_IF(bindingSize == 0, "Binding size for %s is zero.", entry.buffer);

    // Note that no overflow can happen because we already checked that
    // bufferSize >= bindingSize
    DAWN_INVALID_IF(entry.offset > bufferSize - bindingSize,
                    "Binding range (offset: %u, size: %u) doesn't fit in the size (%u) of %s.",
                    entry.offset, bufferSize, bindingSize, entry.buffer);

    wgpu::BufferUsage requiredUsage;
    uint64_t requiredBindingAlignment;
    switch (layout.type) {
        case wgpu::BufferBindingType::Uniform:
            requiredUsage = wgpu::BufferUsage::Uniform;
            requiredBindingAlignment = device->GetLimits().v1.minUniformBufferOffsetAlignment;
            break;
        case wgpu::BufferBindingType::Storage:
        case wgpu::BufferBindingType::ReadOnlyStorage:
            requiredUsage = wgpu::BufferUsage::Storage;
            requiredBindingAlignment = device->GetLimits().v1.minStorageBufferOffsetAlignment;
            DAWN_INVALID_IF(
                bindingSize % 4 != 0,
                "Binding size (%u) of %s isn't a multiple of 4 when binding type is (%s).",
                bindingSize, entry.buffer, layout.type);
            break;
        case kInternalStorageBufferBinding:
            requiredUsage = kInternalStorageBuffer;
            requiredBindingAlignment = device->GetLimits().v1.minStorageBufferOffsetAlignment;
            break;
        case kInternalReadOnlyStorageBufferBinding:
            // This is needed for for some workarounds that read a buffer in shaders. The buffer
            // only needs kReadOnlyStorageBuffer usage in this case. Unlike the standard
            // wgpu::BufferBindingType::ReadOnlyStorage which requires the read-write Storage usage.
            // On some backends such as D3D11, using only kReadOnlyStorageBuffer usage could avoid
            // extra allocations.
            requiredUsage = kReadOnlyStorageBuffer;
            requiredBindingAlignment = device->GetLimits().v1.minStorageBufferOffsetAlignment;
            break;
        case wgpu::BufferBindingType::BindingNotUsed:
        case wgpu::BufferBindingType::Undefined:
            DAWN_UNREACHABLE();
    }

    DAWN_INVALID_IF(!IsAligned(entry.offset, requiredBindingAlignment),
                    "Offset (%u) of %s does not satisfy the minimum %s alignment (%u).",
                    entry.offset, entry.buffer, layout.type, requiredBindingAlignment);

    DAWN_INVALID_IF(!(entry.buffer->GetInternalUsage() & requiredUsage),
                    "Binding usage (%s) of %s doesn't match expected usage (%s).",
                    entry.buffer->GetUsage(), entry.buffer, requiredUsage);

    DAWN_INVALID_IF(bindingSize < layout.minBindingSize,
                    "Binding size (%u) of %s is smaller than the minimum binding size (%u).",
                    bindingSize, entry.buffer, layout.minBindingSize);

    uint64_t maxUniformBufferBindingSize;
    uint64_t maxStorageBufferBindingSize;
    switch (layout.type) {
        case wgpu::BufferBindingType::Uniform:
            maxUniformBufferBindingSize = device->GetLimits().v1.maxUniformBufferBindingSize;
            DAWN_INVALID_IF(bindingSize > maxUniformBufferBindingSize,
                            "Binding size (%u) of %s is larger than the maximum uniform buffer "
                            "binding size (%u).%s",
                            bindingSize, entry.buffer, maxUniformBufferBindingSize,
                            DAWN_INCREASE_LIMIT_MESSAGE(device->GetAdapter()->GetLimits().v1,
                                                        maxUniformBufferBindingSize, bindingSize));
            break;
        case wgpu::BufferBindingType::Storage:
        case wgpu::BufferBindingType::ReadOnlyStorage:
        case kInternalStorageBufferBinding:
        case kInternalReadOnlyStorageBufferBinding:
            maxStorageBufferBindingSize = device->GetLimits().v1.maxStorageBufferBindingSize;
            DAWN_INVALID_IF(bindingSize > maxStorageBufferBindingSize,
                            "Binding size (%u) of %s is larger than the maximum storage buffer "
                            "binding size (%u).%s",
                            bindingSize, entry.buffer, maxStorageBufferBindingSize,
                            DAWN_INCREASE_LIMIT_MESSAGE(device->GetAdapter()->GetLimits().v1,
                                                        maxStorageBufferBindingSize, bindingSize));
            break;
        case wgpu::BufferBindingType::BindingNotUsed:
        case wgpu::BufferBindingType::Undefined:
            DAWN_UNREACHABLE();
    }

    return {};
}

MaybeError ValidateTextureBindGroupEntry(DeviceBase* device, const BindGroupEntry& entry) {
    DAWN_INVALID_IF(entry.textureView == nullptr, "Binding entry textureView not set.");

    DAWN_INVALID_IF(entry.sampler != nullptr || entry.buffer != nullptr,
                    "Expected only textureView to be set for binding entry.");

    DAWN_INVALID_IF(entry.nextInChain != nullptr, "nextInChain must be nullptr.");

    TextureViewBase* view = entry.textureView;
    DAWN_TRY(device->ValidateObject(entry.textureView));

    Aspect aspect = view->GetAspects();
    DAWN_INVALID_IF(!HasOneBit(aspect), "Multiple aspects (%s) selected in %s.", aspect, view);

    return {};
}

MaybeError ValidateCompatibilityModeTextureViewArrayLayer(DeviceBase* device,
                                                          const TextureViewBase* view,
                                                          const TextureBase* texture) {
    DAWN_INVALID_IF(
        view->GetBaseArrayLayer() != 0 || view->GetLayerCount() != texture->GetArrayLayers(),
        "Texture binding uses %s with baseArrayLayer (%u) and arrayLayerCount (%u), but must use "
        "all (%u) layers of %s in compatibility mode.",
        view, view->GetBaseArrayLayer(), view->GetLayerCount(), texture->GetArrayLayers(), texture);

    return {};
}

MaybeError ValidateSampledTextureBinding(DeviceBase* device,
                                         const BindGroupEntry& entry,
                                         const TextureBindingInfo& layout,
                                         UsageValidationMode mode) {
    DAWN_TRY(ValidateTextureBindGroupEntry(device, entry));

    TextureViewBase* view = entry.textureView;
    TextureBase* texture = view->GetTexture();

    Aspect aspect = view->GetAspects();
    SampleTypeBit supportedTypes = texture->GetFormat().GetAspectInfo(aspect).supportedSampleTypes;
    if (supportedTypes == SampleTypeBit::External) {
        DAWN_ASSERT(texture->GetSharedResourceMemoryContents());
        supportedTypes =
            static_cast<SharedTextureMemoryContents*>(texture->GetSharedResourceMemoryContents())
                ->GetExternalFormatSupportedSampleTypes();
    }
    DAWN_TRY(ValidateCanUseAs(view, wgpu::TextureUsage::TextureBinding, mode));

    DAWN_INVALID_IF(texture->IsMultisampledTexture() != layout.multisampled,
                    "Sample count (%u) of %s doesn't match expectation (multisampled: %d).",
                    texture->GetSampleCount(), texture, layout.multisampled);

    SampleTypeBit requiredType;
    if (layout.sampleType == kInternalResolveAttachmentSampleType) {
        // If the binding's sample type is kInternalResolveAttachmentSampleType,
        // then the supported types must contain float.
        requiredType = SampleTypeBit::UnfilterableFloat;
    } else {
        requiredType = SampleTypeToSampleTypeBit(layout.sampleType);
    }

    DAWN_INVALID_IF(!(supportedTypes & requiredType),
                    "None of the supported sample types (%s) of %s match the expected sample "
                    "types (%s).",
                    supportedTypes, texture, requiredType);

    DAWN_INVALID_IF(entry.textureView->GetDimension() != layout.viewDimension,
                    "Dimension (%s) of %s doesn't match the expected dimension (%s).",
                    entry.textureView->GetDimension(), entry.textureView, layout.viewDimension);

    if (!device->HasFlexibleTextureViews()) {
        DAWN_INVALID_IF(
            view->GetDimension() != texture->GetCompatibilityTextureBindingViewDimension(),
            "Dimension (%s) of %s must match textureBindingViewDimension (%s) of "
            "%s in compatibility mode.",
            view->GetDimension(), view, texture->GetCompatibilityTextureBindingViewDimension(),
            texture);

        DAWN_TRY(ValidateCompatibilityModeTextureViewArrayLayer(device, view, texture));
    }

    return {};
}

MaybeError ValidateTextureViewBindingUsedAsExternalTexture(DeviceBase* device,
                                                           const BindGroupEntry& entry) {
    // TODO(crbug.com/398752857): Error message should include that entry is neither an
    // ExternalTexture nor a TextureView when the layout contains an ExternalTexture entry.
    DAWN_TRY(ValidateTextureBindGroupEntry(device, entry));

    TextureViewBase* view = entry.textureView;
    TextureBase* texture = view->GetTexture();
    wgpu::TextureFormat format = view->GetFormat().format;

    DAWN_INVALID_IF(
        format != wgpu::TextureFormat::RGBA8Unorm && format != wgpu::TextureFormat::BGRA8Unorm &&
            format != wgpu::TextureFormat::RGBA16Float,
        "%s format (%s) is not %s, %s, or %s.", view, format, wgpu::TextureFormat::RGBA8Unorm,
        wgpu::TextureFormat::BGRA8Unorm, wgpu::TextureFormat::RGBA16Float);

    DAWN_INVALID_IF((view->GetUsage() & wgpu::TextureUsage::TextureBinding) == 0,
                    "%s usage (%s) doesn't include the required usage (%s)", view, view->GetUsage(),
                    wgpu::TextureUsage::TextureBinding);

    DAWN_INVALID_IF(view->GetDimension() != wgpu::TextureViewDimension::e2D,
                    "%s dimension (%s) is not 2D.", view, view->GetDimension());

    DAWN_INVALID_IF(view->GetLevelCount() > 1, "%s mip level count (%u) is not 1.", view,
                    view->GetLevelCount());

    DAWN_INVALID_IF(texture->GetSampleCount() != 1, "%s sample count (%u) is not 1.", texture,
                    texture->GetSampleCount());

    return {};
}

MaybeError ValidateStorageTextureBinding(DeviceBase* device,
                                         const BindGroupEntry& entry,
                                         const StorageTextureBindingInfo& layout,
                                         UsageValidationMode mode) {
    DAWN_TRY(ValidateTextureBindGroupEntry(device, entry));

    TextureViewBase* view = entry.textureView;
    TextureBase* texture = view->GetTexture();

    DAWN_TRY(ValidateCanUseAs(view, wgpu::TextureUsage::StorageBinding, mode));

    DAWN_ASSERT(!texture->IsMultisampledTexture());

    DAWN_INVALID_IF(texture->GetFormat().format != layout.format,
                    "Format (%s) of %s expected to be (%s).", texture->GetFormat().format, texture,
                    layout.format);

    DAWN_INVALID_IF(view->GetDimension() != layout.viewDimension,
                    "Dimension (%s) of %s doesn't match the expected dimension (%s).",
                    view->GetDimension(), entry.textureView, layout.viewDimension);

    DAWN_INVALID_IF(view->GetLevelCount() != 1, "mipLevelCount (%u) of %s expected to be 1.",
                    view->GetLevelCount(), view);

    if (!device->HasFlexibleTextureViews()) {
        DAWN_TRY(ValidateCompatibilityModeTextureViewArrayLayer(device, view, texture));
    }

    return {};
}

MaybeError ValidateSamplerBinding(const DeviceBase* device,
                                  const BindGroupEntry& entry,
                                  const SamplerBindingInfo& layout) {
    DAWN_INVALID_IF(entry.sampler == nullptr, "Binding entry sampler not set.");

    DAWN_INVALID_IF(entry.sampler->IsYCbCr(),
                    "YCbCr sampler is incompatible with SamplerBindingLayout");

    DAWN_INVALID_IF(entry.textureView != nullptr || entry.buffer != nullptr,
                    "Expected only sampler to be set for binding entry.");

    DAWN_INVALID_IF(entry.nextInChain != nullptr, "nextInChain must be nullptr.");

    DAWN_TRY(device->ValidateObject(entry.sampler));

    switch (layout.type) {
        case wgpu::SamplerBindingType::NonFiltering:
            DAWN_INVALID_IF(entry.sampler->IsFiltering(),
                            "Filtering sampler %s is incompatible with non-filtering sampler "
                            "binding.",
                            entry.sampler);
            [[fallthrough]];
        case wgpu::SamplerBindingType::Filtering:
            DAWN_INVALID_IF(entry.sampler->IsComparison(),
                            "Comparison sampler %s is incompatible with non-comparison sampler "
                            "binding.",
                            entry.sampler);
            break;
        case wgpu::SamplerBindingType::Comparison:
            DAWN_INVALID_IF(!entry.sampler->IsComparison(),
                            "Non-comparison sampler %s is incompatible with comparison sampler "
                            "binding.",
                            entry.sampler);
            break;
        default:
            DAWN_UNREACHABLE();
            break;
    }

    return {};
}

MaybeError ValidateExternalTextureBinding(
    const DeviceBase* device,
    const BindGroupEntry& entry,
    const ExternalTextureBindingEntry* externalTextureBindingEntry,
    const ExternalTextureBindingExpansionMap& expansions) {
    DAWN_INVALID_IF(externalTextureBindingEntry == nullptr,
                    "Binding entry external texture not set.");

    DAWN_INVALID_IF(
        entry.sampler != nullptr || entry.textureView != nullptr || entry.buffer != nullptr,
        "Expected only external texture to be set for binding entry.");

    DAWN_INVALID_IF(!expansions.contains(BindingNumber(entry.binding)),
                    "External texture binding entry %u is not present in the bind group layout.",
                    entry.binding);

    DAWN_TRY(device->ValidateObject(externalTextureBindingEntry->externalTexture));

    return {};
}

template <typename F>
void ForEachUnverifiedBufferBindingIndexImpl(const BindGroupLayoutInternalBase* bgl, F&& f) {
    uint32_t packedIndex = 0;
    for (BindingIndex bindingIndex : bgl->GetBufferIndices()) {
        const auto& bufferLayout =
            std::get<BufferBindingInfo>(bgl->GetBindingInfo(bindingIndex).bindingLayout);
        if (bufferLayout.minBindingSize == 0) {
            f(bindingIndex, packedIndex++);
        }
    }
}

MaybeError ValidateStaticSamplersWithSampledTextures(
    const UnpackedPtr<BindGroupDescriptor>& descriptor,
    const BindGroupLayoutInternalBase* layout) {
    absl::flat_hash_map<BindingNumber, uint32_t> bindingNumberToEntryIndexMap;
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        bindingNumberToEntryIndexMap[BindingNumber(descriptor->entries[i].binding)] = i;
    }

    // Entry indices of YCbCr textures sampled by a static sampler.
    ityp::bitset<uint32_t, kMaxBindingsPerPipelineLayout> sampledYcbcrTextures;
    for (BindingIndex index{0}; index < layout->GetBindingCount(); ++index) {
        const BindingInfo& bindingInfo = layout->GetBindingInfo(index);
        auto* staticSamplerLayout =
            std::get_if<StaticSamplerBindingInfo>(&bindingInfo.bindingLayout);
        if (staticSamplerLayout && staticSamplerLayout->isUsedForSingleTextureBinding) {
            const SamplerBase* sampler = staticSamplerLayout->sampler.Get();

            uint32_t textureEntryIndex = bindingNumberToEntryIndexMap.at(
                BindingNumber(staticSamplerLayout->sampledTextureBinding));
            const TextureViewBase* textureView = descriptor->entries[textureEntryIndex].textureView;

            // Compare static sampler and sampled textures to make sure they are compatible.
            if (sampler->IsYCbCr()) {
                DAWN_INVALID_IF(!textureView->IsYCbCr(),
                                "YCbCr static sampler at binding (%u) samples a non-YCbCr texture.",
                                bindingInfo.binding);

                sampledYcbcrTextures.set(textureEntryIndex);
            } else {
                DAWN_INVALID_IF(textureView->IsYCbCr(),
                                "Non-YCbCr static sampler at binding (%u) samples a YCbCr texture.",
                                bindingInfo.binding);
            }
        }
    }

    // Validate that all YCbCr texture entries are sampled by a static sampler.
    const auto& bindingMap = layout->GetBindingMap();
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        const BindGroupEntry& entry = descriptor->entries[i];
        const BindingInfo& bindingInfo =
            layout->GetBindingInfo(bindingMap.at(BindingNumber(entry.binding)));
        if (std::holds_alternative<TextureBindingInfo>(bindingInfo.bindingLayout) &&
            entry.textureView && entry.textureView->IsYCbCr()) {
            DAWN_INVALID_IF(!sampledYcbcrTextures.test(i),
                            "YCbCr texture at binding (%u) is not sampled by a static sampler.",
                            entry.binding);
        }
    }

    return {};
}

MaybeError ValidateBindGroupDynamicBindingArray(DeviceBase* device,
                                                const UnpackedPtr<BindGroupDescriptor> descriptor,
                                                UsageValidationMode mode) {
    auto* dynamic = descriptor.Get<BindGroupDynamicBindingArray>();
    DAWN_ASSERT(dynamic != nullptr);

    BindGroupLayoutInternalBase* layout = descriptor->layout->GetInternalBindGroupLayout();
    const BindGroupLayoutInternalBase::BindingMap& staticBindingMap = layout->GetBindingMap();

    DAWN_INVALID_IF(!device->HasFeature(Feature::ChromiumExperimentalBindless),
                    "Dynamic binding array used without the %s feature enabled.",
                    wgpu::FeatureName::ChromiumExperimentalBindless);

    DAWN_INVALID_IF(
        !layout->HasDynamicArray(),
        "dynamicArraySize specified when the layout (%s) doesn't contain a dynamic binding array.",
        layout);

    const uint32_t maxDynamicBindingArraySize =
        device->GetLimits().dynamicBindingArrayLimits.maxDynamicBindingArraySize;
    DAWN_INVALID_IF(dynamic->dynamicArraySize > maxDynamicBindingArraySize,
                    "dynamicArraySize (%u) exceeds the maxDynamicBindingArraySize limit (%u).",
                    dynamic->dynamicArraySize, maxDynamicBindingArraySize);

    const BindingNumber dynamicArrayStart = layout->GetAPIDynamicArrayStart();
    const BindingNumber dynamicArraySize = BindingNumber(dynamic->dynamicArraySize);

    // Validate that any non-static entry fits in the dynamic binding array and that they match its
    // kind as well.
    absl::flat_hash_set<BindingNumber> dynamicBindingsSeen;
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        const BindGroupEntry& entry = descriptor->entries[i];
        BindingNumber binding = BindingNumber(entry.binding);

        // Skip static entries to only look at dynamic entries, or invalid ones.
        if (staticBindingMap.contains(binding)) {
            continue;
        }

        DAWN_INVALID_IF(binding - dynamicArrayStart >= dynamicArraySize,
                        "In entries[%u], binding index %u doesn't fit in the dynamic binding "
                        "array range of indices [%u, %u).",
                        i, binding, dynamicArrayStart, dynamicArrayStart + dynamicArraySize);

        DAWN_INVALID_IF(dynamicBindingsSeen.contains(binding),
                        "In entries[%u], binding index %u already used by a previous entry", i,
                        binding);
        dynamicBindingsSeen.insert(binding);

        switch (layout->GetDynamicArrayKind()) {
            case wgpu::DynamicBindingKind::SampledTexture:
                // TODO(https://issues.chromium.org/435251399): Figure out the additional validation
                // rules for the texture kind. Also figure out how we should honor the
                // UsageValidationMode.
                DAWN_TRY(ValidateTextureBindGroupEntry(device, entry));
                break;

            case wgpu::DynamicBindingKind::Undefined:
                DAWN_UNREACHABLE();
        }
    }

    return {};
}

}  // anonymous namespace

ResultOrError<UnpackedPtr<BindGroupDescriptor>> ValidateBindGroupDescriptor(
    DeviceBase* device,
    const BindGroupDescriptor* descriptorChain,
    UsageValidationMode mode) {
    UnpackedPtr<BindGroupDescriptor> descriptor;
    DAWN_TRY_ASSIGN(descriptor, ValidateAndUnpack(descriptorChain));

    DAWN_TRY(device->ValidateObject(descriptor->layout));
    BindGroupLayoutInternalBase* layout = descriptor->layout->GetInternalBindGroupLayout();

    const BindGroupLayoutInternalBase::BindingMap& staticBindingMap = layout->GetBindingMap();
    DAWN_ASSERT(staticBindingMap.size() <= kMaxBindingsPerPipelineLayout);

    // Validate the static entries first as that's the common case that should be optimized for.
    // Validation of the dynamic entries is done with a second iteration over the entries, only if
    // needed.
    uint32_t staticEntryCount = 0;
    bool needsCrossBindingValidation = layout->NeedsCrossBindingValidation();
    ityp::bitset<BindingIndex, kMaxBindingsPerPipelineLayout> bindingsSet;
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        const BindGroupEntry& entry = descriptor->entries[i];
        BindingNumber binding = BindingNumber(entry.binding);

        // Do a single combined check for the entry being dynamic or a non-existent one to avoid
        // doing an extra branch in the static binding case.
        const auto& it = staticBindingMap.find(binding);
        if (it == staticBindingMap.end()) {
            if (descriptor.Has<BindGroupDynamicBindingArray>() &&
                binding >= layout->GetAPIDynamicArrayStart()) {
                continue;
            }
            return DAWN_VALIDATION_ERROR(
                "In entries[%u], binding index %u not present in the bind group layout."
                "\nExpected layout: %s",
                i, binding, layout->EntriesToString());
        }
        staticEntryCount++;

        // Check for redundant static entries.
        BindingIndex bindingIndex = it->second;
        DAWN_INVALID_IF(bindingsSet[bindingIndex],
                        "In entries[%u], binding index %u already used by a previous entry", i,
                        binding);
        bindingsSet.set(bindingIndex);

        const BindingInfo& bindingInfo = layout->GetBindingInfo(bindingIndex);

        // Below this block we validate entries based on the bind group layout, in which
        // external textures have been expanded into their underlying contents. For this reason
        // we must identify external texture binding entries by checking the bind group entry
        // itself.
        // TODO(42240282): Store external textures in
        // BindGroupLayoutBase::BindingDataPointers::bindings so checking external textures can
        // be moved in the switch below.
        if (layout->GetExternalTextureBindingExpansionMap().contains(binding)) {
            UnpackedPtr<BindGroupEntry> unpacked;
            DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(&entry));
            if (auto* externalTextureBindingEntry = unpacked.Get<ExternalTextureBindingEntry>()) {
                DAWN_TRY(ValidateExternalTextureBinding(
                    device, entry, externalTextureBindingEntry,
                    layout->GetExternalTextureBindingExpansionMap()));
                continue;
            }
            if (!device->IsToggleEnabled(Toggle::DisableTextureViewBindingUsedAsExternalTexture)) {
                DAWN_TRY_CONTEXT(ValidateTextureViewBindingUsedAsExternalTexture(device, entry),
                                 "validating entries[%u] as a TextureView."
                                 "\nExpected entry layout: %s",
                                 i, layout);
                continue;
            }
            return DAWN_VALIDATION_ERROR(
                "entries[%u] not an ExternalTexture when the layout contains an ExternalTexture "
                "entry.",
                i);
        }
        DAWN_INVALID_IF(entry.nextInChain != nullptr, "nextInChain must be nullptr.");

        // Perform binding-type specific validation.
        DAWN_TRY(MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& layout) -> MaybeError {
                // TODO(dawn:1485): Validate buffer binding with usage validation mode.
                DAWN_TRY_CONTEXT(ValidateBufferBinding(device, entry, layout),
                                 "validating entries[%u] as a Buffer."
                                 "\nExpected entry layout: %s",
                                 i, layout);
                return {};
            },
            [&](const TextureBindingInfo& layout) -> MaybeError {
                DAWN_TRY_CONTEXT(ValidateSampledTextureBinding(device, entry, layout, mode),
                                 "validating entries[%u] as a Sampled Texture."
                                 "\nExpected entry layout: %s",
                                 i, layout);
                if (entry.textureView->IsYCbCr()) {
                    // Need to validate that the YCbCr texture is statically sampled.
                    needsCrossBindingValidation = true;
                }

                return {};
            },
            [&](const StorageTextureBindingInfo& layout) -> MaybeError {
                DAWN_TRY_CONTEXT(ValidateStorageTextureBinding(device, entry, layout, mode),
                                 "validating entries[%u] as a Storage Texture."
                                 "\nExpected entry layout: %s",
                                 i, layout);
                return {};
            },
            [&](const SamplerBindingInfo& layout) -> MaybeError {
                DAWN_TRY_CONTEXT(ValidateSamplerBinding(device, entry, layout),
                                 "validating entries[%u] as a Sampler."
                                 "\nExpected entry layout: %s",
                                 i, layout);
                return {};
            },
            [&](const StaticSamplerBindingInfo& layout) -> MaybeError {
                return DAWN_VALIDATION_ERROR(
                    "entries[%u] is provided when the layout contains a static sampler for that "
                    "binding.",
                    i);
            },
            [](const InputAttachmentBindingInfo&) -> MaybeError {
                // Internal use only. No validation.
                return {};
            }));
    }

    // Check that we have all the required static entries.
    // NOTE: Static sampler layout bindings should not have bind group entries, as the sampler is
    // specified in the layout itself.
    const auto expectedStaticEntryCount =
        layout->GetUnexpandedBindingCount() - layout->GetStaticSamplerCount();

    DAWN_INVALID_IF(
        staticEntryCount != expectedStaticEntryCount,
        "Number of entries (%u) did not match the expected number of entries (%u) for %s."
        "\nExpected layout: %s",
        descriptor->entryCount, expectedStaticEntryCount, layout, layout->EntriesToString());

    // This should always be true because
    //  - numBindings has to match between the bind group and its layout.
    //  - Each binding must be set at most once
    //
    // We don't validate the equality because it wouldn't be possible to cover it with a test.
    DAWN_ASSERT(bindingsSet.count() == expectedStaticEntryCount);

    if (needsCrossBindingValidation) {
        // This additional validation is only needed when there are static samplers used with a
        // single texture binding and/or there are YCbCr textures.
        DAWN_TRY(ValidateStaticSamplersWithSampledTextures(descriptor, layout));
    }

    // Validate the dynamic entries.
    if (descriptor.Has<BindGroupDynamicBindingArray>()) {
        DAWN_TRY(ValidateBindGroupDynamicBindingArray(device, descriptor, mode));
    } else {
        DAWN_INVALID_IF(
            layout->HasDynamicArray(),
            "dynamicArraySize is not specified when the layout contains a dynamic binding array.");
    }

    return descriptor;
}

// BindGroup

BindGroupBase::BindGroupBase(DeviceBase* device,
                             const UnpackedPtr<BindGroupDescriptor>& descriptor,
                             void* bindingDataStart)
    : ApiObjectBase(device, descriptor->label),
      mLayout(descriptor->layout),
      mBindingData(GetLayout()->ComputeBindingDataPointers(bindingDataStart)) {
    GetObjectTrackingList()->Track(this);
}

MaybeError BindGroupBase::Initialize(const UnpackedPtr<BindGroupDescriptor>& descriptor) {
    BindGroupLayoutInternalBase* layout = GetLayout();

    for (BindingIndex i{0}; i < layout->GetBindingCount(); ++i) {
        // TODO(enga): Shouldn't be needed when bindings are tightly packed.
        // This is to fill Ref<ObjectBase> holes with nullptrs.
        new (&mBindingData.bindings[i]) Ref<ObjectBase>();
    }

    // Get the dynamic array start or a fake start that will make sure no binding gets accounted as
    // being in the dynamic array. This keeps the condition in the loop below simple.
    BindingNumber dynamicArrayStart = std::numeric_limits<BindingNumber>::max();
    if (layout->HasDynamicArray()) {
        dynamicArrayStart = layout->GetAPIDynamicArrayStart();
    }

    // Gather static bindings.
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        UnpackedPtr<BindGroupEntry> entry = Unpack(&descriptor->entries[i]);
        BindingNumber binding = BindingNumber(entry->binding);

        if (binding >= dynamicArrayStart) {
            continue;
        }

        BindingIndex bindingIndex = layout->GetBindingIndex(binding);
        DAWN_ASSERT(bindingIndex < layout->GetBindingCount());

        // Only a single binding type should be set, so once we found it we can skip to the
        // next loop iteration.

        if (entry->buffer != nullptr) {
            DAWN_ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
            mBindingData.bindings[bindingIndex] = entry->buffer;
            mBindingData.bufferData[bindingIndex].offset = entry->offset;
            uint64_t bufferSize = (entry->size == wgpu::kWholeSize)
                                      ? entry->buffer->GetSize() - entry->offset
                                      : entry->size;
            mBindingData.bufferData[bindingIndex].size = bufferSize;
            continue;
        }

        if (entry->textureView != nullptr) {
            // TODO(42240282): Store external textures in
            // BindGroupLayoutBase::BindingDataPointers::bindings so that we can have a MatchVariant
            // that contains the ExternalTextureInfo.
            ExternalTextureBindingExpansionMap expansions =
                layout->GetExternalTextureBindingExpansionMap();
            ExternalTextureBindingExpansionMap::iterator it =
                expansions.find(BindingNumber(entry->binding));

            if (it == expansions.end()) {
                DAWN_ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
                mBindingData.bindings[bindingIndex] = entry->textureView;
            } else {
                // If this is for a texture view that is used as an external texture, we need to
                // also provide placeholder for the second plane and a parameter buffer.
                BindingIndex plane0BindingIndex = layout->GetBindingIndex(it->second.plane0);
                BindingIndex plane1BindingIndex = layout->GetBindingIndex(it->second.plane1);
                BindingIndex paramsBindingIndex = layout->GetBindingIndex(it->second.params);

                DAWN_ASSERT(mBindingData.bindings[plane0BindingIndex] == nullptr);
                mBindingData.bindings[plane0BindingIndex] = entry->textureView;

                DAWN_ASSERT(mBindingData.bindings[plane1BindingIndex] == nullptr);
                DAWN_TRY_ASSIGN(mBindingData.bindings[plane1BindingIndex],
                                GetDevice()->GetOrCreatePlaceholderTextureViewForExternalTexture());

                DAWN_ASSERT(mBindingData.bindings[paramsBindingIndex] == nullptr);
                Ref<BufferBase> paramsBuffer;
                DAWN_TRY_ASSIGN(paramsBuffer,
                                MakeParamsBufferForSimpleView(GetDevice(), entry->textureView));
                mBindingData.bindings[paramsBindingIndex] = paramsBuffer;
                mBindingData.bufferData[paramsBindingIndex].offset = 0;
                mBindingData.bufferData[paramsBindingIndex].size = paramsBuffer->GetSize();
            }
            continue;
        }

        if (entry->sampler != nullptr) {
            DAWN_ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
            mBindingData.bindings[bindingIndex] = entry->sampler;
            continue;
        }

        // Here we unpack external texture bindings into multiple additional bindings for the
        // external texture's contents. New binding locations previously determined in the bind
        // group layout are created in this bind group and filled with the external texture's
        // underlying resources.
        if (auto* externalTextureBindingEntry = entry.Get<ExternalTextureBindingEntry>()) {
            mBoundExternalTextures.push_back(externalTextureBindingEntry->externalTexture);

            ExternalTextureBindingExpansionMap expansions =
                layout->GetExternalTextureBindingExpansionMap();
            ExternalTextureBindingExpansionMap::iterator it =
                expansions.find(BindingNumber(entry->binding));

            DAWN_ASSERT(it != expansions.end());

            BindingIndex plane0BindingIndex = layout->GetBindingIndex(it->second.plane0);
            BindingIndex plane1BindingIndex = layout->GetBindingIndex(it->second.plane1);
            BindingIndex paramsBindingIndex = layout->GetBindingIndex(it->second.params);

            DAWN_ASSERT(mBindingData.bindings[plane0BindingIndex] == nullptr);

            mBindingData.bindings[plane0BindingIndex] =
                externalTextureBindingEntry->externalTexture->GetTextureViews()[0];

            DAWN_ASSERT(mBindingData.bindings[plane1BindingIndex] == nullptr);
            mBindingData.bindings[plane1BindingIndex] =
                externalTextureBindingEntry->externalTexture->GetTextureViews()[1];

            DAWN_ASSERT(mBindingData.bindings[paramsBindingIndex] == nullptr);
            mBindingData.bindings[paramsBindingIndex] =
                externalTextureBindingEntry->externalTexture->GetParamsBuffer();
            mBindingData.bufferData[paramsBindingIndex].offset = 0;
            mBindingData.bufferData[paramsBindingIndex].size =
                sizeof(dawn::native::ExternalTextureParams);

            continue;
        }
    }

    ForEachUnverifiedBufferBindingIndexImpl(layout,
                                            [&](BindingIndex bindingIndex, uint32_t packedIndex) {
                                                mBindingData.unverifiedBufferSizes[packedIndex] =
                                                    mBindingData.bufferData[bindingIndex].size;
                                            });

    // Gather dynamic binding entries in a second loop to put the handling off the critical path.
    if (auto* dynamic = descriptor.Get<BindGroupDynamicBindingArray>()) {
        mDynamicArray =
            std::make_unique<DynamicArrayState>(BindingIndex(dynamic->dynamicArraySize));
        DAWN_TRY(mDynamicArray->Initialize(GetDevice()));

        for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
            UnpackedPtr<BindGroupEntry> entry = Unpack(&descriptor->entries[i]);
            BindingNumber binding = BindingNumber(entry->binding);

            if (binding < layout->GetAPIDynamicArrayStart()) {
                continue;
            }
            mDynamicArray->Update(layout->GetDynamicBindingIndex(binding), entry->textureView);
        }

        // Add the metadata storage buffer in the static bindings.
        BindingIndex metadataIndex = layout->GetDynamicArrayMetadataBinding();
        mBindingData.bindings[metadataIndex] = mDynamicArray->GetMetadataBuffer();
        mBindingData.bufferData[metadataIndex].offset = 0;
        mBindingData.bufferData[metadataIndex].size = mDynamicArray->GetMetadataBuffer()->GetSize();
    }

    DAWN_TRY(InitializeImpl());

    return {};
}

BindGroupBase::~BindGroupBase() = default;

void BindGroupBase::DestroyImpl() {
    if (mLayout != nullptr) {
        DAWN_ASSERT(!IsError());
        for (BindingIndex i{0}; i < GetLayout()->GetBindingCount(); ++i) {
            mBindingData.bindings[i].~Ref<ObjectBase>();
        }
    }

    if (mDynamicArray != nullptr && !mDynamicArray->IsDestroyed()) {
        DAWN_ASSERT(!IsError());
        mDynamicArray->Destroy();
    }
}

BindGroupBase::BindGroupBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label)
    : ApiObjectBase(device, tag, label), mBindingData() {}

// static
Ref<BindGroupBase> BindGroupBase::MakeError(DeviceBase* device, StringView label) {
    class ErrorBindGroupBase final : public BindGroupBase {
      public:
        explicit ErrorBindGroupBase(DeviceBase* device, StringView label)
            : BindGroupBase(device, ObjectBase::kError, label) {}

        MaybeError InitializeImpl() override { DAWN_UNREACHABLE(); }
    };
    return AcquireRef(new ErrorBindGroupBase(device, label));
}

ObjectType BindGroupBase::GetType() const {
    return ObjectType::BindGroup;
}

void BindGroupBase::APIDestroy() {
    if (GetDevice()->IsValidationEnabled() &&
        GetDevice()->ConsumedError(ValidateDestroy(), "validating %s.Destroy()", this)) {
        return;
    }

    // Destroy only the dynamic array part. Static bindings are always supposed to be valid so that
    // SetBindGroup can iterate them without first checking if the BindGroup is destroyed. This also
    // makes the behavior match between the bind groups with only static bindings and the static
    // bindings part of the bind groups with dynamic arrays.
    if (!mDynamicArray->IsDestroyed()) {
        mDynamicArray->Destroy();
    }
}

BindGroupLayoutBase* BindGroupBase::GetFrontendLayout() {
    DAWN_ASSERT(!IsError());
    return mLayout.Get();
}

const BindGroupLayoutBase* BindGroupBase::GetFrontendLayout() const {
    DAWN_ASSERT(!IsError());
    return mLayout.Get();
}

BindGroupLayoutInternalBase* BindGroupBase::GetLayout() {
    DAWN_ASSERT(!IsError());
    return mLayout->GetInternalBindGroupLayout();
}

const BindGroupLayoutInternalBase* BindGroupBase::GetLayout() const {
    DAWN_ASSERT(!IsError());
    return mLayout->GetInternalBindGroupLayout();
}

const ityp::span<uint32_t, uint64_t>& BindGroupBase::GetUnverifiedBufferSizes() const {
    DAWN_ASSERT(!IsError());
    return mBindingData.unverifiedBufferSizes;
}

BufferBase* BindGroupBase::GetBindingAsBuffer(BindingIndex bindingIndex) {
    DAWN_ASSERT(!IsError());
    const BindGroupLayoutInternalBase* layout = GetLayout();
    DAWN_ASSERT(bindingIndex < layout->GetBindingCount());
    DAWN_ASSERT(std::holds_alternative<BufferBindingInfo>(
        layout->GetBindingInfo(bindingIndex).bindingLayout));
    return static_cast<BufferBase*>(mBindingData.bindings[bindingIndex].Get());
}

SamplerBase* BindGroupBase::GetBindingAsSampler(BindingIndex bindingIndex) const {
    DAWN_ASSERT(!IsError());
    const BindGroupLayoutInternalBase* layout = GetLayout();
    DAWN_ASSERT(bindingIndex < layout->GetBindingCount());
    DAWN_ASSERT(std::holds_alternative<SamplerBindingInfo>(
        layout->GetBindingInfo(bindingIndex).bindingLayout));
    return static_cast<SamplerBase*>(mBindingData.bindings[bindingIndex].Get());
}

TextureViewBase* BindGroupBase::GetBindingAsTextureView(BindingIndex bindingIndex) {
    DAWN_ASSERT(!IsError());
    const BindGroupLayoutInternalBase* layout = GetLayout();
    DAWN_ASSERT(bindingIndex < layout->GetBindingCount());
    DAWN_ASSERT(std::holds_alternative<TextureBindingInfo>(
                    layout->GetBindingInfo(bindingIndex).bindingLayout) ||
                std::holds_alternative<StorageTextureBindingInfo>(
                    layout->GetBindingInfo(bindingIndex).bindingLayout) ||
                std::holds_alternative<InputAttachmentBindingInfo>(
                    layout->GetBindingInfo(bindingIndex).bindingLayout));
    return static_cast<TextureViewBase*>(mBindingData.bindings[bindingIndex].Get());
}

BufferBinding BindGroupBase::GetBindingAsBufferBinding(BindingIndex bindingIndex) {
    DAWN_ASSERT(!IsError());
    return {GetBindingAsBuffer(bindingIndex), mBindingData.bufferData[bindingIndex].offset,
            mBindingData.bufferData[bindingIndex].size};
}

const std::vector<Ref<ExternalTextureBase>>& BindGroupBase::GetBoundExternalTextures() const {
    DAWN_ASSERT(!IsError());
    return mBoundExternalTextures;
}

void BindGroupBase::ForEachUnverifiedBufferBindingIndex(
    std::function<void(BindingIndex, uint32_t)> fn) const {
    ForEachUnverifiedBufferBindingIndexImpl(GetLayout(), fn);
}

bool BindGroupBase::HasDynamicArray() const {
    DAWN_ASSERT(!IsError());
    return mDynamicArray != nullptr;
}

ityp::span<BindingIndex, const Ref<TextureViewBase>> BindGroupBase::GetDynamicArrayBindings()
    const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(HasDynamicArray());
    return mDynamicArray->GetBindings();
}

MaybeError BindGroupBase::ValidateCanUseOnQueueNow() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(HasDynamicArray());

    DAWN_INVALID_IF(mDynamicArray->IsDestroyed(), "Destroyed bind group %s used in a submit.",
                    this);
    return {};
}

MaybeError BindGroupBase::ValidateDestroy() const {
    DAWN_TRY(GetDevice()->ValidateObject(this));

    // On the queue we only validate that dynamic array bind groups are alive in a submit because
    // validating for all bind groups would be too expensive. For that reason only allow dynamic
    // arrays to be destroyed early.
    DAWN_INVALID_IF(!HasDynamicArray(), "%s doesn't contain a dynamic array.", this);
    return {};
}

}  // namespace dawn::native
