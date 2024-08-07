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

////////////////////////////////////////////////////////////////////////////////
// File generated by 'tools/src/cmd/gen' using the template:
//   src/tint/lang/core/attribute_test.cc.tmpl
//
// To regenerate run: './tools/run gen'
//
//                       Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#include "src/tint/lang/core/attribute.h"

#include <gtest/gtest.h>

#include <string>

#include "src/tint/utils/text/string.h"

namespace tint::core {
namespace {

namespace parse_print_tests {

struct Case {
    const char* string;
    Attribute value;
};

inline std::ostream& operator<<(std::ostream& out, Case c) {
    return out << "'" << std::string(c.string) << "'";
}

static constexpr Case kValidCases[] = {
    {"align", Attribute::kAlign},
    {"binding", Attribute::kBinding},
    {"blend_src", Attribute::kBlendSrc},
    {"builtin", Attribute::kBuiltin},
    {"color", Attribute::kColor},
    {"compute", Attribute::kCompute},
    {"diagnostic", Attribute::kDiagnostic},
    {"fragment", Attribute::kFragment},
    {"group", Attribute::kGroup},
    {"id", Attribute::kId},
    {"input_attachment_index", Attribute::kInputAttachmentIndex},
    {"interpolate", Attribute::kInterpolate},
    {"invariant", Attribute::kInvariant},
    {"location", Attribute::kLocation},
    {"must_use", Attribute::kMustUse},
    {"size", Attribute::kSize},
    {"vertex", Attribute::kVertex},
    {"workgroup_size", Attribute::kWorkgroupSize},
};

static constexpr Case kInvalidCases[] = {
    {"alccn", Attribute::kUndefined},
    {"3g", Attribute::kUndefined},
    {"aVign", Attribute::kUndefined},
    {"bind1ng", Attribute::kUndefined},
    {"bqnJing", Attribute::kUndefined},
    {"bindin7ll", Attribute::kUndefined},
    {"blen_sppqHH", Attribute::kUndefined},
    {"blen_r", Attribute::kUndefined},
    {"bbndGsrc", Attribute::kUndefined},
    {"builiivn", Attribute::kUndefined},
    {"8WWiltin", Attribute::kUndefined},
    {"bxxltin", Attribute::kUndefined},
    {"cggor", Attribute::kUndefined},
    {"VoX", Attribute::kUndefined},
    {"colo3", Attribute::kUndefined},
    {"Eompute", Attribute::kUndefined},
    {"cPTTpute", Attribute::kUndefined},
    {"compudxx", Attribute::kUndefined},
    {"diagno44tic", Attribute::kUndefined},
    {"diaVVSnostic", Attribute::kUndefined},
    {"2iRRnostic", Attribute::kUndefined},
    {"rFg9ent", Attribute::kUndefined},
    {"frament", Attribute::kUndefined},
    {"ROOrHgeVt", Attribute::kUndefined},
    {"royp", Attribute::kUndefined},
    {"n77rrloGp", Attribute::kUndefined},
    {"grou40", Attribute::kUndefined},
    {"HH", Attribute::kUndefined},
    {"p", Attribute::kUndefined},
    {"1ii", Attribute::kUndefined},
    {"input_attachment_iXXdex", Attribute::kUndefined},
    {"input_attnnIIhme99t55index", Attribute::kUndefined},
    {"inputYarrHHacaamentSSindex", Attribute::kUndefined},
    {"nHkkolate", Attribute::kUndefined},
    {"jRRterogte", Attribute::kUndefined},
    {"iterpolbe", Attribute::kUndefined},
    {"invariajt", Attribute::kUndefined},
    {"ivariant", Attribute::kUndefined},
    {"ivariqt", Attribute::kUndefined},
    {"locaioNN", Attribute::kUndefined},
    {"ocvvion", Attribute::kUndefined},
    {"loatQQon", Attribute::kUndefined},
    {"musf_re", Attribute::kUndefined},
    {"mujt_use", Attribute::kUndefined},
    {"muswNN82e", Attribute::kUndefined},
    {"sze", Attribute::kUndefined},
    {"sirre", Attribute::kUndefined},
    {"sGze", Attribute::kUndefined},
    {"verFFex", Attribute::kUndefined},
    {"vre", Attribute::kUndefined},
    {"vrrrte", Attribute::kUndefined},
    {"orkgroup_sze", Attribute::kUndefined},
    {"DXkgroJJp_size", Attribute::kUndefined},
    {"wo8kroup_sz", Attribute::kUndefined},
};

using AttributeParseTest = testing::TestWithParam<Case>;

TEST_P(AttributeParseTest, Parse) {
    const char* string = GetParam().string;
    Attribute expect = GetParam().value;
    EXPECT_EQ(expect, ParseAttribute(string));
}

INSTANTIATE_TEST_SUITE_P(ValidCases, AttributeParseTest, testing::ValuesIn(kValidCases));
INSTANTIATE_TEST_SUITE_P(InvalidCases, AttributeParseTest, testing::ValuesIn(kInvalidCases));

using AttributePrintTest = testing::TestWithParam<Case>;

TEST_P(AttributePrintTest, Print) {
    Attribute value = GetParam().value;
    const char* expect = GetParam().string;
    EXPECT_EQ(expect, ToString(value));
}

INSTANTIATE_TEST_SUITE_P(ValidCases, AttributePrintTest, testing::ValuesIn(kValidCases));

}  // namespace parse_print_tests

}  // namespace
}  // namespace tint::core
