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

#include "src/tint/lang/spirv/type/resource_binding.h"

#include <sstream>

#include "src/tint/lang/core/type/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::spirv::type::ResourceBinding);

namespace tint::spirv::type {

ResourceBinding::ResourceBinding(const core::type::Type* binding_type)
    : Base(static_cast<size_t>(Hash(tint::TypeCode::Of<ResourceBinding>().bits, binding_type)),
           core::type::Flags{}),
      binding_type_(binding_type) {}

bool ResourceBinding::Equals(const UniqueNode& other) const {
    if (auto* o = other.As<ResourceBinding>()) {
        return o->binding_type_ == binding_type_;
    }
    return false;
}

std::string ResourceBinding::FriendlyName() const {
    std::stringstream str;
    str << "spirv.resource_binding<" << binding_type_->FriendlyName() << ">";
    return str.str();
}

core::type::TypeAndCount ResourceBinding::Elements(
    [[maybe_unused]] const core::type::Type* type_if_unused,
    uint32_t count_if_invalid) const {
    return {binding_type_, count_if_invalid};
}

const core::type::Type* ResourceBinding::Element([[maybe_unused]] uint32_t index) const {
    return binding_type_;
}

ResourceBinding* ResourceBinding::Clone(core::type::CloneContext& ctx) const {
    auto* binding_type = binding_type_->Clone(ctx);
    return ctx.dst.mgr->Get<ResourceBinding>(binding_type);
}

}  // namespace tint::spirv::type
