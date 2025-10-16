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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_RAISE_REMOVE_UNIFORM_VECTOR_COMPONENT_LOADS_H_
#define SRC_TINT_LANG_SPIRV_WRITER_RAISE_REMOVE_UNIFORM_VECTOR_COMPONENT_LOADS_H_

#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::spirv::writer::raise {

/// RemoveUniformVectorComponentLoads is a transform that removes load_vector_element instructions
/// for uniform buffers. Instead, we load the whole vector and then extract the desired component.
/// This is to avoid a bug on Qualcomm devices (see crbug.com/452350626).
/// @param module the module to transform
/// @returns success or failure
Result<SuccessType> RemoveUniformVectorComponentLoads(core::ir::Module& module);

}  // namespace tint::spirv::writer::raise

#endif  // SRC_TINT_LANG_SPIRV_WRITER_RAISE_REMOVE_UNIFORM_VECTOR_COMPONENT_LOADS_H_
