// Copyright 2022 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

////////////////////////////////////////////////////////////////////////////////
// File generated by tools/src/cmd/gen
// using the template:
//   src/tint/ast/texel_format_test.cc.tmpl
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#include "src/tint/ast/texel_format.h"

#include <string>

#include "src/tint/ast/test_helper.h"
#include "src/tint/utils/string.h"

namespace tint::ast {
namespace {

namespace parse_print_tests {

struct Case {
    const char* string;
    TexelFormat value;
};

inline std::ostream& operator<<(std::ostream& out, Case c) {
    return out << "'" << std::string(c.string) << "'";
}

static constexpr Case kValidCases[] = {
    {"rgba8unorm", TexelFormat::kRgba8Unorm},   {"rgba8snorm", TexelFormat::kRgba8Snorm},
    {"rgba8uint", TexelFormat::kRgba8Uint},     {"rgba8sint", TexelFormat::kRgba8Sint},
    {"rgba16uint", TexelFormat::kRgba16Uint},   {"rgba16sint", TexelFormat::kRgba16Sint},
    {"rgba16float", TexelFormat::kRgba16Float}, {"r32uint", TexelFormat::kR32Uint},
    {"r32sint", TexelFormat::kR32Sint},         {"r32float", TexelFormat::kR32Float},
    {"rg32uint", TexelFormat::kRg32Uint},       {"rg32sint", TexelFormat::kRg32Sint},
    {"rg32float", TexelFormat::kRg32Float},     {"rgba32uint", TexelFormat::kRgba32Uint},
    {"rgba32sint", TexelFormat::kRgba32Sint},   {"rgba32float", TexelFormat::kRgba32Float},
};

static constexpr Case kInvalidCases[] = {
    {"rgbaunccrm", TexelFormat::kInvalid},   {"rlbanr3", TexelFormat::kInvalid},
    {"rVba8unorm", TexelFormat::kInvalid},   {"rgba1snorm", TexelFormat::kInvalid},
    {"rgbJqqnorm", TexelFormat::kInvalid},   {"rgb7ll8snorm", TexelFormat::kInvalid},
    {"rgbauippqHH", TexelFormat::kInvalid},  {"rgbaun", TexelFormat::kInvalid},
    {"rba8Gint", TexelFormat::kInvalid},     {"rgvia8sint", TexelFormat::kInvalid},
    {"rgba8WWint", TexelFormat::kInvalid},   {"rgbasxxMt", TexelFormat::kInvalid},
    {"rXba16ungg", TexelFormat::kInvalid},   {"rba1XuVt", TexelFormat::kInvalid},
    {"rgba16uin3", TexelFormat::kInvalid},   {"rgba16sinE", TexelFormat::kInvalid},
    {"TTgba16sPPn", TexelFormat::kInvalid},  {"rgbad6xxint", TexelFormat::kInvalid},
    {"rgba446float", TexelFormat::kInvalid}, {"SSVVba16float", TexelFormat::kInvalid},
    {"rgbRR6float", TexelFormat::kInvalid},  {"rFui9t", TexelFormat::kInvalid},
    {"r32int", TexelFormat::kInvalid},       {"VOORRHnt", TexelFormat::kInvalid},
    {"r3siyt", TexelFormat::kInvalid},       {"lln3rrs77nt", TexelFormat::kInvalid},
    {"r32s4n00", TexelFormat::kInvalid},     {"32ooat", TexelFormat::kInvalid},
    {"r32fzzt", TexelFormat::kInvalid},      {"r3iippl1a", TexelFormat::kInvalid},
    {"XXg32uint", TexelFormat::kInvalid},    {"rII39955nnnt", TexelFormat::kInvalid},
    {"aagHH2uinYSS", TexelFormat::kInvalid}, {"rkk3it", TexelFormat::kInvalid},
    {"gj3sRRn", TexelFormat::kInvalid},      {"r3bsnt", TexelFormat::kInvalid},
    {"rg32flojt", TexelFormat::kInvalid},    {"r32floa", TexelFormat::kInvalid},
    {"rg32lot", TexelFormat::kInvalid},      {"rgb3uit", TexelFormat::kInvalid},
    {"rgjj3uint", TexelFormat::kInvalid},    {"rgb2urnff", TexelFormat::kInvalid},
    {"rgba32sijt", TexelFormat::kInvalid},   {"NNgba32ww2t", TexelFormat::kInvalid},
    {"rgba32snt", TexelFormat::kInvalid},    {"rgba32rrloat", TexelFormat::kInvalid},
    {"rgGa32float", TexelFormat::kInvalid},  {"FFgba32float", TexelFormat::kInvalid},
};

using TexelFormatParseTest = testing::TestWithParam<Case>;

TEST_P(TexelFormatParseTest, Parse) {
    const char* string = GetParam().string;
    TexelFormat expect = GetParam().value;
    EXPECT_EQ(expect, ParseTexelFormat(string));
}

INSTANTIATE_TEST_SUITE_P(ValidCases, TexelFormatParseTest, testing::ValuesIn(kValidCases));
INSTANTIATE_TEST_SUITE_P(InvalidCases, TexelFormatParseTest, testing::ValuesIn(kInvalidCases));

using TexelFormatPrintTest = testing::TestWithParam<Case>;

TEST_P(TexelFormatPrintTest, Print) {
    TexelFormat value = GetParam().value;
    const char* expect = GetParam().string;
    EXPECT_EQ(expect, utils::ToString(value));
}

INSTANTIATE_TEST_SUITE_P(ValidCases, TexelFormatPrintTest, testing::ValuesIn(kValidCases));

}  // namespace parse_print_tests

}  // namespace
}  // namespace tint::ast
