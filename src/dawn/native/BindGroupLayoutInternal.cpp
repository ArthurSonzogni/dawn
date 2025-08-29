// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/BindGroupLayoutInternal.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <list>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "dawn/common/Enumerator.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/Error.h"
#include "dawn/native/Instance.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/TexelBufferView.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/platform/metrics/HistogramMacros.h"

namespace dawn::native {

namespace {

bool TextureFormatSupportStorageAccess(const Format& format, wgpu::StorageTextureAccess access) {
    switch (access) {
        case wgpu::StorageTextureAccess::ReadOnly:
            return format.supportsReadOnlyStorageUsage;
        case wgpu::StorageTextureAccess::WriteOnly:
            return format.supportsWriteOnlyStorageUsage;
        case wgpu::StorageTextureAccess::ReadWrite:
            return format.supportsReadWriteStorageUsage;
        default:
            DAWN_UNREACHABLE();
    }
}

MaybeError ValidateStorageTextureFormat(DeviceBase* device,
                                        wgpu::TextureFormat storageTextureFormat,
                                        wgpu::StorageTextureAccess access) {
    const Format* format = nullptr;
    DAWN_TRY_ASSIGN(format, device->GetInternalFormat(storageTextureFormat));
    DAWN_ASSERT(format != nullptr);

    // TODO(427681156): Remove this deprecation warning
    if (storageTextureFormat == wgpu::TextureFormat::BGRA8Unorm &&
        access == wgpu::StorageTextureAccess::ReadOnly) {
        DAWN_HISTOGRAM_BOOLEAN(device->GetPlatform(), "BGRA8UnormStorageTextureReadOnlyUsage",
                               true);
        device->EmitWarningOnce(
            "bgra8unorm with read-only access is deprecated. bgra8unorm only supports write-only "
            "access. Note: allowing this usage was a bug in Chrome. The spec disallows it as it is "
            "not portable.");
    }

    DAWN_INVALID_IF(!TextureFormatSupportStorageAccess(*format, access),
                    "Texture format %s does not support storage texture access %s.",
                    storageTextureFormat, access);

    return {};
}

MaybeError ValidateStorageTextureViewDimension(wgpu::TextureViewDimension dimension) {
    switch (dimension) {
        case wgpu::TextureViewDimension::Cube:
        case wgpu::TextureViewDimension::CubeArray:
            return DAWN_VALIDATION_ERROR("%s texture views cannot be used as storage textures.",
                                         dimension);

        case wgpu::TextureViewDimension::e1D:
        case wgpu::TextureViewDimension::e2D:
        case wgpu::TextureViewDimension::e2DArray:
        case wgpu::TextureViewDimension::e3D:
            return {};

        case wgpu::TextureViewDimension::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MaybeError ValidateBindGroupLayoutEntry(DeviceBase* device,
                                        const UnpackedPtr<BindGroupLayoutEntry>& entry,
                                        bool allowInternalBinding) {
    DAWN_TRY(ValidateShaderStage(entry->visibility));

    uint32_t arraySize = std::max(1u, entry->bindingArraySize);

    int bindingMemberCount = 0;

    if (entry->buffer.type != wgpu::BufferBindingType::BindingNotUsed) {
        bindingMemberCount++;
        const BufferBindingLayout& buffer = entry->buffer;

        // The kInternalStorageBufferBinding is used internally and not a value
        // in wgpu::BufferBindingType.
        if (buffer.type == kInternalStorageBufferBinding ||
            buffer.type == kInternalReadOnlyStorageBufferBinding) {
            DAWN_INVALID_IF(!allowInternalBinding, "Internal binding types are disallowed");
        } else {
            DAWN_TRY(ValidateBufferBindingType(buffer.type));
        }

        if (buffer.type == wgpu::BufferBindingType::Storage ||
            buffer.type == kInternalStorageBufferBinding) {
            DAWN_INVALID_IF(
                entry->visibility & wgpu::ShaderStage::Vertex,
                "Read-write storage buffer binding is used with a visibility (%s) that contains %s "
                "(note that read-only storage buffer bindings are allowed).",
                entry->visibility, wgpu::ShaderStage::Vertex);
        }

        // TODO(393558555): Support bindingArraySize > 1 for non-dynamic buffers.
        DAWN_INVALID_IF(arraySize > 1,
                        "bindingArraySize (%u) > 1 for a buffer binding is not implemented yet.",
                        arraySize);
    }

    if (entry->sampler.type != wgpu::SamplerBindingType::BindingNotUsed) {
        bindingMemberCount++;
        DAWN_TRY(ValidateSamplerBindingType(entry->sampler.type));

        // TODO(393558555): Support bindingArraySize > 1 for samplers.
        DAWN_INVALID_IF(arraySize > 1,
                        "bindingArraySize (%u) > 1 for a sampler binding is not implemented yet.",
                        arraySize);
    }

    if (entry->texture.sampleType != wgpu::TextureSampleType::BindingNotUsed) {
        bindingMemberCount++;
        const TextureBindingLayout& texture = entry->texture;
        // The kInternalResolveAttachmentSampleType is used internally and not a value
        // in wgpu::TextureSampleType.
        switch (texture.sampleType) {
            case kInternalResolveAttachmentSampleType:
                if (allowInternalBinding) {
                    break;
                }
                // should return validation error.
                [[fallthrough]];
            default:
                DAWN_TRY(ValidateTextureSampleType(texture.sampleType));
                break;
        }

        // viewDimension defaults to 2D if left undefined, needs validation otherwise.
        wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::e2D;
        if (texture.viewDimension != wgpu::TextureViewDimension::Undefined) {
            switch (texture.viewDimension) {
                case kInternalInputAttachmentDim:
                    if (allowInternalBinding) {
                        break;
                    }
                    // should return validation error.
                    [[fallthrough]];
                default:
                    DAWN_TRY(ValidateTextureViewDimension(texture.viewDimension));
            }
            viewDimension = texture.viewDimension;
        }

        DAWN_INVALID_IF(texture.multisampled && viewDimension != wgpu::TextureViewDimension::e2D,
                        "View dimension (%s) for a multisampled texture bindings was not %s.",
                        viewDimension, wgpu::TextureViewDimension::e2D);

        DAWN_INVALID_IF(
            texture.multisampled && texture.sampleType == wgpu::TextureSampleType::Float,
            "Sample type for multisampled texture binding was %s.", wgpu::TextureSampleType::Float);
    }

    if (entry->storageTexture.access != wgpu::StorageTextureAccess::BindingNotUsed) {
        bindingMemberCount++;
        const StorageTextureBindingLayout& storageTexture = entry->storageTexture;
        DAWN_TRY(ValidateStorageTextureAccess(storageTexture.access));
        DAWN_TRY(
            ValidateStorageTextureFormat(device, storageTexture.format, storageTexture.access));

        // viewDimension defaults to 2D if left undefined, needs validation otherwise.
        if (storageTexture.viewDimension != wgpu::TextureViewDimension::Undefined) {
            DAWN_TRY(ValidateTextureViewDimension(storageTexture.viewDimension));
            DAWN_TRY(ValidateStorageTextureViewDimension(storageTexture.viewDimension));
        }

        switch (storageTexture.access) {
            case wgpu::StorageTextureAccess::ReadOnly:
                break;
            case wgpu::StorageTextureAccess::ReadWrite:
            case wgpu::StorageTextureAccess::WriteOnly:
                DAWN_INVALID_IF(entry->visibility & wgpu::ShaderStage::Vertex,
                                "Storage texture binding with %s is used with a visibility (%s) "
                                "that contains %s.",
                                storageTexture.access, entry->visibility,
                                wgpu::ShaderStage::Vertex);
                break;
            default:
                DAWN_UNREACHABLE();
        }

        // TODO(393558555): Support bindingArraySize > 1 for storage textures.
        DAWN_INVALID_IF(
            arraySize > 1,
            "bindingArraySize (%u) > 1 for a storage texture binding is not implemented yet.",
            arraySize);
    }

    if (auto* staticSamplerBindingLayout = entry.Get<StaticSamplerBindingLayout>()) {
        bindingMemberCount++;

        DAWN_INVALID_IF(!device->HasFeature(Feature::StaticSamplers),
                        "Static samplers used without the %s feature enabled.",
                        wgpu::FeatureName::StaticSamplers);

        DAWN_TRY(device->ValidateObject(staticSamplerBindingLayout->sampler));
        DAWN_INVALID_IF(arraySize > 1,
                        "BindGroupLayoutEntry bindingArraySize (%u) > 1 for a static "
                        "sampler entry.",
                        arraySize);

        if (staticSamplerBindingLayout->sampledTextureBinding == WGPU_LIMIT_U32_UNDEFINED) {
            DAWN_INVALID_IF(staticSamplerBindingLayout->sampler->IsYCbCr(),
                            "YCbCr static sampler requires a sampled texture binding");
        }
    }

    if (auto* texelBufferLayout = entry.Get<TexelBufferBindingLayout>()) {
        bindingMemberCount++;
        DAWN_INVALID_IF(!device->AreTexelBuffersEnabled(), "%s is not enabled.",
                        wgpu::WGSLLanguageFeatureName::TexelBuffers);

        DAWN_TRY(ValidateTexelBufferAccess(texelBufferLayout->access));

        // TODO(393558555): Support bindingArraySize > 1 for texel buffers.
        DAWN_INVALID_IF(
            arraySize > 1,
            "bindingArraySize (%u) > 1 for a storage texture binding is not implemented yet.",
            arraySize);

        DAWN_INVALID_IF(entry->visibility & wgpu::ShaderStage::Vertex &&
                            texelBufferLayout->access != wgpu::TexelBufferAccess::ReadOnly,
                        "Vertex visibility requires read-only texel buffer access.");

        const Format* format;
        DAWN_TRY_ASSIGN(format, device->GetInternalFormat(texelBufferLayout->format));
        DAWN_INVALID_IF(!IsFormatSupportedForTexelBuffer(format->format),
                        "Texel buffer layout format (%s) is not allowed for texel buffers.",
                        format->format);
    }

    if (entry.Get<ExternalTextureBindingLayout>()) {
        bindingMemberCount++;
        DAWN_INVALID_IF(arraySize > 1,
                        "BindGroupLayoutEntry bindingArraySize (%u) > 1 for an "
                        "external texture entry.",
                        arraySize);
    }

    DAWN_INVALID_IF(bindingMemberCount == 0,
                    "BindGroupLayoutEntry had none of buffer, sampler, texture, "
                    "storageTexture, texelBuffer, or externalTexture set");

    DAWN_INVALID_IF(bindingMemberCount != 1,
                    "BindGroupLayoutEntry had more than one of buffer, sampler, texture, "
                    "storageTexture, texelBuffer, or externalTexture set");

    DAWN_INVALID_IF(
        arraySize > 1 && entry->texture.sampleType == wgpu::TextureSampleType::BindingNotUsed,
        "Entry that is not a sampled texture has an bindingArraySize (%u) > 1.", arraySize);

    return {};
}

MaybeError ValidateStaticSamplersWithTextureBindings(
    DeviceBase* device,
    const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor,
    const std::map<BindingNumber, uint32_t>& bindingNumberToIndexMap) {
    // Map of texture binding number to static sampler binding number.
    std::map<BindingNumber, BindingNumber> textureToStaticSamplerBindingMap;

    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        UnpackedPtr<BindGroupLayoutEntry> entry = Unpack(&descriptor->entries[i]);
        auto* staticSamplerLayout = entry.Get<StaticSamplerBindingLayout>();
        if (!staticSamplerLayout ||
            staticSamplerLayout->sampledTextureBinding == WGPU_LIMIT_U32_UNDEFINED) {
            continue;
        }

        BindingNumber samplerBinding(entry->binding);
        BindingNumber sampledTextureBinding(staticSamplerLayout->sampledTextureBinding);

        bool inserted =
            textureToStaticSamplerBindingMap.insert({sampledTextureBinding, samplerBinding}).second;
        DAWN_INVALID_IF(!inserted,
                        "For static sampler binding (%u) the sampled texture binding (%u) is "
                        "already bound to a static sampler at binding (%u).",
                        samplerBinding, sampledTextureBinding,
                        textureToStaticSamplerBindingMap[sampledTextureBinding]);

        DAWN_INVALID_IF(!bindingNumberToIndexMap.contains(sampledTextureBinding),
                        "For static sampler binding (%u) the sampled texture binding (%u) is not a "
                        "valid binding number.",
                        samplerBinding, sampledTextureBinding);

        auto& textureEntry = descriptor->entries[bindingNumberToIndexMap.at(sampledTextureBinding)];
        DAWN_INVALID_IF(textureEntry.texture.sampleType == wgpu::TextureSampleType::BindingNotUsed,
                        "For static sampler binding (%u) the sampled texture binding (%u) is not a "
                        "texture binding.",
                        samplerBinding, sampledTextureBinding);
    }

    return {};
}

}  // anonymous namespace

ResultOrError<UnpackedPtr<BindGroupLayoutDescriptor>> ValidateBindGroupLayoutDescriptor(
    DeviceBase* device,
    const BindGroupLayoutDescriptor* descriptorChain,
    bool allowInternalBinding) {
    UnpackedPtr<BindGroupLayoutDescriptor> descriptor;
    DAWN_TRY_ASSIGN(descriptor, ValidateAndUnpack(descriptorChain));

    // A running total of the number of bindings used by the layout.
    BindingCounts bindingCounts = {};

    // Handle the dynamic binding array first to also extract information needed to validate the
    // rest of the bindings.
    std::optional<BindingNumber> startOfDynamicArray = {};
    if (auto* dynamic = descriptor.Get<BindGroupLayoutDynamicBindingArray>()) {
        DAWN_INVALID_IF(!device->HasFeature(Feature::ChromiumExperimentalBindless),
                        "Dynamic binding array used without the %s feature enabled.",
                        wgpu::FeatureName::ChromiumExperimentalBindless);
        DAWN_INVALID_IF(dynamic->dynamicArray.nextInChain != nullptr,
                        "DynamicBindingArrayLayout::nextInChain must be nullptr");

        startOfDynamicArray = BindingNumber(dynamic->dynamicArray.start);
        DAWN_TRY(ValidateDynamicBindingKind(dynamic->dynamicArray.kind));

        DAWN_INVALID_IF(startOfDynamicArray >= kMaxBindingsPerBindGroupTyped,
                        "dynamic array start (%u) exceeds the maxBindingsPerBindGroup limit (%u).",
                        startOfDynamicArray.value(), kMaxBindingsPerBindGroup);

        // Add to the limits the storage buffer that will be used for the availability data of the
        // dynamic array. Set a minimum binding size so as to not increment unverifiedBufferCount.
        BindGroupLayoutEntry availabilityEntry{
            .binding = 0,
            .visibility = kAllStages,
            .buffer =
                {
                    .type = wgpu::BufferBindingType::ReadOnlyStorage,
                    .minBindingSize = 4,
                },
        };
        IncrementBindingCounts(&bindingCounts, Unpack(&availabilityEntry));

        // Check that no ExternalTexture entry is in the static bindings part. It is redundant with
        // validation below but since this is a temporary constraint only, it is fine.
        // TODO(https://crbug.com/42240282): This is a workaround for an issue generating the
        // binding numbers for expanded ExternalTexture entries. See comment in
        // ConvertAndExpandBGLEntries.
        // TODO(https://crbug.com/435317394): Remove this constraint that isn't necessary for
        // dynamic binding arrays, except as a temporary workaround.
        for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
            UnpackedPtr<BindGroupLayoutEntry> entry;
            DAWN_TRY_ASSIGN(entry, ValidateAndUnpack(&descriptor->entries[i]));
            DAWN_INVALID_IF(
                entry.Has<ExternalTextureBindingLayout>(),
                "entries[%i] is an ExternalTexture when a dynamic binding array is specified.", i);
        }
    }

    // Map of binding number to entry index.
    std::map<BindingNumber, uint32_t> bindingMap;

    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        UnpackedPtr<BindGroupLayoutEntry> entry;
        DAWN_TRY_ASSIGN(entry, ValidateAndUnpack(&descriptor->entries[i]));

        BindingNumber bindingNumber = BindingNumber(entry->binding);
        DAWN_INVALID_IF(
            bindingNumber >= kMaxBindingsPerBindGroupTyped,
            "On entries[%u]: binding number (%u) exceeds the maxBindingsPerBindGroup limit (%u).",
            i, bindingNumber, kMaxBindingsPerBindGroup);

        BindingNumber arraySize{1};
        if (entry->bindingArraySize > 1) {
            arraySize = BindingNumber(entry->bindingArraySize);

            DAWN_INVALID_IF(device->IsToggleEnabled(Toggle::DisableBindGroupLayoutEntryArraySize),
                            "On entries[%u]: use of bindingArraySize > 1 is disabled.", i);
            DAWN_INVALID_IF(!device->IsToggleEnabled(Toggle::AllowUnsafeAPIs),
                            "On entries[%u]: use of bindingArraySize > 1 is currently unsafe.", i);

            DAWN_INVALID_IF(arraySize > kMaxBindingsPerBindGroupTyped - bindingNumber,
                            "On entries[%u]: binding (%u) + arraySize (%u) is %u which is larger "
                            "than maxBindingsPerBindGroup (%u).",
                            i, arraySize, bindingNumber,
                            uint32_t(arraySize) + uint32_t(bindingNumber),
                            kMaxBindingsPerBindGroupTyped);
        }

        DAWN_INVALID_IF(startOfDynamicArray.has_value() &&
                            bindingNumber + arraySize > startOfDynamicArray.value(),
                        "On entries[%u]: the range of binding used [%u, %u) conflicts with the "
                        "dynamic binding array that starts at binding %u.",
                        i, bindingNumber, bindingNumber + arraySize, startOfDynamicArray.value());

        // Check that the same binding is not set twice. bindingNumber + arraySize cannot overflow
        // as they are both smaller than kMaxBindingsPerBindGroupTyped.
        static_assert(kMaxBindingsPerBindGroup < std::numeric_limits<uint32_t>::max() / 2);
        for (BindingNumber usedBinding : Range(bindingNumber, bindingNumber + arraySize)) {
            DAWN_INVALID_IF(bindingMap.contains(usedBinding),
                            "On entries[%u]: binding index (%u) was specified by a previous entry.",
                            i, entry->binding);
            bindingMap.insert({usedBinding, i});
        }

        DAWN_TRY_CONTEXT(ValidateBindGroupLayoutEntry(device, entry, allowInternalBinding),
                         "validating entries[%u]", i);

        IncrementBindingCounts(&bindingCounts, entry);
    }

    // Perform a second validation pass for static samplers. This is done after initial validation
    // as static samplers can have associated texture entries that need to be validated first.
    DAWN_TRY(ValidateStaticSamplersWithTextureBindings(device, descriptor, bindingMap));

    DAWN_TRY_CONTEXT(
        ValidateBindingCounts(device->GetLimits(), bindingCounts, device->GetAdapter()),
        "validating binding counts");

    return descriptor;
}

namespace {

BindingInfo CreateSampledTextureBindingForExternalTexture(BindingNumber binding,
                                                          wgpu::ShaderStage visibility) {
    return {
        .binding = binding,
        .visibility = visibility,
        .bindingLayout = TextureBindingInfo{{
            .sampleType = wgpu::TextureSampleType::Float,
            .viewDimension = wgpu::TextureViewDimension::e2D,
            .multisampled = false,
        }},
    };
}

BindingInfo CreateUniformBindingForExternalTexture(BindingNumber binding,
                                                   wgpu::ShaderStage visibility) {
    return {
        .binding = binding,
        .visibility = visibility,
        .bindingLayout = BufferBindingInfo{{
            .type = wgpu::BufferBindingType::Uniform,
            .minBindingSize = 0,
            .hasDynamicOffset = false,
        }},
    };
}

BindingInfo ConvertToBindingInfo(const UnpackedPtr<BindGroupLayoutEntry>& binding) {
    BindingInfo bindingInfo;
    bindingInfo.binding = BindingNumber(binding->binding);
    bindingInfo.visibility = binding->visibility;
    bindingInfo.arraySize = BindingIndex(std::max(1u, binding->bindingArraySize));

    if (binding->buffer.type != wgpu::BufferBindingType::BindingNotUsed) {
        bindingInfo.bindingLayout = BufferBindingInfo::From(binding->buffer);
    } else if (binding->sampler.type != wgpu::SamplerBindingType::BindingNotUsed) {
        bindingInfo.bindingLayout = SamplerBindingInfo::From(binding->sampler);
    } else if (binding->texture.sampleType != wgpu::TextureSampleType::BindingNotUsed) {
        auto textureBindingInfo = TextureBindingInfo::From(binding->texture);
        if (binding->texture.viewDimension == kInternalInputAttachmentDim) {
            bindingInfo.bindingLayout = InputAttachmentBindingInfo{{textureBindingInfo.sampleType}};
        } else {
            bindingInfo.bindingLayout = textureBindingInfo;
        }
    } else if (binding->storageTexture.access != wgpu::StorageTextureAccess::BindingNotUsed) {
        bindingInfo.bindingLayout = StorageTextureBindingInfo::From(binding->storageTexture);
    } else if (auto* texelBufferLayout = binding.Get<TexelBufferBindingLayout>()) {
        // TODO(382544164): Prototype texel buffer feature.
        // Placeholder implementation for `TexelBufferBindingLayout` from `TexelBufferBindingInfo`.
        BufferBindingInfo bufferInfo{};
        switch (texelBufferLayout->access) {
            case wgpu::TexelBufferAccess::ReadOnly:
                bufferInfo.type = wgpu::BufferBindingType::ReadOnlyStorage;
                break;
            case wgpu::TexelBufferAccess::ReadWrite:
                bufferInfo.type = wgpu::BufferBindingType::Storage;
                break;
            default:
                DAWN_UNREACHABLE();
        }
        bufferInfo.minBindingSize = 0;
        bufferInfo.hasDynamicOffset = false;
        bindingInfo.bindingLayout = bufferInfo;
    } else if (auto* staticSamplerBindingLayout = binding.Get<StaticSamplerBindingLayout>()) {
        bindingInfo.bindingLayout = StaticSamplerBindingInfo::From(*staticSamplerBindingLayout);
    } else {
        DAWN_UNREACHABLE();
    }

    return bindingInfo;
}

// This function handles the conversion of the API format for each binding info to Dawn's internal
// representation of them. This is also where the ExternalTextures are replaced and expanded in the
// various bindings that are used internally in Dawn. Arrays are also expanded to individual
// bindings here.
struct ExpandedBindingInfo {
    ityp::vector<BindingIndex, BindingInfo> entries;
    ExternalTextureBindingExpansionMap externalTextureBindingExpansions;
    std::optional<BindingNumber> dynamicArrayMetatada;
};
ExpandedBindingInfo ConvertAndExpandBGLEntries(
    const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor) {
    ExpandedBindingInfo result;

    // When new BGL entries are created, we use binding numbers decreasing from the max uint32_t
    // to ensure there are no collisions and that validation will prevent using these BindingNumbers
    // when creating a bindgroup (so there is no risk of applications injecting their own buffer for
    // the metadata for example).
    // New BGL entries that are added internally we must ensure that there are no collisions with
    // existing BindingNumbers and that validation will prevent using these BindingNumbers when
    // creating a bindgroup. Otherwise a misbehaved application could try injecting its own planes
    // for ExternalTextures or its own metadata buffer for dynamic binding array metadata.
    //
    // TODO(https://crbug.com/42240282): The scheme for ExternalTextures is to generate increasing
    // BindingNumbers from maxBindingsPerBindGroup. This won't work with dynamic binding arrays as
    // these can contain BindingNumbers way past maxBindingsPerBindGroup. Instead change to use the
    // scheme decreasing from UINT32_MAX. (this couldn't be done immediately due to failures on
    // D3D12).
    // TODO(https://crbug.com/435317394): Changing the scheme will allow re-enabling use in
    // ExternalTextures in bind group with dynamic binding arrays.
    BindingNumber nextOpenBindingNumberForNewEntryET = kMaxBindingsPerBindGroupTyped;
    BindingNumber nextOpenBindingNumberForNewEntryNonET = std::numeric_limits<BindingNumber>::max();
    for (uint32_t i = 0; i < descriptor->entryCount; i++) {
        UnpackedPtr<BindGroupLayoutEntry> entry = Unpack(&descriptor->entries[i]);

        // External textures are expanded from a texture_external into two sampled texture bindings
        // and one uniform buffer binding. The original binding number is used for the first sampled
        // texture.
        if (entry.Get<ExternalTextureBindingLayout>()) {
            DAWN_ASSERT(entry->bindingArraySize <= 1);
            dawn::native::ExternalTextureBindingExpansion bindingExpansion;

            BindingInfo plane0Entry = CreateSampledTextureBindingForExternalTexture(
                BindingNumber(entry->binding), entry->visibility);
            bindingExpansion.plane0 = BindingNumber(plane0Entry.binding);
            result.entries.push_back(plane0Entry);

            BindingInfo plane1Entry = CreateSampledTextureBindingForExternalTexture(
                nextOpenBindingNumberForNewEntryET++, entry->visibility);
            bindingExpansion.plane1 = BindingNumber(plane1Entry.binding);
            result.entries.push_back(plane1Entry);

            BindingInfo paramsEntry = CreateUniformBindingForExternalTexture(
                nextOpenBindingNumberForNewEntryET++, entry->visibility);
            bindingExpansion.params = BindingNumber(paramsEntry.binding);
            result.entries.push_back(paramsEntry);

            result.externalTextureBindingExpansions.insert(
                {BindingNumber(entry->binding), bindingExpansion});
            continue;
        }

        // Add one BindingInfo per element of the array with increasing indexInArray for backends to
        // know which element it is when they need it, but also with increasing BindingNumber as the
        // array takes consecutive binding numbers on the API side.
        BindingInfo info = ConvertToBindingInfo(entry);
        for (BindingIndex indexInArray : Range(info.arraySize)) {
            info.indexInArray = indexInArray;
            result.entries.push_back(info);
            info.binding++;
        }
    }

    // Add an internal entry for the metadata buffer of the dynamic array if needed.
    if (descriptor.Has<BindGroupLayoutDynamicBindingArray>()) {
        BindingInfo metadataEntry = {
            .binding = nextOpenBindingNumberForNewEntryNonET--,
            .visibility = kAllStages,
            .bindingLayout = BufferBindingInfo{{
                .type = wgpu::BufferBindingType::ReadOnlyStorage,
                // This is an internal buffer so it doesn't need its minBindingSize runtime-checked.
                .minBindingSize = 4,
                .hasDynamicOffset = false,
            }}};
        result.entries.push_back(metadataEntry);
        result.dynamicArrayMetatada = metadataEntry.binding;
    }

    return result;
}

// This is a utility function to help DAWN_ASSERT that the BGL-binding comparator places buffers
// first.
bool CheckBufferBindingsFirst(ityp::span<BindingIndex, const BindingInfo> bindings) {
    BindingIndex lastBufferIndex{0};
    BindingIndex firstNonBufferIndex = std::numeric_limits<BindingIndex>::max();
    for (auto [i, binding] : Enumerate(bindings)) {
        if (std::holds_alternative<BufferBindingInfo>(binding.bindingLayout)) {
            lastBufferIndex = std::max(i, lastBufferIndex);
        } else {
            firstNonBufferIndex = std::min(i, firstNonBufferIndex);
        }
    }

    // If there are no buffers, then |lastBufferIndex| is initialized to 0 and
    // |firstNonBufferIndex| gets set to 0.
    return firstNonBufferIndex >= lastBufferIndex;
}

}  // namespace

// BindGroupLayoutInternalBase

BindGroupLayoutInternalBase::BindGroupLayoutInternalBase(
    DeviceBase* device,
    const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor,
    ApiObjectBase::UntrackedByDeviceTag tag)
    : ApiObjectBase(device, descriptor->label) {
    ExpandedBindingInfo unpackedBindings = ConvertAndExpandBGLEntries(descriptor);
    mExternalTextureBindingExpansionMap =
        std::move(unpackedBindings.externalTextureBindingExpansions);
    mBindingInfo = std::move(unpackedBindings.entries);

    // Reorder bindings internally and compute the BindingNumber->BindingIndex map.
    std::sort(mBindingInfo.begin(), mBindingInfo.end(), SortBindingsCompare);
    for (const auto [i, binding] : Enumerate(mBindingInfo)) {
        const auto& [_, inserted] = mBindingMap.emplace(binding.binding, i);
        DAWN_ASSERT(inserted);
    }

    DAWN_ASSERT(CheckBufferBindingsFirst({mBindingInfo.data(), GetBindingCount()}));
    DAWN_ASSERT(mBindingInfo.size() <= kMaxBindingsPerPipelineLayoutTyped);

    // Compute various counts of expanded bindings and other metadata.
    std::array<BindingIndex, Order_Count + 1> counts{};
    for (const auto& binding : mBindingInfo) {
        MatchVariant(
            binding.bindingLayout,
            [&](const BufferBindingInfo& layout) {
                if (layout.minBindingSize == 0) {
                    mUnverifiedBufferCount++;
                }
                if (layout.hasDynamicOffset) {
                    counts[Order_DynamicBuffer]++;
                    switch (layout.type) {
                        case wgpu::BufferBindingType::Storage:
                        case kInternalStorageBufferBinding:
                        case kInternalReadOnlyStorageBufferBinding:
                        case wgpu::BufferBindingType::ReadOnlyStorage:
                            mDynamicStorageBufferCount++;
                            break;

                        case wgpu::BufferBindingType::Uniform:
                        case wgpu::BufferBindingType::BindingNotUsed:
                        case wgpu::BufferBindingType::Undefined:
                            break;
                    }
                } else {
                    counts[Order_RegularBuffer]++;
                }
            },
            [&](const TextureBindingInfo&) { counts[Order_SampledTexture]++; },
            [&](const StorageTextureBindingInfo&) { counts[Order_StorageTexture]++; },
            [&](const SamplerBindingInfo&) { counts[Order_RegularSampler]++; },
            [&](const StaticSamplerBindingInfo& layout) {
                counts[Order_StaticSampler]++;
                if (layout.isUsedForSingleTextureBinding) {
                    mNeedsCrossBindingValidation = true;
                }
            },
            [&](const InputAttachmentBindingInfo&) { counts[Order_InputAttachment]++; });
    }

    // Do a prefix sum to store the start offset of each binding type.
    BindingIndex sum{0};
    for (auto [type, count] : Enumerate(counts)) {
        mBindingTypeStart[type] = sum;
        sum += count;
    }

    // Recompute the number of bindings of each type from the descriptor since that is used for
    // validation of the pipeline layout.
    for (uint32_t i = 0; i < descriptor->entryCount; i++) {
        UnpackedPtr<BindGroupLayoutEntry> entry = Unpack(&descriptor->entries[i]);
        IncrementBindingCounts(&mValidationBindingCounts, entry);
    }

    // Handle the dynamic binding array if there is one.
    if (auto* dynamic = descriptor.Get<BindGroupLayoutDynamicBindingArray>()) {
        mHasDynamicArray = true;
        mAPIDynamicArrayStart = BindingNumber(dynamic->dynamicArray.start);
        mDynamicArrayKind = dynamic->dynamicArray.kind;
        mDynamicArrayMetadataBinding = mBindingMap[unpackedBindings.dynamicArrayMetatada.value()];

        // Pack the dynamic array to start right after static bindings.
        mDynamicArrayStart = mBindingInfo.size();
    }
}

// static
bool BindGroupLayoutInternalBase::SortBindingsCompare(const BindingInfo& a, const BindingInfo& b) {
    if (&a == &b) {
        return false;
    }

    // Buffers with dynamic offsets come first and then the rest of the buffers. Other bindings are
    // only grouped by types. This is to make it easier and faster to handle them.
    auto TypeOrder = [](const BindingInfo& info) {
        return MatchVariant(
            info.bindingLayout,
            [&](const BufferBindingInfo& layout) {
                return layout.hasDynamicOffset ? Order_DynamicBuffer : Order_RegularBuffer;
            },
            [&](const TextureBindingInfo&) { return Order_SampledTexture; },
            [&](const StorageTextureBindingInfo&) { return Order_StorageTexture; },
            [&](const SamplerBindingInfo&) { return Order_RegularSampler; },
            [&](const StaticSamplerBindingInfo&) { return Order_StaticSampler; },
            [&](const InputAttachmentBindingInfo&) { return Order_InputAttachment; });
    };

    auto aOrder = TypeOrder(a);
    auto bOrder = TypeOrder(b);
    if (aOrder != bOrder) {
        return aOrder < bOrder;
    }

    // Afterwards sort the bindings by binding number. This is necessary because dynamic buffers
    // are applied in order of increasing binding number in SetBindGroup.
    return a.binding < b.binding;
}

BindGroupLayoutInternalBase::BindGroupLayoutInternalBase(
    DeviceBase* device,
    const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor)
    : BindGroupLayoutInternalBase(device, descriptor, kUntrackedByDevice) {
    GetObjectTrackingList()->Track(this);
}

BindGroupLayoutInternalBase::BindGroupLayoutInternalBase(DeviceBase* device,
                                                         ObjectBase::ErrorTag tag,
                                                         StringView label)
    : ApiObjectBase(device, tag, label) {}

BindGroupLayoutInternalBase::~BindGroupLayoutInternalBase() = default;

void BindGroupLayoutInternalBase::DestroyImpl() {
    Uncache();
}

ObjectType BindGroupLayoutInternalBase::GetType() const {
    return ObjectType::BindGroupLayoutInternal;
}

const BindingInfo& BindGroupLayoutInternalBase::GetBindingInfo(BindingIndex bindingIndex) const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(bindingIndex < mBindingInfo.size());
    return mBindingInfo[bindingIndex];
}

const BindGroupLayoutInternalBase::BindingMap& BindGroupLayoutInternalBase::GetBindingMap() const {
    DAWN_ASSERT(!IsError());
    return mBindingMap;
}

BindingIndex BindGroupLayoutInternalBase::GetBindingIndex(BindingNumber bindingNumber) const {
    DAWN_ASSERT(!IsError());
    const auto& it = mBindingMap.find(bindingNumber);
    DAWN_ASSERT(it != mBindingMap.end());
    return it->second;
}

bool BindGroupLayoutInternalBase::HasDynamicArray() const {
    DAWN_ASSERT(!IsError());
    return mHasDynamicArray;
}

BindingNumber BindGroupLayoutInternalBase::GetAPIDynamicArrayStart() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mHasDynamicArray);
    return mAPIDynamicArrayStart;
}

BindingIndex BindGroupLayoutInternalBase::GetDynamicArrayStart() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mHasDynamicArray);
    return mDynamicArrayStart;
}

BindingIndex BindGroupLayoutInternalBase::GetDynamicBindingIndex(BindingNumber binding) const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mHasDynamicArray);
    DAWN_ASSERT(binding >= mAPIDynamicArrayStart);
    uint32_t indexInArray = uint32_t(binding - mAPIDynamicArrayStart);
    return BindingIndex(indexInArray);
}

BindingIndex BindGroupLayoutInternalBase::GetDynamicArrayMetadataBinding() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mHasDynamicArray);
    return mDynamicArrayMetadataBinding;
}

wgpu::DynamicBindingKind BindGroupLayoutInternalBase::GetDynamicArrayKind() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mHasDynamicArray);
    return mDynamicArrayKind;
}

void BindGroupLayoutInternalBase::ReduceMemoryUsage() {}

size_t BindGroupLayoutInternalBase::ComputeContentHash() {
    ObjectContentHasher recorder;

    // std::map is sorted by key, so two BGLs constructed in different orders
    // will still record the same.
    for (const auto [id, index] : mBindingMap) {
        recorder.Record(id, index);

        const BindingInfo& info = mBindingInfo[index];
        recorder.Record(info.visibility);
        recorder.Record(info.arraySize);
        recorder.Record(info.indexInArray);

        MatchVariant(
            info.bindingLayout,
            [&](const BufferBindingInfo& layout) {
                recorder.Record(BindingInfoType::Buffer, layout.hasDynamicOffset, layout.type,
                                layout.minBindingSize);
            },
            [&](const SamplerBindingInfo& layout) {
                recorder.Record(BindingInfoType::Sampler, layout.type);
            },
            [&](const TextureBindingInfo& layout) {
                recorder.Record(BindingInfoType::Texture, layout.sampleType, layout.viewDimension,
                                layout.multisampled);
            },
            [&](const StorageTextureBindingInfo& layout) {
                recorder.Record(BindingInfoType::StorageTexture, layout.access, layout.format,
                                layout.viewDimension);
            },
            [&](const StaticSamplerBindingInfo& layout) {
                recorder.Record(BindingInfoType::StaticSampler, layout.sampler->GetContentHash());
            },
            [&](const InputAttachmentBindingInfo& layout) {
                recorder.Record(BindingInfoType::InputAttachment, layout.sampleType);
            });
    }

    recorder.Record(mHasDynamicArray, mAPIDynamicArrayStart, mDynamicArrayKind);

    return recorder.GetContentHash();
}

bool BindGroupLayoutInternalBase::EqualityFunc::operator()(
    const BindGroupLayoutInternalBase* a,
    const BindGroupLayoutInternalBase* b) const {
    if (a->GetBindingCount() != b->GetBindingCount()) {
        return false;
    }
    for (BindingIndex i{0}; i < a->GetBindingCount(); ++i) {
        if (a->mBindingInfo[i] != b->mBindingInfo[i]) {
            return false;
        }
    }
    if (a->mBindingMap != b->mBindingMap) {
        return false;
    }

    if (a->mHasDynamicArray != b->mHasDynamicArray ||
        a->mDynamicArrayKind != b->mDynamicArrayKind ||
        a->mAPIDynamicArrayStart != b->mAPIDynamicArrayStart) {
        return false;
    }
    return true;
}

bool BindGroupLayoutInternalBase::IsEmpty() const {
    DAWN_ASSERT(!IsError());
    return mBindingInfo.empty() && !mHasDynamicArray;
}

BindingIndex BindGroupLayoutInternalBase::GetBindingCount() const {
    DAWN_ASSERT(!IsError());
    return mBindingInfo.size();
}

BindingIndex BindGroupLayoutInternalBase::GetDynamicBufferCount() const {
    DAWN_ASSERT(!IsError());
    return GetBindingTypeEnd(Order_DynamicBuffer) - GetBindingTypeStart(Order_DynamicBuffer);
}

uint32_t BindGroupLayoutInternalBase::GetDynamicStorageBufferCount() const {
    DAWN_ASSERT(!IsError());
    return mDynamicStorageBufferCount;
}

uint32_t BindGroupLayoutInternalBase::GetUnverifiedBufferCount() const {
    DAWN_ASSERT(!IsError());
    return mUnverifiedBufferCount;
}

uint32_t BindGroupLayoutInternalBase::GetStaticSamplerCount() const {
    DAWN_ASSERT(!IsError());
    return uint32_t(GetBindingTypeEnd(Order_StaticSampler) -
                    GetBindingTypeStart(Order_StaticSampler));
}

const BindingCounts& BindGroupLayoutInternalBase::GetValidationBindingCounts() const {
    DAWN_ASSERT(!IsError());
    return mValidationBindingCounts;
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetDynamicBufferIndices() const {
    return Range(GetBindingTypeStart(Order_DynamicBuffer), GetBindingTypeEnd(Order_DynamicBuffer));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetBufferIndices() const {
    return Range(GetBindingTypeStart(Order_DynamicBuffer), GetBindingTypeEnd(Order_RegularBuffer));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetStorageTextureIndices() const {
    return Range(GetBindingTypeStart(Order_StorageTexture),
                 GetBindingTypeEnd(Order_StorageTexture));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetSampledTextureIndices() const {
    return Range(GetBindingTypeStart(Order_SampledTexture),
                 GetBindingTypeEnd(Order_SampledTexture));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetTextureIndices() const {
    return Range(GetBindingTypeStart(Order_SampledTexture),
                 GetBindingTypeEnd(Order_InputAttachment));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetSamplerIndices() const {
    return Range(GetBindingTypeStart(Order_StaticSampler), GetBindingTypeEnd(Order_RegularSampler));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetNonStaticSamplerIndices() const {
    return Range(GetBindingTypeStart(Order_RegularSampler),
                 GetBindingTypeEnd(Order_RegularSampler));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetInputAttachmentIndices() const {
    return Range(GetBindingTypeStart(Order_InputAttachment),
                 GetBindingTypeEnd(Order_InputAttachment));
}

const ExternalTextureBindingExpansionMap&
BindGroupLayoutInternalBase::GetExternalTextureBindingExpansionMap() const {
    DAWN_ASSERT(!IsError());
    return mExternalTextureBindingExpansionMap;
}

bool BindGroupLayoutInternalBase::NeedsCrossBindingValidation() const {
    DAWN_ASSERT(!IsError());
    return mNeedsCrossBindingValidation;
}

uint32_t BindGroupLayoutInternalBase::GetUnexpandedBindingCount() const {
    DAWN_ASSERT(!IsError());
    return mValidationBindingCounts.totalCount;
}

size_t BindGroupLayoutInternalBase::GetBindingDataSize() const {
    DAWN_ASSERT(!IsError());
    // | ------ buffer-specific ----------| ------------ object pointers -------------|
    // | --- offsets + sizes -------------| --------------- Ref<ObjectBase> ----------|
    // Followed by:
    // |---------buffer size array--------|
    // |-uint64_t[mUnverifiedBufferCount]-|
    const size_t bufferCount = size_t(GetBindingTypeEnd(Order_RegularBuffer));
    const size_t bindingCount = size_t(mBindingInfo.size());

    size_t objectPointerStart = bufferCount * sizeof(BufferBindingData);
    DAWN_ASSERT(IsAligned(objectPointerStart, alignof(Ref<ObjectBase>)));
    size_t bufferSizeArrayStart =
        Align(objectPointerStart + bindingCount * sizeof(Ref<ObjectBase>), sizeof(uint64_t));
    DAWN_ASSERT(IsAligned(bufferSizeArrayStart, alignof(uint64_t)));
    return bufferSizeArrayStart + mUnverifiedBufferCount * sizeof(uint64_t);
}

BindGroupLayoutInternalBase::BindingDataPointers
BindGroupLayoutInternalBase::ComputeBindingDataPointers(void* dataStart) const {
    const size_t bufferCount = size_t(GetBindingTypeEnd(Order_RegularBuffer));
    const size_t bindingCount = size_t(mBindingInfo.size());

    BufferBindingData* bufferData = reinterpret_cast<BufferBindingData*>(dataStart);
    auto bindings = reinterpret_cast<Ref<ObjectBase>*>(bufferData + bufferCount);
    uint64_t* unverifiedBufferSizes =
        AlignPtr(reinterpret_cast<uint64_t*>(bindings + bindingCount), sizeof(uint64_t));

    DAWN_ASSERT(IsPtrAligned(bufferData, alignof(BufferBindingData)));
    DAWN_ASSERT(IsPtrAligned(bindings, alignof(Ref<ObjectBase>)));
    DAWN_ASSERT(IsPtrAligned(unverifiedBufferSizes, alignof(uint64_t)));

    return {{bufferData, GetBindingTypeEnd(Order_RegularBuffer)},
            {bindings, GetBindingCount()},
            {unverifiedBufferSizes, mUnverifiedBufferCount}};
}

bool BindGroupLayoutInternalBase::IsStorageBufferBinding(BindingIndex bindingIndex) const {
    switch (std::get<BufferBindingInfo>(GetBindingInfo(bindingIndex).bindingLayout).type) {
        case wgpu::BufferBindingType::Uniform:
            return false;
        case kInternalStorageBufferBinding:
        case kInternalReadOnlyStorageBufferBinding:
        case wgpu::BufferBindingType::Storage:
        case wgpu::BufferBindingType::ReadOnlyStorage:
            return true;
        case wgpu::BufferBindingType::BindingNotUsed:
        case wgpu::BufferBindingType::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

std::string BindGroupLayoutInternalBase::EntriesToString() const {
    std::string entries = "[";
    std::string sep = "";
    const BindGroupLayoutInternalBase::BindingMap& bindingMap = GetBindingMap();
    for (const auto [bindingNumber, bindingIndex] : bindingMap) {
        const BindingInfo& bindingInfo = GetBindingInfo(bindingIndex);
        entries += absl::StrFormat("%s%s", sep, bindingInfo);
        sep = ", ";
    }
    entries += "]";
    return entries;
}

BindingIndex BindGroupLayoutInternalBase::GetBindingTypeStart(BindingTypeOrder type) const {
    return mBindingTypeStart[type];
}

BindingIndex BindGroupLayoutInternalBase::GetBindingTypeEnd(BindingTypeOrder type) const {
    return mBindingTypeStart[BindingTypeOrder(static_cast<uint32_t>(type) + 1)];
}

}  // namespace dawn::native
