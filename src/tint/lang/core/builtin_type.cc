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
//   src/tint/lang/core/builtin_type.cc.tmpl
//
// To regenerate run: './tools/run gen'
//
//                       Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#include "src/tint/lang/core/builtin_type.h"

namespace tint::core {

/// ParseBuiltinType parses a BuiltinType from a string.
/// @param str the string to parse
/// @returns the parsed enum, or BuiltinType::kUndefined if the string could not be parsed.
BuiltinType ParseBuiltinType(std::string_view str) {
    if (str == "__atomic_compare_exchange_result_i32") {
        return BuiltinType::kAtomicCompareExchangeResultI32;
    }
    if (str == "__atomic_compare_exchange_result_u32") {
        return BuiltinType::kAtomicCompareExchangeResultU32;
    }
    if (str == "__frexp_result_abstract") {
        return BuiltinType::kFrexpResultAbstract;
    }
    if (str == "__frexp_result_f16") {
        return BuiltinType::kFrexpResultF16;
    }
    if (str == "__frexp_result_f32") {
        return BuiltinType::kFrexpResultF32;
    }
    if (str == "__frexp_result_vec2_abstract") {
        return BuiltinType::kFrexpResultVec2Abstract;
    }
    if (str == "__frexp_result_vec2_f16") {
        return BuiltinType::kFrexpResultVec2F16;
    }
    if (str == "__frexp_result_vec2_f32") {
        return BuiltinType::kFrexpResultVec2F32;
    }
    if (str == "__frexp_result_vec3_abstract") {
        return BuiltinType::kFrexpResultVec3Abstract;
    }
    if (str == "__frexp_result_vec3_f16") {
        return BuiltinType::kFrexpResultVec3F16;
    }
    if (str == "__frexp_result_vec3_f32") {
        return BuiltinType::kFrexpResultVec3F32;
    }
    if (str == "__frexp_result_vec4_abstract") {
        return BuiltinType::kFrexpResultVec4Abstract;
    }
    if (str == "__frexp_result_vec4_f16") {
        return BuiltinType::kFrexpResultVec4F16;
    }
    if (str == "__frexp_result_vec4_f32") {
        return BuiltinType::kFrexpResultVec4F32;
    }
    if (str == "__modf_result_abstract") {
        return BuiltinType::kModfResultAbstract;
    }
    if (str == "__modf_result_f16") {
        return BuiltinType::kModfResultF16;
    }
    if (str == "__modf_result_f32") {
        return BuiltinType::kModfResultF32;
    }
    if (str == "__modf_result_vec2_abstract") {
        return BuiltinType::kModfResultVec2Abstract;
    }
    if (str == "__modf_result_vec2_f16") {
        return BuiltinType::kModfResultVec2F16;
    }
    if (str == "__modf_result_vec2_f32") {
        return BuiltinType::kModfResultVec2F32;
    }
    if (str == "__modf_result_vec3_abstract") {
        return BuiltinType::kModfResultVec3Abstract;
    }
    if (str == "__modf_result_vec3_f16") {
        return BuiltinType::kModfResultVec3F16;
    }
    if (str == "__modf_result_vec3_f32") {
        return BuiltinType::kModfResultVec3F32;
    }
    if (str == "__modf_result_vec4_abstract") {
        return BuiltinType::kModfResultVec4Abstract;
    }
    if (str == "__modf_result_vec4_f16") {
        return BuiltinType::kModfResultVec4F16;
    }
    if (str == "__modf_result_vec4_f32") {
        return BuiltinType::kModfResultVec4F32;
    }
    if (str == "array") {
        return BuiltinType::kArray;
    }
    if (str == "atomic") {
        return BuiltinType::kAtomic;
    }
    if (str == "binding_array") {
        return BuiltinType::kBindingArray;
    }
    if (str == "bool") {
        return BuiltinType::kBool;
    }
    if (str == "f16") {
        return BuiltinType::kF16;
    }
    if (str == "f32") {
        return BuiltinType::kF32;
    }
    if (str == "i32") {
        return BuiltinType::kI32;
    }
    if (str == "i8") {
        return BuiltinType::kI8;
    }
    if (str == "input_attachment") {
        return BuiltinType::kInputAttachment;
    }
    if (str == "mat2x2") {
        return BuiltinType::kMat2X2;
    }
    if (str == "mat2x2f") {
        return BuiltinType::kMat2X2F;
    }
    if (str == "mat2x2h") {
        return BuiltinType::kMat2X2H;
    }
    if (str == "mat2x3") {
        return BuiltinType::kMat2X3;
    }
    if (str == "mat2x3f") {
        return BuiltinType::kMat2X3F;
    }
    if (str == "mat2x3h") {
        return BuiltinType::kMat2X3H;
    }
    if (str == "mat2x4") {
        return BuiltinType::kMat2X4;
    }
    if (str == "mat2x4f") {
        return BuiltinType::kMat2X4F;
    }
    if (str == "mat2x4h") {
        return BuiltinType::kMat2X4H;
    }
    if (str == "mat3x2") {
        return BuiltinType::kMat3X2;
    }
    if (str == "mat3x2f") {
        return BuiltinType::kMat3X2F;
    }
    if (str == "mat3x2h") {
        return BuiltinType::kMat3X2H;
    }
    if (str == "mat3x3") {
        return BuiltinType::kMat3X3;
    }
    if (str == "mat3x3f") {
        return BuiltinType::kMat3X3F;
    }
    if (str == "mat3x3h") {
        return BuiltinType::kMat3X3H;
    }
    if (str == "mat3x4") {
        return BuiltinType::kMat3X4;
    }
    if (str == "mat3x4f") {
        return BuiltinType::kMat3X4F;
    }
    if (str == "mat3x4h") {
        return BuiltinType::kMat3X4H;
    }
    if (str == "mat4x2") {
        return BuiltinType::kMat4X2;
    }
    if (str == "mat4x2f") {
        return BuiltinType::kMat4X2F;
    }
    if (str == "mat4x2h") {
        return BuiltinType::kMat4X2H;
    }
    if (str == "mat4x3") {
        return BuiltinType::kMat4X3;
    }
    if (str == "mat4x3f") {
        return BuiltinType::kMat4X3F;
    }
    if (str == "mat4x3h") {
        return BuiltinType::kMat4X3H;
    }
    if (str == "mat4x4") {
        return BuiltinType::kMat4X4;
    }
    if (str == "mat4x4f") {
        return BuiltinType::kMat4X4F;
    }
    if (str == "mat4x4h") {
        return BuiltinType::kMat4X4H;
    }
    if (str == "ptr") {
        return BuiltinType::kPtr;
    }
    if (str == "sampler") {
        return BuiltinType::kSampler;
    }
    if (str == "sampler_comparison") {
        return BuiltinType::kSamplerComparison;
    }
    if (str == "subgroup_matrix_left") {
        return BuiltinType::kSubgroupMatrixLeft;
    }
    if (str == "subgroup_matrix_result") {
        return BuiltinType::kSubgroupMatrixResult;
    }
    if (str == "subgroup_matrix_right") {
        return BuiltinType::kSubgroupMatrixRight;
    }
    if (str == "texture_1d") {
        return BuiltinType::kTexture1D;
    }
    if (str == "texture_2d") {
        return BuiltinType::kTexture2D;
    }
    if (str == "texture_2d_array") {
        return BuiltinType::kTexture2DArray;
    }
    if (str == "texture_3d") {
        return BuiltinType::kTexture3D;
    }
    if (str == "texture_cube") {
        return BuiltinType::kTextureCube;
    }
    if (str == "texture_cube_array") {
        return BuiltinType::kTextureCubeArray;
    }
    if (str == "texture_depth_2d") {
        return BuiltinType::kTextureDepth2D;
    }
    if (str == "texture_depth_2d_array") {
        return BuiltinType::kTextureDepth2DArray;
    }
    if (str == "texture_depth_cube") {
        return BuiltinType::kTextureDepthCube;
    }
    if (str == "texture_depth_cube_array") {
        return BuiltinType::kTextureDepthCubeArray;
    }
    if (str == "texture_depth_multisampled_2d") {
        return BuiltinType::kTextureDepthMultisampled2D;
    }
    if (str == "texture_external") {
        return BuiltinType::kTextureExternal;
    }
    if (str == "texture_multisampled_2d") {
        return BuiltinType::kTextureMultisampled2D;
    }
    if (str == "texture_storage_1d") {
        return BuiltinType::kTextureStorage1D;
    }
    if (str == "texture_storage_2d") {
        return BuiltinType::kTextureStorage2D;
    }
    if (str == "texture_storage_2d_array") {
        return BuiltinType::kTextureStorage2DArray;
    }
    if (str == "texture_storage_3d") {
        return BuiltinType::kTextureStorage3D;
    }
    if (str == "u32") {
        return BuiltinType::kU32;
    }
    if (str == "u8") {
        return BuiltinType::kU8;
    }
    if (str == "vec2") {
        return BuiltinType::kVec2;
    }
    if (str == "vec2f") {
        return BuiltinType::kVec2F;
    }
    if (str == "vec2h") {
        return BuiltinType::kVec2H;
    }
    if (str == "vec2i") {
        return BuiltinType::kVec2I;
    }
    if (str == "vec2u") {
        return BuiltinType::kVec2U;
    }
    if (str == "vec3") {
        return BuiltinType::kVec3;
    }
    if (str == "vec3f") {
        return BuiltinType::kVec3F;
    }
    if (str == "vec3h") {
        return BuiltinType::kVec3H;
    }
    if (str == "vec3i") {
        return BuiltinType::kVec3I;
    }
    if (str == "vec3u") {
        return BuiltinType::kVec3U;
    }
    if (str == "vec4") {
        return BuiltinType::kVec4;
    }
    if (str == "vec4f") {
        return BuiltinType::kVec4F;
    }
    if (str == "vec4h") {
        return BuiltinType::kVec4H;
    }
    if (str == "vec4i") {
        return BuiltinType::kVec4I;
    }
    if (str == "vec4u") {
        return BuiltinType::kVec4U;
    }
    return BuiltinType::kUndefined;
}

std::string_view ToString(BuiltinType value) {
    switch (value) {
        case BuiltinType::kUndefined:
            return "undefined";
        case BuiltinType::kAtomicCompareExchangeResultI32:
            return "__atomic_compare_exchange_result_i32";
        case BuiltinType::kAtomicCompareExchangeResultU32:
            return "__atomic_compare_exchange_result_u32";
        case BuiltinType::kFrexpResultAbstract:
            return "__frexp_result_abstract";
        case BuiltinType::kFrexpResultF16:
            return "__frexp_result_f16";
        case BuiltinType::kFrexpResultF32:
            return "__frexp_result_f32";
        case BuiltinType::kFrexpResultVec2Abstract:
            return "__frexp_result_vec2_abstract";
        case BuiltinType::kFrexpResultVec2F16:
            return "__frexp_result_vec2_f16";
        case BuiltinType::kFrexpResultVec2F32:
            return "__frexp_result_vec2_f32";
        case BuiltinType::kFrexpResultVec3Abstract:
            return "__frexp_result_vec3_abstract";
        case BuiltinType::kFrexpResultVec3F16:
            return "__frexp_result_vec3_f16";
        case BuiltinType::kFrexpResultVec3F32:
            return "__frexp_result_vec3_f32";
        case BuiltinType::kFrexpResultVec4Abstract:
            return "__frexp_result_vec4_abstract";
        case BuiltinType::kFrexpResultVec4F16:
            return "__frexp_result_vec4_f16";
        case BuiltinType::kFrexpResultVec4F32:
            return "__frexp_result_vec4_f32";
        case BuiltinType::kModfResultAbstract:
            return "__modf_result_abstract";
        case BuiltinType::kModfResultF16:
            return "__modf_result_f16";
        case BuiltinType::kModfResultF32:
            return "__modf_result_f32";
        case BuiltinType::kModfResultVec2Abstract:
            return "__modf_result_vec2_abstract";
        case BuiltinType::kModfResultVec2F16:
            return "__modf_result_vec2_f16";
        case BuiltinType::kModfResultVec2F32:
            return "__modf_result_vec2_f32";
        case BuiltinType::kModfResultVec3Abstract:
            return "__modf_result_vec3_abstract";
        case BuiltinType::kModfResultVec3F16:
            return "__modf_result_vec3_f16";
        case BuiltinType::kModfResultVec3F32:
            return "__modf_result_vec3_f32";
        case BuiltinType::kModfResultVec4Abstract:
            return "__modf_result_vec4_abstract";
        case BuiltinType::kModfResultVec4F16:
            return "__modf_result_vec4_f16";
        case BuiltinType::kModfResultVec4F32:
            return "__modf_result_vec4_f32";
        case BuiltinType::kArray:
            return "array";
        case BuiltinType::kAtomic:
            return "atomic";
        case BuiltinType::kBindingArray:
            return "binding_array";
        case BuiltinType::kBool:
            return "bool";
        case BuiltinType::kF16:
            return "f16";
        case BuiltinType::kF32:
            return "f32";
        case BuiltinType::kI32:
            return "i32";
        case BuiltinType::kI8:
            return "i8";
        case BuiltinType::kInputAttachment:
            return "input_attachment";
        case BuiltinType::kMat2X2:
            return "mat2x2";
        case BuiltinType::kMat2X2F:
            return "mat2x2f";
        case BuiltinType::kMat2X2H:
            return "mat2x2h";
        case BuiltinType::kMat2X3:
            return "mat2x3";
        case BuiltinType::kMat2X3F:
            return "mat2x3f";
        case BuiltinType::kMat2X3H:
            return "mat2x3h";
        case BuiltinType::kMat2X4:
            return "mat2x4";
        case BuiltinType::kMat2X4F:
            return "mat2x4f";
        case BuiltinType::kMat2X4H:
            return "mat2x4h";
        case BuiltinType::kMat3X2:
            return "mat3x2";
        case BuiltinType::kMat3X2F:
            return "mat3x2f";
        case BuiltinType::kMat3X2H:
            return "mat3x2h";
        case BuiltinType::kMat3X3:
            return "mat3x3";
        case BuiltinType::kMat3X3F:
            return "mat3x3f";
        case BuiltinType::kMat3X3H:
            return "mat3x3h";
        case BuiltinType::kMat3X4:
            return "mat3x4";
        case BuiltinType::kMat3X4F:
            return "mat3x4f";
        case BuiltinType::kMat3X4H:
            return "mat3x4h";
        case BuiltinType::kMat4X2:
            return "mat4x2";
        case BuiltinType::kMat4X2F:
            return "mat4x2f";
        case BuiltinType::kMat4X2H:
            return "mat4x2h";
        case BuiltinType::kMat4X3:
            return "mat4x3";
        case BuiltinType::kMat4X3F:
            return "mat4x3f";
        case BuiltinType::kMat4X3H:
            return "mat4x3h";
        case BuiltinType::kMat4X4:
            return "mat4x4";
        case BuiltinType::kMat4X4F:
            return "mat4x4f";
        case BuiltinType::kMat4X4H:
            return "mat4x4h";
        case BuiltinType::kPtr:
            return "ptr";
        case BuiltinType::kSampler:
            return "sampler";
        case BuiltinType::kSamplerComparison:
            return "sampler_comparison";
        case BuiltinType::kSubgroupMatrixLeft:
            return "subgroup_matrix_left";
        case BuiltinType::kSubgroupMatrixResult:
            return "subgroup_matrix_result";
        case BuiltinType::kSubgroupMatrixRight:
            return "subgroup_matrix_right";
        case BuiltinType::kTexture1D:
            return "texture_1d";
        case BuiltinType::kTexture2D:
            return "texture_2d";
        case BuiltinType::kTexture2DArray:
            return "texture_2d_array";
        case BuiltinType::kTexture3D:
            return "texture_3d";
        case BuiltinType::kTextureCube:
            return "texture_cube";
        case BuiltinType::kTextureCubeArray:
            return "texture_cube_array";
        case BuiltinType::kTextureDepth2D:
            return "texture_depth_2d";
        case BuiltinType::kTextureDepth2DArray:
            return "texture_depth_2d_array";
        case BuiltinType::kTextureDepthCube:
            return "texture_depth_cube";
        case BuiltinType::kTextureDepthCubeArray:
            return "texture_depth_cube_array";
        case BuiltinType::kTextureDepthMultisampled2D:
            return "texture_depth_multisampled_2d";
        case BuiltinType::kTextureExternal:
            return "texture_external";
        case BuiltinType::kTextureMultisampled2D:
            return "texture_multisampled_2d";
        case BuiltinType::kTextureStorage1D:
            return "texture_storage_1d";
        case BuiltinType::kTextureStorage2D:
            return "texture_storage_2d";
        case BuiltinType::kTextureStorage2DArray:
            return "texture_storage_2d_array";
        case BuiltinType::kTextureStorage3D:
            return "texture_storage_3d";
        case BuiltinType::kU32:
            return "u32";
        case BuiltinType::kU8:
            return "u8";
        case BuiltinType::kVec2:
            return "vec2";
        case BuiltinType::kVec2F:
            return "vec2f";
        case BuiltinType::kVec2H:
            return "vec2h";
        case BuiltinType::kVec2I:
            return "vec2i";
        case BuiltinType::kVec2U:
            return "vec2u";
        case BuiltinType::kVec3:
            return "vec3";
        case BuiltinType::kVec3F:
            return "vec3f";
        case BuiltinType::kVec3H:
            return "vec3h";
        case BuiltinType::kVec3I:
            return "vec3i";
        case BuiltinType::kVec3U:
            return "vec3u";
        case BuiltinType::kVec4:
            return "vec4";
        case BuiltinType::kVec4F:
            return "vec4f";
        case BuiltinType::kVec4H:
            return "vec4h";
        case BuiltinType::kVec4I:
            return "vec4i";
        case BuiltinType::kVec4U:
            return "vec4u";
    }
    return "<unknown>";
}

}  // namespace tint::core
