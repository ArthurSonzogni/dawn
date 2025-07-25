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

#include <gtest/gtest.h>

#include "src/tint/lang/wgsl/enums.h"
#include "src/tint/lang/wgsl/feature_status.h"

namespace tint::wgsl {
namespace {

TEST(LanguageFeatureStatusTest, AllFeaturesAreKnown) {
    for (auto f : kAllLanguageFeatures) {
        EXPECT_NE(FeatureStatus::kUnknown, GetLanguageFeatureStatus(f));
    }
}

TEST(LanguageFeatureStatusTest, ChromiumTestingValues) {
    EXPECT_EQ(FeatureStatus::kUnimplemented,
              GetLanguageFeatureStatus(tint::wgsl::LanguageFeature::kChromiumTestingUnimplemented));
    EXPECT_EQ(
        FeatureStatus::kUnsafeExperimental,
        GetLanguageFeatureStatus(tint::wgsl::LanguageFeature::kChromiumTestingUnsafeExperimental));
    EXPECT_EQ(FeatureStatus::kExperimental,
              GetLanguageFeatureStatus(tint::wgsl::LanguageFeature::kChromiumTestingExperimental));
    EXPECT_EQ(FeatureStatus::kShippedWithKillswitch,
              GetLanguageFeatureStatus(
                  tint::wgsl::LanguageFeature::kChromiumTestingShippedWithKillswitch));
    EXPECT_EQ(FeatureStatus::kShipped,
              GetLanguageFeatureStatus(tint::wgsl::LanguageFeature::kChromiumTestingShipped));
}

TEST(LanguageFeatureStatusTest, ChromiumDeveloperFeatures) {
    EXPECT_EQ(FeatureStatus::kUnsafeExperimental,
              GetLanguageFeatureStatus(tint::wgsl::LanguageFeature::kChromiumPrint));
}

}  // namespace
}  // namespace tint::wgsl
