// Copyright 2021 The Dawn & Tint Authors
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

#include <gtest/gtest.h>

#include "dawn/common/GPUInfo.h"

namespace dawn {
namespace {

//  Intel D3D12
const gpu_info::IntelWindowsDriverVersion version1 = {20, 19, 15, 5107};
const gpu_info::IntelWindowsDriverVersion version2 = {21, 20, 16, 5077};
const gpu_info::IntelWindowsDriverVersion version3 = {27, 20, 100, 9946};
const gpu_info::IntelWindowsDriverVersion version4 = {27, 20, 101, 2003};
// Intel Vulkan
const gpu_info::IntelWindowsDriverVersion version5 = {100, 9466};
const gpu_info::IntelWindowsDriverVersion version6 = {101, 3222};
const gpu_info::IntelWindowsDriverVersion version7 = {101, 3790};

TEST(GPUInfo, CompareIntelWindowsDriverVersion) {
    EXPECT_LT(gpu_info::IntelWindowsDriverVersion(version1), version2);
    EXPECT_LT(gpu_info::IntelWindowsDriverVersion(version2), version3);
    EXPECT_LT(gpu_info::IntelWindowsDriverVersion(version3), version4);
    EXPECT_LT(gpu_info::IntelWindowsDriverVersion(version5), version6);
    EXPECT_LT(gpu_info::IntelWindowsDriverVersion(version6), version7);
    // Windows Vulkan driver releases together with D3D12 driver, so they share the same version.
    // Expect Intel D3D12 driver and Vulkan driver to be comparable.
    EXPECT_LT(gpu_info::IntelWindowsDriverVersion(version3), version6);
    EXPECT_LT(gpu_info::IntelWindowsDriverVersion(version4), version7);
}

}  // anonymous namespace
}  // namespace dawn
