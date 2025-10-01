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

#include "src/tint/lang/spirv/writer/raise/resource_binding.h"

#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/resource_type.h"
#include "src/tint/lang/spirv/type/resource_binding.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer::raise {

// Returns a map of types to the var which is used to access the memory of that type
Hashmap<const core::type::Type*, core::ir::Var*, 4> ResourceBindingHelper::GenerateAliases(
    core::ir::Builder& b,
    core::ir::Var* var,
    const std::vector<ResourceType>& types) const {
    Hashmap<const core::type::Type*, core::ir::Var*, 4> res;

    auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
    TINT_IR_ASSERT(b.ir, ptr);

    for (auto& type : types) {
        auto* t = core::type::ResourceTypeToType(b.ir.Types(), type);
        b.InsertBefore(var, [&] {
            auto* spv_ty = b.ir.Types().Get<spirv::type::ResourceBinding>(t);
            auto* v = b.Var(b.ir.Types().ptr(handle, spv_ty, ptr->Access()));
            v->SetBindingPoint(var->BindingPoint()->group, var->BindingPoint()->binding);
            res.Add(t, v);
        });
    }

    return res;
}

}  // namespace tint::spirv::writer::raise
