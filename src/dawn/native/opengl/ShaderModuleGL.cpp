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

#include "dawn/native/opengl/ShaderModuleGL.h"

#include <sstream>
#include <unordered_map>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/Enumerator.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/CacheRequest.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/opengl/BindGroupLayoutGL.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/PipelineGL.h"
#include "dawn/native/opengl/PipelineLayoutGL.h"
#include "dawn/native/opengl/UtilsGL.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"
#include "tint/api/common/binding_point.h"

namespace dawn::native::opengl {
namespace {
using InterstageLocationAndName = std::pair<uint32_t, std::string>;
using SubstituteOverrideConfig = std::unordered_map<tint::OverrideId, double>;

#define GLSL_COMPILATION_REQUEST_MEMBERS(X)                                          \
    X(ShaderModuleBase::ShaderModuleHash, shaderModuleHash)                          \
    X(UnsafeUnserializedValue<ShaderModuleBase::ScopedUseTintProgram>, inputProgram) \
    X(std::string, entryPointName)                                                   \
    X(SingleShaderStage, stage)                                                      \
    X(SubstituteOverrideConfig, substituteOverrideConfig)                            \
    X(LimitsForCompilationRequest, limits)                                           \
    X(UnsafeUnserializedValue<LimitsForCompilationRequest>, adapterSupportedLimits)  \
    X(bool, disableSymbolRenaming)                                                   \
    X(std::vector<InterstageLocationAndName>, interstageVariables)                   \
    X(tint::glsl::writer::Options, tintOptions)                                      \
    X(UnsafeUnserializedValue<dawn::platform::Platform*>, platform)
DAWN_MAKE_CACHE_REQUEST(GLSLCompilationRequest, GLSL_COMPILATION_REQUEST_MEMBERS);
#undef GLSL_COMPILATION_REQUEST_MEMBERS

#define GLSL_COMPILATION_MEMBERS(X) X(std::string, glsl)
DAWN_SERIALIZABLE(struct, GLSLCompilation, GLSL_COMPILATION_MEMBERS) {
    static ResultOrError<GLSLCompilation> FromValidatedBlob(Blob blob);
};
#undef GLSL_COMPILATION_MEMBERS

// Separated from the class definition above to fix a clang-format over-indentation.
// static
ResultOrError<GLSLCompilation> GLSLCompilation::FromValidatedBlob(Blob blob) {
    GLSLCompilation result;
    DAWN_TRY_ASSIGN(result, FromBlob(std::move(blob)));
    DAWN_INVALID_IF(result.glsl.empty(), "Cached GLSLCompilation result has no GLSL");
    return result;
}

GLenum GLShaderType(SingleShaderStage stage) {
    switch (stage) {
        case SingleShaderStage::Vertex:
            return GL_VERTEX_SHADER;
        case SingleShaderStage::Fragment:
            return GL_FRAGMENT_SHADER;
        case SingleShaderStage::Compute:
            return GL_COMPUTE_SHADER;
    }
    DAWN_UNREACHABLE();
}

tint::glsl::writer::Version::Standard ToTintGLStandard(opengl::OpenGLVersion::Standard standard) {
    switch (standard) {
        case OpenGLVersion::Standard::Desktop:
            return tint::glsl::writer::Version::Standard::kDesktop;
        case OpenGLVersion::Standard::ES:
            return tint::glsl::writer::Version::Standard::kES;
    }
    DAWN_UNREACHABLE();
}

// Returns information about the texture/sampler pairs used by the entry point. This is necessary
// because GL uses combined texture/sampler bindings while WGSL allows mixing and matching textures
// and samplers in the shader. GL also uses a placeholder sampler to use with textures when they
// aren't combined with a sampler in the WGSL.
//
// Another subtlety is that Dawn uses pre-remapping BindingPoints when referring to bindings while
// the Tint GLSL writer uses post-remapping BindingPoints.
void GenerateCombinedSamplerInfo(
    const EntryPointMetadata& metadata,
    const tint::Bindings& bindings,
    const PipelineLayout* layout,
    std::vector<CombinedSampler>* combinedSamplers,
    tint::glsl::writer::CombinedTextureSamplerInfo* samplerTextureToName,
    tint::BindingPoint* placeholder_sampler_bind_point) {
    // Helper to avoid duplicated logic for when a CombinedSampler is determined. It takes a bunch
    // of information for both the texture and the sampler and translate to what Dawn/Tint need.
    struct CombinedBindingInfo {
        // Dawn takes BindGroupIndex + BindingIndex.
        BindGroupIndex group;
        BindingIndex index;
        BindingIndex shaderArraySize = BindingIndex(1);

        // Tint takes the post-remapping binding point.
        tint::BindingPoint remappedBinding;
    };
    auto AddCombinedSampler = [&](CombinedBindingInfo texture,
                                  std::optional<CombinedBindingInfo> sampler,
                                  bool isPlane1 = false) {
        // Reflect to the pipeline the combination with BindGroupIndex + BindingIndex in that BGL.
        CombinedSampler combinedSampler = {{
            .samplerLocation = std::nullopt,
            .textureLocation = {{
                .group = texture.group,
                .index = texture.index,
                .shaderArraySize = texture.shaderArraySize,
            }},
        }};
        if (sampler.has_value()) {
            combinedSampler.samplerLocation = {{{
                .group = sampler->group,
                .index = sampler->index,
                .shaderArraySize = sampler->shaderArraySize,
            }}};
        }
        combinedSamplers->push_back(combinedSampler);

        // Let Tint know to generate a new GLSL sampler for this combination.
        tint::BindingPoint samplerRemapped = *placeholder_sampler_bind_point;
        if (sampler.has_value()) {
            samplerRemapped = {0, sampler->remappedBinding.binding};
        }
        samplerTextureToName->emplace(
            tint::glsl::writer::CombinedTextureSamplerPair{
                {0, texture.remappedBinding.binding}, samplerRemapped, isPlane1},
            combinedSampler.GetName());
    };

    for (const auto& use : metadata.samplerAndNonSamplerTexturePairs) {
        // Replace uses of the placeholder sampler with its actual binding point.
        std::optional<CombinedBindingInfo> sampler = std::nullopt;
        if (use.sampler != EntryPointMetadata::nonSamplerBindingPoint) {
            const BindGroupLayoutInternalBase* bgl = layout->GetBindGroupLayout(use.sampler.group);
            sampler = {
                .group = use.sampler.group,
                .index = bgl->AsBindingIndex(bgl->GetBindingMap().at(use.sampler.binding)),
                .remappedBinding = bindings.sampler.at(ToTint(use.sampler)),
            };
        }

        // Tint reflection returns information about uses of both regular textures and sampled
        // textures so we need to differentiate both cases here.
        const BindGroupLayoutInternalBase* bgl = layout->GetBindGroupLayout(use.texture.group);
        APIBindingIndex textureAPIIndex = bgl->GetBindingMap().at(use.texture.binding);
        const auto& bindingInfo = bgl->GetAPIBindingInfo(textureAPIIndex);

        // The easy case is when a regular texture is being handled.
        if (std::holds_alternative<TextureBindingInfo>(bindingInfo.bindingLayout)) {
            CombinedBindingInfo texture = {
                .group = use.texture.group,
                .index = bgl->AsBindingIndex(textureAPIIndex),
                .shaderArraySize =
                    metadata.bindings.at(use.texture.group).at(use.texture.binding).arraySize,
                .remappedBinding = bindings.texture.at(ToTint(use.texture)),
            };
            AddCombinedSampler(texture, sampler);
            continue;
        }

        // This is an external texture, add planes individually.
        const auto& bindingLayout = std::get<ExternalTextureBindingInfo>(bindingInfo.bindingLayout);

        CombinedBindingInfo plane0 = {
            .group = use.texture.group,
            .index = bindingLayout.plane0,
            .remappedBinding = bindings.external_texture.at(ToTint(use.texture)).plane0,
        };
        AddCombinedSampler(plane0, sampler, false);

        CombinedBindingInfo plane1 = {
            .group = use.texture.group,
            .index = bindingLayout.plane1,
            .remappedBinding = bindings.external_texture.at(ToTint(use.texture)).plane1,
        };
        AddCombinedSampler(plane1, sampler, true);
    }
}

// Returns whether the stage uses any texture builtin metadata.
void GenerateTextureBuiltinFromUniformData(
    const EntryPointMetadata& metadata,
    const PipelineLayout* layout,
    const tint::Bindings& bindings,
    EmulatedTextureBuiltinRegistrar* emulatedTextureBuiltins,
    tint::glsl::writer::TextureBuiltinsFromUniformOptions* textureBuiltinsFromUniform) {
    // Tell Tint where the uniform containing the builtin data will be (in post-remapping space),
    // only when this shader stage uses some builtin metadata.
    if (!metadata.textureQueries.empty()) {
        textureBuiltinsFromUniform->ubo_binding = {
            .group = 0,
            .binding = uint32_t(layout->GetInternalTextureBuiltinsUniformBinding()),
        };
    }

    for (auto [i, query] : Enumerate(metadata.textureQueries)) {
        BindGroupIndex group = BindGroupIndex(query.group);
        const auto* bgl = layout->GetBindGroupLayout(group);
        BindingIndex binding =
            bgl->AsBindingIndex(bgl->GetAPIBindingIndex(BindingNumber{query.binding}));

        // Register that the query needs to be emulated and get the offset in the UBO where the data
        // will be passed.
        TextureQuery textureQuery;
        switch (query.type) {
            case EntryPointMetadata::TextureMetadataQuery::TextureQueryType::TextureNumLevels:
                textureQuery = TextureQuery::NumLevels;
                break;
            case EntryPointMetadata::TextureMetadataQuery::TextureQueryType::TextureNumSamples:
                textureQuery = TextureQuery::NumSamples;
                break;
        }
        uint32_t offset = emulatedTextureBuiltins->Register(group, binding, textureQuery);

        // Tint uses post-remapping binding points for textureBuiltinFromUniform options.
        tint::BindingPoint wgslBindPoint = {.group = query.group, .binding = query.binding};

        tint::BindingPoint remappedBinding;
        if (bindings.texture.contains(wgslBindPoint)) {
            remappedBinding = bindings.texture.at(wgslBindPoint);
        } else {
            remappedBinding = bindings.storage_texture.at(wgslBindPoint);
        }
        textureBuiltinsFromUniform->ubo_contents.push_back({
            .offset = offset,
            .count = 1,
            .binding = remappedBinding,
        });
    }
}

bool GenerateArrayLengthFromuniformData(
    const BindingInfoArray& moduleBindingInfo,
    const PipelineLayout* layout,
    tint::glsl::writer::ArrayLengthFromUniformOptions& options) {
    const PipelineLayout::BindingIndexInfo& indexInfo = layout->GetBindingIndexInfo();

    for (BindGroupIndex group : layout->GetBindGroupLayoutsMask()) {
        const BindGroupLayoutInternalBase* bgl = layout->GetBindGroupLayout(group);

        for (BindingIndex binding : bgl->GetBufferIndices()) {
            const BindingInfo& bindingInfo = bgl->GetBindingInfo(binding);

            switch (std::get<BufferBindingInfo>(bindingInfo.bindingLayout).type) {
                case wgpu::BufferBindingType::Storage:
                case kInternalStorageBufferBinding:
                case wgpu::BufferBindingType::ReadOnlyStorage:
                case kInternalReadOnlyStorageBufferBinding: {
                    // Use ssbo index as the indices for the buffer size lookups
                    // in the array length from uniform transform.
                    tint::BindingPoint srcBindingPoint = {uint32_t(group),
                                                          uint32_t(bindingInfo.binding)};
                    FlatBindingIndex ssboIndex = indexInfo[group][binding];
                    options.bindpoint_to_size_index.emplace(srcBindingPoint, uint32_t(ssboIndex));
                    break;
                }
                default:
                    break;
            }
        }
    }

    return options.bindpoint_to_size_index.size() > 0;
}

}  // namespace

std::string GetBindingName(BindGroupIndex group, BindingNumber bindingNumber) {
    std::ostringstream o;
    o << "dawn_binding_" << static_cast<uint32_t>(group) << "_"
      << static_cast<uint32_t>(bindingNumber);
    return o.str();
}

bool operator<(const CombinedSamplerElement& a, const CombinedSamplerElement& b) {
    return std::tie(a.group, a.index, a.shaderArraySize) <
           std::tie(b.group, b.index, b.shaderArraySize);
}

bool operator<(const CombinedSampler& a, const CombinedSampler& b) {
    return std::tie(a.samplerLocation, a.textureLocation) <
           std::tie(b.samplerLocation, b.textureLocation);
}

std::string CombinedSampler::GetName() const {
    std::ostringstream o;
    o << "dawn_combined";
    if (!samplerLocation) {
        o << "_placeholder_sampler";
    } else {
        o << "_" << static_cast<uint32_t>(samplerLocation->group) << "_"
          << static_cast<uint32_t>(samplerLocation->index);
    }
    o << "_with_" << static_cast<uint32_t>(textureLocation.group) << "_"
      << static_cast<uint32_t>(textureLocation.index);
    return o.str();
}

// static
ResultOrError<Ref<ShaderModule>> ShaderModule::Create(
    Device* device,
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions) {
    Ref<ShaderModule> shader = AcquireRef(new ShaderModule(device, descriptor, internalExtensions));
    shader->Initialize();
    return shader;
}

ShaderModule::ShaderModule(Device* device,
                           const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                           std::vector<tint::wgsl::Extension> internalExtensions)
    : ShaderModuleBase(device, descriptor, std::move(internalExtensions)) {}

ResultOrError<GLuint> ShaderModule::CompileShader(
    const OpenGLFunctions& gl,
    const ProgrammableStage& programmableStage,
    SingleShaderStage stage,
    bool usesVertexIndex,
    bool usesInstanceIndex,
    bool usesFragDepth,
    VertexAttributeMask bgraSwizzleAttributes,
    std::vector<CombinedSampler>* combinedSamplersOut,
    const PipelineLayout* layout,
    EmulatedTextureBuiltinRegistrar* emulatedTextureBuiltins,
    bool* needsSSBOLengthUniformBuffer) {
    TRACE_EVENT0(GetDevice()->GetPlatform(), General, "TranslateToGLSL");

    const OpenGLVersion& version = ToBackend(GetDevice())->GetGL().GetVersion();

    GLSLCompilationRequest req = {};

    req.shaderModuleHash = GetHash();
    req.inputProgram = UnsafeUnserializedValue(UseTintProgram());

    // Since (non-Vulkan) GLSL does not support descriptor sets, generate a mapping from the
    // original group/binding pair to a binding-only value. This mapping will be used by Tint to
    // remap all global variables to the 1D space.
    const EntryPointMetadata& entryPointMetaData = GetEntryPoint(programmableStage.entryPoint);
    const BindingInfoArray& moduleBindingInfo = entryPointMetaData.bindings;

    tint::Bindings bindings =
        GenerateBindingRemapping(layout, stage, [&](BindGroupIndex group, BindingIndex index) {
            return tint::BindingPoint{
                .group = 0,
                .binding = uint32_t(layout->GetBindingIndexInfo()[group][index]),
            };
        });

    // When textures are accessed without a sampler (e.g., textureLoad()), returned
    // CombinedSamplerInfo should use this sentinel value as sampler binding point.
    req.tintOptions.placeholder_sampler_bind_point = {
        .group = static_cast<uint32_t>(kMaxBindGroupsTyped),
        .binding = 0,
    };

    // Compute the metadata necessary for translating to GL's combined textures and samplers, both
    // for Dawn and for the Tint translation to GLSL.
    {
        std::vector<CombinedSampler> combinedSamplers;
        GenerateCombinedSamplerInfo(entryPointMetaData, bindings, layout, &combinedSamplers,
                                    &(req.tintOptions.sampler_texture_to_name),
                                    &(req.tintOptions.placeholder_sampler_bind_point));
        *combinedSamplersOut = std::move(combinedSamplers);
    }

    // Compute the metadata necessary to emulate some of the texture "getter" builtins not present
    // in GLSL, both for Dawn and for the Tint translation to GLSL.
    GenerateTextureBuiltinFromUniformData(entryPointMetaData, layout, bindings,
                                          emulatedTextureBuiltins,
                                          &(req.tintOptions.texture_builtins_from_uniform));

    req.stage = stage;
    req.entryPointName = programmableStage.entryPoint;
    req.substituteOverrideConfig = BuildSubstituteOverridesTransformConfig(programmableStage);
    req.limits = LimitsForCompilationRequest::Create(GetDevice()->GetLimits().v1);
    req.adapterSupportedLimits = UnsafeUnserializedValue(
        LimitsForCompilationRequest::Create(GetDevice()->GetAdapter()->GetLimits().v1));

    if (GetDevice()->IsToggleEnabled(Toggle::GLUseArrayLengthFromUniform)) {
        *needsSSBOLengthUniformBuffer = GenerateArrayLengthFromuniformData(
            moduleBindingInfo, layout, req.tintOptions.array_length_from_uniform);
        if (*needsSSBOLengthUniformBuffer) {
            req.tintOptions.use_array_length_from_uniform = true;
            req.tintOptions.array_length_from_uniform.ubo_binding = {
                .group = kMaxBindGroups + 2,
                .binding = 0,
            };
            bindings.uniform.emplace(
                req.tintOptions.array_length_from_uniform.ubo_binding,
                tint::BindingPoint{
                    .group = 0,
                    .binding = uint32_t(layout->GetInternalArrayLengthUniformBinding()),
                });
        }
    }

    req.platform = UnsafeUnserializedValue(GetDevice()->GetPlatform());

    req.tintOptions.version = tint::glsl::writer::Version(ToTintGLStandard(version.GetStandard()),
                                                          version.GetMajor(), version.GetMinor());

    req.tintOptions.disable_robustness = !GetDevice()->IsRobustnessEnabled();
    req.tintOptions.disable_workgroup_init =
        GetDevice()->IsToggleEnabled(Toggle::DisableWorkgroupInit);

    if (usesVertexIndex) {
        req.tintOptions.first_vertex_offset = 4 * PipelineLayout::ImmediateLocation::FirstVertex;
    }

    if (usesInstanceIndex) {
        req.tintOptions.first_instance_offset =
            4 * PipelineLayout::ImmediateLocation::FirstInstance;
    }

    if (usesFragDepth) {
        req.tintOptions.depth_range_offsets = {4 * PipelineLayout::ImmediateLocation::MinDepth,
                                               4 * PipelineLayout::ImmediateLocation::MaxDepth};
    }

    if (stage == SingleShaderStage::Vertex) {
        for (VertexAttributeLocation i : bgraSwizzleAttributes) {
            req.tintOptions.bgra_swizzle_locations.insert(static_cast<uint8_t>(i));
        }
    }

    req.tintOptions.strip_all_names = !GetDevice()->IsToggleEnabled(Toggle::DisableSymbolRenaming);

    req.interstageVariables = {};
    for (size_t i = 0; i < entryPointMetaData.interStageVariables.size(); i++) {
        if (entryPointMetaData.usedInterStageVariables[i]) {
            req.interstageVariables.emplace_back(static_cast<uint32_t>(i),
                                                 entryPointMetaData.interStageVariables[i].name);
        }
    }

    req.tintOptions.bindings = std::move(bindings);
    req.tintOptions.disable_polyfill_integer_div_mod =
        GetDevice()->IsToggleEnabled(Toggle::DisablePolyfillsOnIntegerDivisonAndModulo);

    req.tintOptions.enable_integer_range_analysis =
        GetDevice()->IsToggleEnabled(Toggle::EnableIntegerRangeAnalysisInRobustness);

    req.tintOptions.decompose_uniform_buffers =
        GetDevice()->IsToggleEnabled(Toggle::DecomposeUniformBuffers);

    CacheResult<GLSLCompilation> compilationResult;
    DAWN_TRY_LOAD_OR_RUN(
        compilationResult, GetDevice(), std::move(req), GLSLCompilation::FromValidatedBlob,
        [](GLSLCompilationRequest r) -> ResultOrError<GLSLCompilation> {
            // Requires Tint Program here right before actual using.
            auto inputProgram = r.inputProgram.UnsafeGetValue()->GetTintProgram();
            const tint::Program* tintInputProgram = &(inputProgram->program);
            // Convert the AST program to an IR module.
            tint::Result<tint::core::ir::Module> ir;
            {
                SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                   "ShaderModuleProgramToIR");
                ir = tint::wgsl::reader::ProgramToLoweredIR(*tintInputProgram);
                DAWN_INVALID_IF(ir != tint::Success,
                                "An error occurred while generating Tint IR\n%s",
                                ir.Failure().reason);
            }

            {
                SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                   "ShaderModuleSingleEntryPoint");
                auto singleEntryPointResult =
                    tint::core::ir::transform::SingleEntryPoint(ir.Get(), r.entryPointName);
                DAWN_INVALID_IF(singleEntryPointResult != tint::Success,
                                "Pipeline single entry point (IR) failed:\n%s",
                                singleEntryPointResult.Failure().reason);
            }

            // this needs to run after SingleEntryPoint transform which removes unused
            // overrides for the current entry point.

            {
                SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                   "ShaderModuleSubstituteOverrides");
                tint::SubstituteOverridesConfig cfg;
                cfg.map = std::move(r.substituteOverrideConfig);
                auto substituteOverridesResult =
                    tint::core::ir::transform::SubstituteOverrides(ir.Get(), cfg);
                DAWN_INVALID_IF(substituteOverridesResult != tint::Success,
                                "Pipeline override substitution (IR) failed:\n%s",
                                substituteOverridesResult.Failure().reason);
            }

            tint::Result<tint::glsl::writer::Output> result;
            {
                SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(r.platform.UnsafeGetValue(),
                                                   "ShaderModuleGenerateGLSL");
                // Generate GLSL from Tint IR.
                result = tint::glsl::writer::Generate(ir.Get(), r.tintOptions);
                DAWN_INVALID_IF(result != tint::Success,
                                "An error occurred while generating GLSL:\n%s",
                                result.Failure().reason);
            }

            // Workgroup validation has to come after `Generate` because it may require
            // overrides to have been substituted.
            if (r.stage == SingleShaderStage::Compute) {
                // Validate workgroup size after program runs transforms.
                Extent3D _;
                DAWN_TRY_ASSIGN(_,
                                ValidateComputeStageWorkgroupSize(
                                    result->workgroup_info.x, result->workgroup_info.y,
                                    result->workgroup_info.z, result->workgroup_info.storage_size,
                                    /* usesSubgroupMatrix */ false,
                                    /* maxSubgroupSize, GL backend not support */ 0, r.limits,
                                    r.adapterSupportedLimits.UnsafeGetValue()));
            }

            return GLSLCompilation{{std::move(result->glsl)}};
        },
        "OpenGL.CompileShaderToGLSL");

    if (GetDevice()->IsToggleEnabled(Toggle::DumpShaders)) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "/* Dumped generated GLSL */\n" << compilationResult->glsl;

        GetDevice()->EmitLog(wgpu::LoggingType::Info, dumpedMsg.str().c_str());
    }

    GLuint shader = DAWN_GL_TRY(gl, CreateShader(GLShaderType(stage)));
    const char* source = compilationResult->glsl.c_str();
    {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(GetDevice()->GetPlatform(), "GLSL.CompileShader");

        DAWN_GL_TRY(gl, ShaderSource(shader, 1, &source, nullptr));
        DAWN_GL_TRY(gl, CompileShader(shader));
    }

    GLint compileStatus = GL_FALSE;
    DAWN_GL_TRY(gl, GetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus));
    if (compileStatus == GL_FALSE) {
        GLint infoLogLength = 0;
        DAWN_GL_TRY(gl, GetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength));

        if (infoLogLength > 1) {
            std::vector<char> buffer(infoLogLength);
            DAWN_GL_TRY(gl, GetShaderInfoLog(shader, infoLogLength, nullptr, &buffer[0]));
            DAWN_GL_TRY(gl, DeleteShader(shader));
            return DAWN_VALIDATION_ERROR("%s\nProgram compilation failed:\n%s", source,
                                         buffer.data());
        }
    }

    GetDevice()->GetBlobCache()->EnsureStored(compilationResult);

    return shader;
}

}  // namespace dawn::native::opengl
