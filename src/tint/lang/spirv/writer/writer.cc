// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/writer.h"

#include <string>
#include <utility>

#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/spirv/writer/common/option_helpers.h"
#include "src/tint/lang/spirv/writer/printer/printer.h"
#include "src/tint/lang/spirv/writer/raise/raise.h"

// Included by 'ast_printer.h', included again here for './tools/run gen' track the dependency.
#include "spirv/unified1/spirv.h"

namespace tint::spirv::writer {

Result<SuccessType> CanGenerate(const core::ir::Module& ir, const Options& options) {
    // The enum is accessible in the API so ensure we have a valid value.
    switch (options.spirv_version) {
        case SpvVersion::kSpv13:
        case SpvVersion::kSpv14:
        case SpvVersion::kSpv15:
            break;
        default:
            return Failure("unsupported SPIR-V version");
    }

    // Check optionally supported types against their required options.
    for (auto* ty : ir.Types()) {
        if (ty->Is<core::type::SubgroupMatrix>()) {
            if (!options.use_vulkan_memory_model) {
                return Failure("using subgroup matrices requires the Vulkan Memory Model");
            }
        }
    }

    // If a remapped entry point name is provided, it must not be empty, and must not contain
    // embedded null characters.
    if (!options.remapped_entry_point_name.empty()) {
        if (options.remapped_entry_point_name.find('\0') != std::string::npos) {
            return Failure("remapped entry point name contains null character");
        }

        // Check for multiple entry points.
        // TODO(375388101): Remove this check when SingleEntryPoint is part of the backend.
        bool has_entry_point = false;
        for (auto& func : ir.functions) {
            if (func->IsEntryPoint()) {
                if (has_entry_point) {
                    return Failure("module must only contain a single entry point");
                }
                has_entry_point = true;
            }
        }
    }

    // Check for unsupported module-scope variable address spaces and types.
    // Also make sure there is at most one user-declared immediate data, and make a note of its
    // size.
    uint32_t user_immediate_data_size = 0;
    for (auto* inst : *ir.root_block) {
        auto* var = inst->As<core::ir::Var>();
        auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
        if (ptr->AddressSpace() == core::AddressSpace::kPixelLocal) {
            return Failure("pixel_local address space is not supported by the SPIR-V backend");
        }

        if (ptr->AddressSpace() == core::AddressSpace::kImmediate) {
            if (user_immediate_data_size > 0) {
                // We've already seen a user-declared immediate data.
                return Failure("module contains multiple user-declared immediate data");
            }
            user_immediate_data_size = tint::RoundUp(4u, ptr->StoreType()->Size());
        }
    }

    static constexpr uint32_t kMaxOffset = 0x1000;
    Hashset<uint32_t, 4> immediate_data_word_offsets;
    auto check_immediate_data_offset = [&](uint32_t offset) {
        // Excessive values can cause OOM / timeouts when padding structures in the printer.
        if (offset > kMaxOffset) {
            return false;
        }
        // Offset must be 4-byte aligned.
        if (offset & 0x3) {
            return false;
        }
        // Offset must not have already been used.
        if (!immediate_data_word_offsets.Add(offset >> 2)) {
            return false;
        }
        // Offset must be after the user-defined immediate data.
        if (offset < user_immediate_data_size) {
            return false;
        }
        return true;
    };

    if (options.depth_range_offsets) {
        if (!check_immediate_data_offset(options.depth_range_offsets->max) ||
            !check_immediate_data_offset(options.depth_range_offsets->min)) {
            return Failure("invalid offsets for depth range immediate data");
        }
    }

    {
        auto res = ValidateBindingOptions(options);
        if (res != Success) {
            return res.Failure();
        }
    }

    return Success;
}

Result<Output> Generate(core::ir::Module& ir, const Options& options) {
    // Raise from core-dialect to SPIR-V-dialect.
    if (auto res = Raise(ir, options); res != Success) {
        return std::move(res.Failure());
    }

    return Print(ir, options);
}

}  // namespace tint::spirv::writer
