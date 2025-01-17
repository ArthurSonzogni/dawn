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

#include "src/tint/lang/core/ir/validator_test.h"

#include <string>

#include "gtest/gtest.h"

#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::core::ir {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(IR_ValidatorTest, Var_RootBlock_NullResult) {
    auto* v = mod.CreateInstruction<ir::Var>(nullptr);
    v->SetInitializer(b.Constant(0_i));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:2:3 error: var: result is undefined
  undef = var, 0i
  ^^^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  undef = var, 0i
}

)");
}

TEST_F(IR_ValidatorTest, Var_VoidType) {
    mod.root_block->Append(b.Var(ty.ptr(private_, ty.void_())));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:2:3 error: var: pointers to void are not permitted
  %1:ptr<private, void, read_write> = var
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<private, void, read_write> = var
}

)");
}

TEST_F(IR_ValidatorTest, Var_Function_NullResult) {
    auto* v = mod.CreateInstruction<ir::Var>(nullptr);
    v->SetInitializer(b.Constant(0_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: var: result is undefined
    undef = var, 0i
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = var, 0i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Function_NoResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>();
        v->SetInitializer(b.Constant(1_i));
        v->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:13 error: var: expected exactly 1 results, got 0
    undef = var, 1i
            ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = var, 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Function_NonPtrResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>();
        v->Result(0)->SetType(ty.f32());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:14 error: var: result type must be a pointer or a reference
    %2:f32 = var
             ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:f32 = var
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Function_UnexpectedInputAttachmentIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>();
        v->SetInputAttachmentIndex(0);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:41 error: var: '@input_attachment_index' is not valid for non-handle var
    %2:ptr<function, f32, read_write> = var @input_attachment_index(0)
                                        ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var @input_attachment_index(0)
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Function_OutsideFunctionScope) {
    auto* v = b.Var<function, f32>();
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:39 error: var: vars in the 'function' address space must be in a function scope
  %1:ptr<function, f32, read_write> = var
                                      ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<function, f32, read_write> = var
}

)");
}

TEST_F(IR_ValidatorTest, Var_NonFunction_InsideFunctionScope) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Var<private_, f32>();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:40 error: var: vars in a function scope must be in the 'function' address space
    %2:ptr<private, f32, read_write> = var
                                       ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<private, f32, read_write> = var
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Private_InsideFunctionScopeWithCapability) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Var<private_, f32>();
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowPrivateVarsInFunctions});
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Var_Private_UnexpectedInputAttachmentIndex) {
    auto* v = b.Var<private_, f32>();
    v->SetInputAttachmentIndex(0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:38 error: var: '@input_attachment_index' is not valid for non-handle var
  %1:ptr<private, f32, read_write> = var @input_attachment_index(0)
                                     ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<private, f32, read_write> = var @input_attachment_index(0)
}

)");
}

TEST_F(IR_ValidatorTest, Var_PushConstant_UnexpectedInputAttachmentIndex) {
    auto* v = b.Var<push_constant, f32>();
    v->SetInputAttachmentIndex(0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:38 error: var: '@input_attachment_index' is not valid for non-handle var
  %1:ptr<push_constant, f32, read> = var @input_attachment_index(0)
                                     ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<push_constant, f32, read> = var @input_attachment_index(0)
}

)");
}

TEST_F(IR_ValidatorTest, Var_Storage_UnexpectedInputAttachmentIndex) {
    auto* v = b.Var<storage, f32>();
    v->SetBindingPoint(0, 0);
    v->SetInputAttachmentIndex(0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:38 error: var: '@input_attachment_index' is not valid for non-handle var
  %1:ptr<storage, f32, read_write> = var @binding_point(0, 0) @input_attachment_index(0)
                                     ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<storage, f32, read_write> = var @binding_point(0, 0) @input_attachment_index(0)
}

)");
}

TEST_F(IR_ValidatorTest, Var_Uniform_UnexpectedInputAttachmentIndex) {
    auto* v = b.Var<uniform, f32>();
    v->SetBindingPoint(0, 0);
    v->SetInputAttachmentIndex(0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:32 error: var: '@input_attachment_index' is not valid for non-handle var
  %1:ptr<uniform, f32, read> = var @binding_point(0, 0) @input_attachment_index(0)
                               ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<uniform, f32, read> = var @binding_point(0, 0) @input_attachment_index(0)
}

)");
}

TEST_F(IR_ValidatorTest, Var_Workgroup_UnexpectedInputAttachmentIndex) {
    auto* v = b.Var<workgroup, f32>();
    v->SetInputAttachmentIndex(0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:40 error: var: '@input_attachment_index' is not valid for non-handle var
  %1:ptr<workgroup, f32, read_write> = var @input_attachment_index(0)
                                       ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<workgroup, f32, read_write> = var @input_attachment_index(0)
}

)");
}

TEST_F(IR_ValidatorTest, Var_Init_WrongType) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>();
        v->SetInitializer(b.Constant(1_i));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:41 error: var: initializer type 'i32' does not match store type 'f32'
    %2:ptr<function, f32, read_write> = var, 1i
                                        ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var, 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Init_NullType) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* i = b.Var<function, f32>("i");
        i->SetInitializer(b.Constant(0_f));
        auto* load = b.Load(i);
        auto* load_ret = load->Result(0);
        auto* j = b.Var<function, f32>("j");
        j->SetInitializer(load_ret);
        load_ret->SetType(nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:5 error: load: result type is undefined
    %3:undef = load %i
    ^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

:5:46 error: var: operand type is undefined
    %j:ptr<function, f32, read_write> = var, %3
                                             ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %i:ptr<function, f32, read_write> = var, 0.0f
    %3:undef = load %i
    %j:ptr<function, f32, read_write> = var, %3
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Init_FunctionTypeInit) {
    auto* invalid = b.Function("invalid_init", ty.void_());
    b.Append(invalid->Block(), [&] { b.Return(invalid); });
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* i = b.Var<function, f32>("i");
        i->SetInitializer(invalid);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:41 error: var: initializer type '<function>' does not match store type 'f32'
    %i:ptr<function, f32, read_write> = var, %invalid_init
                                        ^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%invalid_init = func():void {
  $B1: {
    ret
  }
}
%my_func = func():void {
  $B2: {
    %i:ptr<function, f32, read_write> = var, %invalid_init
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Init_InvalidAddressSpace) {
    auto* p = b.Var<private_, f32>("p");
    p->SetInitializer(b.Constant(1_f));
    mod.root_block->Append(p);
    auto* s = b.Var<storage, f32>("s");
    s->SetInitializer(b.Constant(1_f));
    mod.root_block->Append(s);
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>("v");
        v->SetInitializer(b.Constant(1_f));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:38 error: var: only variables in the function or private address space may be initialized
  %s:ptr<storage, f32, read_write> = var, 1.0f
                                     ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %p:ptr<private, f32, read_write> = var, 1.0f
  %s:ptr<storage, f32, read_write> = var, 1.0f
}

%my_func = func():void {
  $B2: {
    %v:ptr<function, f32, read_write> = var, 1.0f
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_HandleMissingBindingPoint) {
    auto* v = b.Var(ty.ptr<handle, i32>());
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:31 error: var: a resource variable is missing binding point
  %1:ptr<handle, i32, read> = var
                              ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<handle, i32, read> = var
}

)");
}

TEST_F(IR_ValidatorTest, Var_StorageMissingBindingPoint) {
    auto* v = b.Var(ty.ptr<storage, i32>());
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:38 error: var: a resource variable is missing binding point
  %1:ptr<storage, i32, read_write> = var
                                     ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<storage, i32, read_write> = var
}

)");
}

TEST_F(IR_ValidatorTest, Var_UniformMissingBindingPoint) {
    auto* v = b.Var(ty.ptr<uniform, i32>());
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:32 error: var: a resource variable is missing binding point
  %1:ptr<uniform, i32, read> = var
                               ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<uniform, i32, read> = var
}

)");
}

TEST_F(IR_ValidatorTest, Var_NonResourceWithBindingPoint) {
    auto* v = b.Var(ty.ptr<private_, i32>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:38 error: var: a non-resource variable has binding point
  %1:ptr<private, i32, read_write> = var @binding_point(0, 0)
                                     ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<private, i32, read_write> = var @binding_point(0, 0)
}

)");
}

TEST_F(IR_ValidatorTest, Var_MultipleIOAnnotations) {
    auto* v = b.Var<AddressSpace::kIn, vec4<f32>>();
    IOAttributes attr;
    attr.builtin = BuiltinValue::kPosition;
    attr.location = 0;
    v->SetAttributes(attr);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:2:35 error: var: module scope variable has more than one IO annotation, [ @location, built-in ]
  %1:ptr<__in, vec4<f32>, read> = var @location(0) @builtin(position)
                                  ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<__in, vec4<f32>, read> = var @location(0) @builtin(position)
}

)");
}

TEST_F(IR_ValidatorTest, Var_Struct_MultipleIOAnnotations) {
    IOAttributes attr;
    attr.builtin = BuiltinValue::kPosition;
    attr.color = 0;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr},
                                               });
    auto* v = b.Var(ty.ptr(AddressSpace::kOut, str_ty, read_write));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:6:41 error: var: module scope variable struct member has more than one IO annotation, [ built-in, @color ]
  %1:ptr<__out, MyStruct, read_write> = var
                                        ^^^

:5:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:f32 @offset(0), @color(0), @builtin(position)
}

$B1: {  # root
  %1:ptr<__out, MyStruct, read_write> = var
}

)");
}

TEST_F(IR_ValidatorTest, Var_MissingIOAnnotations) {
    auto* v = b.Var<AddressSpace::kIn, vec4<f32>>();
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:2:35 error: var: module scope variable must have at least one IO annotation, e.g. a binding point, a location, etc
  %1:ptr<__in, vec4<f32>, read> = var
                                  ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<__in, vec4<f32>, read> = var
}

)");
}

TEST_F(IR_ValidatorTest, Var_Struct_MissingIOAnnotations) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.f32(), {}},
                                                          });
    auto* v = b.Var(ty.ptr(AddressSpace::kOut, str_ty, read_write));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:6:41 error: var: module scope variable struct members must have at least one IO annotation, e.g. a binding point, a location, etc
  %1:ptr<__out, MyStruct, read_write> = var
                                        ^^^

:5:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:f32 @offset(0)
}

$B1: {  # root
  %1:ptr<__out, MyStruct, read_write> = var
}

)");
}

TEST_F(IR_ValidatorTest, Let_NullResult) {
    auto* v = mod.CreateInstruction<ir::Let>(nullptr, b.Constant(1_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: let: result is undefined
    undef = let 1i
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = let 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_EmptyResults) {
    auto* v = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.i32()), b.Constant(1_i));
    v->ClearResults();

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:13 error: let: expected exactly 1 results, got 0
    undef = let 1i
            ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = let 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_NullValue) {
    auto* v = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.f32()), nullptr);
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:18 error: let: operand is undefined
    %2:f32 = let undef
                 ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:f32 = let undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_EmptyValue) {
    auto* v = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.i32()), b.Constant(1_i));
    v->ClearOperands();

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:14 error: let: expected exactly 1 operands, got 0
    %2:i32 = let
             ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32 = let
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_WrongType) {
    auto* v = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.f32()), b.Constant(1_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:14 error: let: result type 'f32' does not match value type 'i32'
    %2:f32 = let 1i
             ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:f32 = let 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_VoidResult) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* l = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.void_()), b.Constant(1_i));
        b.Append(l);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:15 error: let: result type cannot be void
    %2:void = let 1i
              ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:void = let 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_VoidValue) {
    auto* v = b.Function("void_func", ty.void_());
    b.Append(v->Block(), [&] { b.Return(v); });

    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* l = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.i32()), b.Value(b.Call(v)));
        b.Append(l);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:9:14 error: let: value type cannot be void
    %4:i32 = let %3
             ^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%void_func = func():void {
  $B1: {
    ret
  }
}
%my_func = func():void {
  $B2: {
    %3:void = call %void_func
    %4:i32 = let %3
    ret
  }
}
)");
}

}  // namespace tint::core::ir
