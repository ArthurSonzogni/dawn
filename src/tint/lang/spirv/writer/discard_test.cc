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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(SpirvWriterTest, Discard) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(core::BuiltinValue::kFrontFacing);
    auto* ep = b.Function("ep", ty.f32(), core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({front_facing});
    ep->SetReturnLocation(0_u);

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {  //
            b.Discard();
            b.ExitIf(ifelse);
        });
        b.Store(buffer, 42_i);
        b.Return(ep, 0.5_f);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
   %ep_inner = OpFunction %float None %16
%front_facing = OpFunctionParameter %bool
         %17 = OpLabel
               OpSelectionMerge %18 None
               OpBranchConditional %front_facing %19 %18
         %19 = OpLabel
               OpStore %continue_execution %false None
               OpBranch %18
         %18 = OpLabel
         %20 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
         %24 = OpLoad %bool %continue_execution None
               OpSelectionMerge %25 None
               OpBranchConditional %24 %26 %25
         %26 = OpLabel
               OpStore %20 %int_42 None
               OpBranch %25
         %25 = OpLabel
         %27 = OpLoad %bool %continue_execution None
         %28 = OpLogicalNot %bool %27
               OpSelectionMerge %29 None
               OpBranchConditional %28 %30 %29
         %30 = OpLabel
               OpKill
         %29 = OpLabel
               OpReturnValue %float_0_5
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, DiscardBeforeAtomic) {
    auto* buffer = b.Var("buffer", ty.ptr(storage, ty.atomic<i32>()));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(core::BuiltinValue::kFrontFacing);
    auto* ep = b.Function("ep", ty.f32(), core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({front_facing});
    ep->SetReturnLocation(0_u);

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {  //
            b.Discard();
            b.ExitIf(ifelse);
        });
        b.Call(ty.i32(), core::BuiltinFn::kAtomicAdd, buffer, 1_i);
        b.Return(ep, 0.5_f);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               ; Function ep_inner
   %ep_inner = OpFunction %float None %16
%front_facing = OpFunctionParameter %bool
         %17 = OpLabel
               OpSelectionMerge %18 None
               OpBranchConditional %front_facing %19 %18
         %19 = OpLabel
               OpStore %continue_execution %false None
               OpBranch %18
         %18 = OpLabel
         %20 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
         %24 = OpLoad %bool %continue_execution None
               OpSelectionMerge %25 None
               OpBranchConditional %24 %26 %27
         %26 = OpLabel
         %29 = OpAtomicIAdd %int %20 %uint_1 %uint_0 %int_1
               OpBranch %25
         %27 = OpLabel
               OpBranch %25
         %25 = OpLabel
         %28 = OpPhi %int %29 %26 %30 %27
         %31 = OpLoad %bool %continue_execution None
         %32 = OpLogicalNot %bool %31
               OpSelectionMerge %33 None
               OpBranchConditional %32 %34 %33
         %34 = OpLabel
               OpKill
         %33 = OpLabel
               OpReturnValue %float_0_5
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Discard_DemoteToHelperWithExtension) {
    Options opts{};
    opts.use_demote_to_helper_invocation_extensions = true;

    auto* v = b.Var("v", ty.ptr<private_, i32>());
    v->SetInitializer(b.Constant(42_i));
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Discard();
        auto* load = b.Load(v);
        auto* add = b.Add(ty.i32(), load, 1_i);

        b.Store(v, add);
        b.Return(func);
    });

    ASSERT_TRUE(Generate(opts)) << Error() << output_;
    EXPECT_INST("OpDemoteToHelperInvocation");
}

TEST_F(SpirvWriterTest, Discard_DemoteToHelperAsTransform) {
    Options opts{};
    opts.use_demote_to_helper_invocation_extensions = false;

    auto* v = b.Var("v", ty.ptr<private_, i32>());
    v->SetInitializer(b.Constant(42_i));
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Discard();
        auto* load = b.Load(v);
        auto* add = b.Add(ty.i32(), load, 1_i);

        b.Store(v, add);
        b.Return(func);
    });

    ASSERT_TRUE(Generate(opts)) << Error() << output_;
    EXPECT_INST("continue_execution");
}

}  // namespace
}  // namespace tint::spirv::writer
