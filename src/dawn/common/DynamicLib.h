// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_DYNAMICLIB_H_
#define SRC_DAWN_COMMON_DYNAMICLIB_H_

#include <string>
#include <type_traits>

#include "dawn/common/Assert.h"
#include "dawn/common/Platform.h"

namespace dawn {

class DynamicLib {
  public:
    DynamicLib() = default;
    ~DynamicLib();

    DynamicLib(const DynamicLib&) = delete;
    DynamicLib& operator=(const DynamicLib&) = delete;

    DynamicLib(DynamicLib&& other);
    DynamicLib& operator=(DynamicLib&& other);

    bool Valid() const;

#if DAWN_PLATFORM_IS(WINDOWS) && !DAWN_PLATFORM_IS(WINUWP)
    bool OpenSystemLibrary(std::wstring_view filename, std::string* error = nullptr);
#endif
    bool Open(const std::string& filename, std::string* error = nullptr);
    void Close();

    void* GetProc(const std::string& procName, std::string* error = nullptr) const;

    template <typename T>
        requires std::is_function_v<T>
    bool GetProc(T** proc, const std::string& procName, std::string* error = nullptr) const {
        DAWN_ASSERT(proc != nullptr);

        *proc = reinterpret_cast<T*>(GetProc(procName, error));
        return *proc != nullptr;
    }

  private:
    void* mHandle = nullptr;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_DYNAMICLIB_H_
