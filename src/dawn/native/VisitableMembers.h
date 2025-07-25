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

#ifndef SRC_DAWN_NATIVE_VISITABLE_H_
#define SRC_DAWN_NATIVE_VISITABLE_H_

#include <utility>

#include "dawn/native/stream/BlobSource.h"
#include "dawn/native/stream/ByteVectorSink.h"
#include "dawn/native/stream/Stream.h"

// Helper for X macro to declare a visitable member.
#define DAWN_INTERNAL_VISITABLE_MEMBER_DECL(type, name, ...) type name{__VA_OPT__(__VA_ARGS__)};

// Helper for X macro for visiting a visitable member.
#define DAWN_INTERNAL_VISITABLE_MEMBER_ARG(type, name, ...) , name

namespace dawn::native::detail {
constexpr int kInternalVisitableUnusedForComma = 0;
}  // namespace dawn::native::detail

// Helper X macro to declare members of a class or struct, along with VisitAll
// methods to call a functor on all members.
// Example usage:
//   #define MEMBERS(X)    \
//       X(int, a)         \
//       X(float, b, 42.0) \
//       X(Foo, foo, kFoo) \
//       X(Bar, bar)
//   struct MyStruct {
//    DAWN_VISITABLE_MEMBERS(MEMBERS)
//   };
//   #undef MEMBERS
#define DAWN_VISITABLE_MEMBERS(MEMBERS)                                     \
    MEMBERS(DAWN_INTERNAL_VISITABLE_MEMBER_DECL)                            \
                                                                            \
    template <typename V>                                                   \
    constexpr auto VisitAll(V&& visit) const {                              \
        return [&](int, const auto&... ms) {                                \
            return visit(ms...);                                            \
        }(::dawn::native::detail::kInternalVisitableUnusedForComma MEMBERS( \
                   DAWN_INTERNAL_VISITABLE_MEMBER_ARG));                    \
    }                                                                       \
                                                                            \
    template <typename V>                                                   \
    constexpr auto VisitAll(V&& visit) {                                    \
        return [&](int, auto&... ms) {                                      \
            return visit(ms...);                                            \
        }(::dawn::native::detail::kInternalVisitableUnusedForComma MEMBERS( \
                   DAWN_INTERNAL_VISITABLE_MEMBER_ARG));                    \
    }

#endif  // SRC_DAWN_NATIVE_VISITABLE_H_
