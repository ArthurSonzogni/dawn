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

#include "src/tint/lang/spirv/reader/parser/helper_test.h"

namespace tint::spirv::reader {
namespace {

TEST_F(SpirvParserTest, FunctionVar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %u32_ptr = OpTypePointer Function %u32
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %u32_ptr Function
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, u32, read_write> = var undef
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FunctionVar_Initializer) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
     %u32_42 = OpConstant %u32 42
    %u32_ptr = OpTypePointer Function %u32
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %u32_ptr Function %u32_42
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, u32, read_write> = var 42u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, PrivateVar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %u32_ptr = OpTypePointer Private %u32
    %ep_type = OpTypeFunction %void
        %var = OpVariable %u32_ptr Private
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<private, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, PrivateVar_Initializer) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
     %u32_42 = OpConstant %u32 42
    %u32_ptr = OpTypePointer Private %u32
    %ep_type = OpTypeFunction %void
        %var = OpVariable %u32_ptr Private %u32_42
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<private, u32, read_write> = var 42u
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, StorageVar_ReadOnly) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %str Block
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %6 NonWritable
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %str = OpTypeStruct %uint
%_ptr_StorageBuffer_str = OpTypePointer StorageBuffer %str
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_StorageBuffer_str StorageBuffer
          %1 = OpFunction %void None %5
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, StorageVar_ReadWrite) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %str Block
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %str = OpTypeStruct %uint
%_ptr_StorageBuffer_str = OpTypePointer StorageBuffer %str
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_StorageBuffer_str StorageBuffer
          %1 = OpFunction %void None %5
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read_write> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, StorageVar_ReadOnly_And_ReadWrite) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %str Block
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %6 NonWritable
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
               OpDecorate %7 DescriptorSet 1
               OpDecorate %7 Binding 3
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %str = OpTypeStruct %uint
%_ptr_StorageBuffer_str = OpTypePointer StorageBuffer %str
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_StorageBuffer_str StorageBuffer
          %7 = OpVariable %_ptr_StorageBuffer_str StorageBuffer
          %1 = OpFunction %void None %5
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read> = var undef @binding_point(1, 2)
  %2:ptr<storage, tint_symbol_1, read_write> = var undef @binding_point(1, 3)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, UniformVar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %str Block
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %6 NonWritable
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %str = OpTypeStruct %uint
%_ptr_Uniform_str = OpTypePointer Uniform %str
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_Uniform_str Uniform
          %1 = OpFunction %void None %5
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, tint_symbol_1, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, UniformConstantVar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
       %samp = OpTypeSampler
%_ptr_Uniform_samp = OpTypePointer UniformConstant %samp
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_Uniform_samp UniformConstant
          %1 = OpFunction %void None %5
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, sampler, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

struct BuiltinCase {
    std::string spirv_type;
    std::string spirv_builtin;
    std::string ir;
};
std::string PrintBuiltinCase(testing::TestParamInfo<BuiltinCase> bc) {
    return bc.param.spirv_builtin;
}

using BuiltinInputTest = SpirvParserTestWithParam<BuiltinCase>;

TEST_P(BuiltinInputTest, Enum) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var BuiltIn )" +
                  params.spirv_builtin + R"(
       %void = OpTypeVoid
       %bool = OpTypeBool
        %u32 = OpTypeInt 32 0
      %vec3u = OpTypeVector %u32 3
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
      %u32_1 = OpConstant %u32 1
  %arr_u32_1 = OpTypeArray %u32 %u32_1
    %fn_type = OpTypeFunction %void

 %_ptr_Input = OpTypePointer Input %)" +
                  params.spirv_type + R"(
        %var = OpVariable %_ptr_Input Input

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              params.ir);
}

INSTANTIATE_TEST_SUITE_P(
    SpirvParser,
    BuiltinInputTest,
    testing::Values(
        BuiltinCase{
            "vec4f",
            "FragCoord",
            "%1:ptr<__in, vec4<f32>, read> = var undef @builtin(position)",
        },
        BuiltinCase{
            "bool",
            "FrontFacing",
            "%1:ptr<__in, bool, read> = var undef @builtin(front_facing)",
        },
        BuiltinCase{
            "vec3u",
            "GlobalInvocationId",
            "%1:ptr<__in, vec3<u32>, read> = var undef @builtin(global_invocation_id)",
        },
        BuiltinCase{
            "u32",
            "InstanceIndex",
            "%1:ptr<__in, u32, read> = var undef @builtin(instance_index)",
        },
        BuiltinCase{
            "vec3u",
            "LocalInvocationId",
            "%1:ptr<__in, vec3<u32>, read> = var undef @builtin(local_invocation_id)",
        },
        BuiltinCase{
            "u32",
            "LocalInvocationIndex",
            "%1:ptr<__in, u32, read> = var undef @builtin(local_invocation_index)",
        },
        BuiltinCase{
            "vec3u",
            "NumWorkgroups",
            "%1:ptr<__in, vec3<u32>, read> = var undef @builtin(num_workgroups)",
        },
        BuiltinCase{
            "u32",
            "SampleId",
            "%1:ptr<__in, u32, read> = var undef @builtin(sample_index)",
        },
        BuiltinCase{
            "arr_u32_1",
            "SampleMask",
            "%1:ptr<__in, array<u32, 1>, read> = var undef @builtin(sample_mask)",
        },
        BuiltinCase{
            "u32",
            "VertexIndex",
            "%1:ptr<__in, u32, read> = var undef @builtin(vertex_index)",
        },
        BuiltinCase{
            "vec3u",
            "WorkgroupId",
            "%1:ptr<__in, vec3<u32>, read> = var undef @builtin(workgroup_id)",
        }),
    PrintBuiltinCase);

using BuiltinOutputTest = SpirvParserTestWithParam<BuiltinCase>;

TEST_P(BuiltinOutputTest, Enum) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var BuiltIn )" +
                  params.spirv_builtin + R"(
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
      %u32_1 = OpConstant %u32 1
  %arr_u32_1 = OpTypeArray %u32 %u32_1
    %fn_type = OpTypeFunction %void

%_ptr_Output = OpTypePointer Output %)" +
                  params.spirv_type + R"(
        %var = OpVariable %_ptr_Output Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              params.ir);
}

INSTANTIATE_TEST_SUITE_P(
    SpirvParser,
    BuiltinOutputTest,
    testing::Values(
        BuiltinCase{
            "f32",
            "PointSize",
            "%1:ptr<__out, f32, read_write> = var undef @builtin(__point_size)",
        },
        BuiltinCase{
            "vec4f",
            "Position",
            "%1:ptr<__out, vec4<f32>, read_write> = var undef @builtin(position)",
        },
        BuiltinCase{
            "arr_u32_1",
            "SampleMask",
            "%1:ptr<__out, array<u32, 1>, read_write> = var undef @builtin(sample_mask)",
        }),
    PrintBuiltinCase);

TEST_F(SpirvParserTest, Invariant_OnVariable) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var BuiltIn Position
               OpDecorate %var Invariant
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %fn_type = OpTypeFunction %void

%_ptr_Output = OpTypePointer Output %vec4f
        %var = OpVariable %_ptr_Output Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              "%1:ptr<__out, vec4<f32>, read_write> = var undef @invariant @builtin(position)");
}

struct LocationCase {
    std::string spirv_decorations;
    std::string ir;
};

using LocationVarTest = SpirvParserTestWithParam<LocationCase>;

TEST_P(LocationVarTest, Input) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          )" + params.spirv_decorations +
                  R"(
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %fn_type = OpTypeFunction %void

%_ptr_Input = OpTypePointer Input %vec4f
        %var = OpVariable %_ptr_Input Input

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              "%1:ptr<__in, vec4<f32>, read> = " + params.ir);
}

TEST_P(LocationVarTest, Output) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          )" + params.spirv_decorations +
                  R"(
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %fn_type = OpTypeFunction %void

%_ptr_Output = OpTypePointer Output %vec4f
        %var = OpVariable %_ptr_Output Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              "%1:ptr<__out, vec4<f32>, read_write> = " + params.ir);
}

INSTANTIATE_TEST_SUITE_P(SpirvParser,
                         LocationVarTest,
                         testing::Values(
                             LocationCase{
                                 "OpDecorate %var Location 1 ",
                                 "var undef @location(1)",
                             },
                             LocationCase{
                                 "OpDecorate %var Location 2 "
                                 "OpDecorate %var NoPerspective ",
                                 "var undef @location(2) @interpolate(linear, center)",
                             },
                             LocationCase{
                                 "OpDecorate %var Location 3 "
                                 "OpDecorate %var Flat ",
                                 "var undef @location(3) @interpolate(flat, center)",
                             },
                             LocationCase{
                                 "OpDecorate %var Location 4 "
                                 "OpDecorate %var Centroid ",
                                 "var undef @location(4) @interpolate(perspective, centroid)",
                             },
                             LocationCase{
                                 "OpDecorate %var Location 5 "
                                 "OpDecorate %var Sample ",
                                 "var undef @location(5) @interpolate(perspective, sample)",
                             },
                             LocationCase{
                                 "OpDecorate %var Location 6 "
                                 "OpDecorate %var NoPerspective "
                                 "OpDecorate %var Centroid ",
                                 "var undef @location(6) @interpolate(linear, centroid)",
                             }));

TEST_F(SpirvParserTest, Var_OpSpecConstantTrue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
               OpDecorate %c SpecId 12
       %void = OpTypeVoid
       %bool = OpTypeBool
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
          %c = OpSpecConstantTrue %bool
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpLogicalAnd %bool %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %myconst:bool = override true @id(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:bool = and %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstant_f16) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
               OpDecorate %c SpecId 12
       %void = OpTypeVoid
        %f16 = OpTypeFloat 16
          %c = OpSpecConstant %f16 2
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpFAdd %f16 %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %myconst:f16 = override 2.0h @id(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:f16 = add %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstant_f32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
               OpDecorate %c SpecId 12
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
          %c = OpSpecConstant %f32 2
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpFAdd %f32 %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %myconst:f32 = override 2.0f @id(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:f32 = add %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstant_u32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
               OpDecorate %c SpecId 12
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
          %c = OpSpecConstant %u32 2
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpIAdd %u32 %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %myconst:u32 = override 2u @id(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = spirv.add<u32> %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstant_i32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
               OpDecorate %c SpecId 12
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
          %c = OpSpecConstant %i32 2
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpIAdd %i32 %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %myconst:i32 = override 2i @id(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:i32 = spirv.add<i32> %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantTrue_NoSpecId) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
          %c = OpSpecConstantTrue %bool
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpLogicalAnd %bool %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = and true, true
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantFalse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
               OpDecorate %c SpecId 12
       %void = OpTypeVoid
       %bool = OpTypeBool
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
          %c = OpSpecConstantFalse %bool
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpLogicalAnd %bool %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %myconst:bool = override false @id(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:bool = and %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantFalse_NoSpecId) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
          %c = OpSpecConstantFalse %bool
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpLogicalAnd %bool %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = and false, false
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_LogicalAnd) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %cond "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
       %cond = OpSpecConstantOp %bool LogicalAnd %false %true
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpLogicalAnd %bool %cond %cond
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = and false, true
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = and %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_LogicalOr) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %cond "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
       %cond = OpSpecConstantOp %bool LogicalOr %false %true
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpLogicalOr %bool %cond %cond
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = or false, true
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = or %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_LogicalNot) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %cond "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
       %cond = OpSpecConstantOp %bool LogicalNot %false
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpLogicalNot %bool %cond
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = not false
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = not %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_LogicalEqual) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %cond "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
       %cond = OpSpecConstantOp %bool LogicalEqual %true %false
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpLogicalEqual %bool %cond %cond
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = eq true, false
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = eq %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_LogicalNotEqual) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %cond "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
       %cond = OpSpecConstantOp %bool LogicalNotEqual %true %false
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpLogicalNotEqual %bool %cond %cond
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = neq true, false
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = neq %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_Not) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %cond "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %i32 = OpTypeInt 32 1
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
        %one = OpConstant %i32 1
       %cond = OpSpecConstantOp %i32 Not %one
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpNot %i32 %cond
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.not<i32> 1i
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = spirv.not<i32> %myconst
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
