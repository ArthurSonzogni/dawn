// Copyright 2022 The Dawn & Tint Authors
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

////////////////////////////////////////////////////////////////////////////////
// File generated by 'tools/src/cmd/gen' using the template:
//   src/tint/lang/core/builtin_type.h.tmpl
//
// To regenerate run: './tools/run gen'
//
//                       Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#ifndef SRC_TINT_LANG_CORE_BUILTIN_TYPE_H_
#define SRC_TINT_LANG_CORE_BUILTIN_TYPE_H_

#include <cstdint>

#include "src/tint/utils/rtti/traits.h"

namespace tint::core {

/// An enumerator of builtin types.
enum class BuiltinType : uint8_t {
    kUndefined,
    kAtomicCompareExchangeResultI32,
    kAtomicCompareExchangeResultU32,
    kFrexpResultAbstract,
    kFrexpResultF16,
    kFrexpResultF32,
    kFrexpResultVec2Abstract,
    kFrexpResultVec2F16,
    kFrexpResultVec2F32,
    kFrexpResultVec3Abstract,
    kFrexpResultVec3F16,
    kFrexpResultVec3F32,
    kFrexpResultVec4Abstract,
    kFrexpResultVec4F16,
    kFrexpResultVec4F32,
    kModfResultAbstract,
    kModfResultF16,
    kModfResultF32,
    kModfResultVec2Abstract,
    kModfResultVec2F16,
    kModfResultVec2F32,
    kModfResultVec3Abstract,
    kModfResultVec3F16,
    kModfResultVec3F32,
    kModfResultVec4Abstract,
    kModfResultVec4F16,
    kModfResultVec4F32,
    kArray,
    kAtomic,
    kBindingArray,
    kBool,
    kF16,
    kF32,
    kI32,
    kI8,
    kInputAttachment,
    kMat2X2,
    kMat2X2F,
    kMat2X2H,
    kMat2X3,
    kMat2X3F,
    kMat2X3H,
    kMat2X4,
    kMat2X4F,
    kMat2X4H,
    kMat3X2,
    kMat3X2F,
    kMat3X2H,
    kMat3X3,
    kMat3X3F,
    kMat3X3H,
    kMat3X4,
    kMat3X4F,
    kMat3X4H,
    kMat4X2,
    kMat4X2F,
    kMat4X2H,
    kMat4X3,
    kMat4X3F,
    kMat4X3H,
    kMat4X4,
    kMat4X4F,
    kMat4X4H,
    kPtr,
    kSampler,
    kSamplerComparison,
    kSubgroupMatrixLeft,
    kSubgroupMatrixResult,
    kSubgroupMatrixRight,
    kTexture1D,
    kTexture2D,
    kTexture2DArray,
    kTexture3D,
    kTextureCube,
    kTextureCubeArray,
    kTextureDepth2D,
    kTextureDepth2DArray,
    kTextureDepthCube,
    kTextureDepthCubeArray,
    kTextureDepthMultisampled2D,
    kTextureExternal,
    kTextureMultisampled2D,
    kTextureStorage1D,
    kTextureStorage2D,
    kTextureStorage2DArray,
    kTextureStorage3D,
    kU32,
    kU8,
    kVec2,
    kVec2F,
    kVec2H,
    kVec2I,
    kVec2U,
    kVec3,
    kVec3F,
    kVec3H,
    kVec3I,
    kVec3U,
    kVec4,
    kVec4F,
    kVec4H,
    kVec4I,
    kVec4U,
};

/// @param value the enum value
/// @returns the string for the given enum value
std::string_view ToString(BuiltinType value);

/// @param out the stream to write to
/// @param value the BuiltinType
/// @returns @p out so calls can be chained
template <typename STREAM>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& out, BuiltinType value) {
    return out << ToString(value);
}

/// ParseBuiltinType parses a BuiltinType from a string.
/// @param str the string to parse
/// @returns the parsed enum, or BuiltinType::kUndefined if the string could not be parsed.
BuiltinType ParseBuiltinType(std::string_view str);

constexpr std::string_view kBuiltinTypeStrings[] = {
    "__atomic_compare_exchange_result_i32",
    "__atomic_compare_exchange_result_u32",
    "__frexp_result_abstract",
    "__frexp_result_f16",
    "__frexp_result_f32",
    "__frexp_result_vec2_abstract",
    "__frexp_result_vec2_f16",
    "__frexp_result_vec2_f32",
    "__frexp_result_vec3_abstract",
    "__frexp_result_vec3_f16",
    "__frexp_result_vec3_f32",
    "__frexp_result_vec4_abstract",
    "__frexp_result_vec4_f16",
    "__frexp_result_vec4_f32",
    "__modf_result_abstract",
    "__modf_result_f16",
    "__modf_result_f32",
    "__modf_result_vec2_abstract",
    "__modf_result_vec2_f16",
    "__modf_result_vec2_f32",
    "__modf_result_vec3_abstract",
    "__modf_result_vec3_f16",
    "__modf_result_vec3_f32",
    "__modf_result_vec4_abstract",
    "__modf_result_vec4_f16",
    "__modf_result_vec4_f32",
    "array",
    "atomic",
    "binding_array",
    "bool",
    "f16",
    "f32",
    "i32",
    "i8",
    "input_attachment",
    "mat2x2",
    "mat2x2f",
    "mat2x2h",
    "mat2x3",
    "mat2x3f",
    "mat2x3h",
    "mat2x4",
    "mat2x4f",
    "mat2x4h",
    "mat3x2",
    "mat3x2f",
    "mat3x2h",
    "mat3x3",
    "mat3x3f",
    "mat3x3h",
    "mat3x4",
    "mat3x4f",
    "mat3x4h",
    "mat4x2",
    "mat4x2f",
    "mat4x2h",
    "mat4x3",
    "mat4x3f",
    "mat4x3h",
    "mat4x4",
    "mat4x4f",
    "mat4x4h",
    "ptr",
    "sampler",
    "sampler_comparison",
    "subgroup_matrix_left",
    "subgroup_matrix_result",
    "subgroup_matrix_right",
    "texture_1d",
    "texture_2d",
    "texture_2d_array",
    "texture_3d",
    "texture_cube",
    "texture_cube_array",
    "texture_depth_2d",
    "texture_depth_2d_array",
    "texture_depth_cube",
    "texture_depth_cube_array",
    "texture_depth_multisampled_2d",
    "texture_external",
    "texture_multisampled_2d",
    "texture_storage_1d",
    "texture_storage_2d",
    "texture_storage_2d_array",
    "texture_storage_3d",
    "u32",
    "u8",
    "vec2",
    "vec2f",
    "vec2h",
    "vec2i",
    "vec2u",
    "vec3",
    "vec3f",
    "vec3h",
    "vec3i",
    "vec3u",
    "vec4",
    "vec4f",
    "vec4h",
    "vec4i",
    "vec4u",
};

}  // namespace tint::core

#endif  // SRC_TINT_LANG_CORE_BUILTIN_TYPE_H_
