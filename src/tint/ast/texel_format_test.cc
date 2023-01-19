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
    {"bgra8unorm", TexelFormat::kBgra8Unorm},   {"r32float", TexelFormat::kR32Float},
    {"r32sint", TexelFormat::kR32Sint},         {"r32uint", TexelFormat::kR32Uint},
    {"rg32float", TexelFormat::kRg32Float},     {"rg32sint", TexelFormat::kRg32Sint},
    {"rg32uint", TexelFormat::kRg32Uint},       {"rgba16float", TexelFormat::kRgba16Float},
    {"rgba16sint", TexelFormat::kRgba16Sint},   {"rgba16uint", TexelFormat::kRgba16Uint},
    {"rgba32float", TexelFormat::kRgba32Float}, {"rgba32sint", TexelFormat::kRgba32Sint},
    {"rgba32uint", TexelFormat::kRgba32Uint},   {"rgba8sint", TexelFormat::kRgba8Sint},
    {"rgba8snorm", TexelFormat::kRgba8Snorm},   {"rgba8uint", TexelFormat::kRgba8Uint},
    {"rgba8unorm", TexelFormat::kRgba8Unorm},
};

static constexpr Case kInvalidCases[] = {
    {"bgraunccrm", TexelFormat::kUndefined},      {"blranr3", TexelFormat::kUndefined},
    {"bVra8unorm", TexelFormat::kUndefined},      {"132float", TexelFormat::kUndefined},
    {"32Jlqqat", TexelFormat::kUndefined},        {"ll3277loat", TexelFormat::kUndefined},
    {"ppqq2snHH", TexelFormat::kUndefined},       {"r3cv", TexelFormat::kUndefined},
    {"b2siGt", TexelFormat::kUndefined},          {"r32uiivt", TexelFormat::kUndefined},
    {"8WW2uint", TexelFormat::kUndefined},        {"rxxuint", TexelFormat::kUndefined},
    {"rX2flggat", TexelFormat::kUndefined},       {"rg3XVut", TexelFormat::kUndefined},
    {"3g32float", TexelFormat::kUndefined},       {"rg3Esint", TexelFormat::kUndefined},
    {"PP32TTint", TexelFormat::kUndefined},       {"xxg32ddnt", TexelFormat::kUndefined},
    {"44g32uint", TexelFormat::kUndefined},       {"rSS32uinVV", TexelFormat::kUndefined},
    {"R322Rint", TexelFormat::kUndefined},        {"rgba16fF9a", TexelFormat::kUndefined},
    {"rgba16floa", TexelFormat::kUndefined},      {"rOObVR16floH", TexelFormat::kUndefined},
    {"ryba1sint", TexelFormat::kUndefined},       {"r77ba1nnsllrrt", TexelFormat::kUndefined},
    {"rgb4006sint", TexelFormat::kUndefined},     {"rb1uioot", TexelFormat::kUndefined},
    {"rga1uzznt", TexelFormat::kUndefined},       {"r11b1uppiit", TexelFormat::kUndefined},
    {"rgba32fXXoat", TexelFormat::kUndefined},    {"rgbII99355float", TexelFormat::kUndefined},
    {"rgbaa32fSSrHHYt", TexelFormat::kUndefined}, {"rbkk2Hit", TexelFormat::kUndefined},
    {"jgba3sgRR", TexelFormat::kUndefined},       {"rgbab2si", TexelFormat::kUndefined},
    {"rgba32jint", TexelFormat::kUndefined},      {"rba32uint", TexelFormat::kUndefined},
    {"rgba2uqn", TexelFormat::kUndefined},        {"rbaNNsint", TexelFormat::kUndefined},
    {"rga8invv", TexelFormat::kUndefined},        {"gba8sQQnt", TexelFormat::kUndefined},
    {"rgbsnrrff", TexelFormat::kUndefined},       {"rgba8snojm", TexelFormat::kUndefined},
    {"NNgba8sww2m", TexelFormat::kUndefined},     {"rgba8uit", TexelFormat::kUndefined},
    {"rrgba8uint", TexelFormat::kUndefined},      {"rgba8uiGt", TexelFormat::kUndefined},
    {"rgba8unFFrm", TexelFormat::kUndefined},     {"g8unErm", TexelFormat::kUndefined},
    {"rgb8urrorm", TexelFormat::kUndefined},
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
