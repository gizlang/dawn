// Copyright 2020 The Tint Authors.
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

#ifndef SRC_AST_TEST_HELPER_H_
#define SRC_AST_TEST_HELPER_H_

#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "src/ast/builder.h"
#include "src/demangler.h"
#include "src/program.h"

namespace tint {
namespace ast {

/// Helper class for testing
template <typename BASE>
class TestHelperBase : public BASE, public BuilderWithProgram {
 public:
  /// Demangles the given string
  /// @param s the string to demangle
  /// @returns the demangled string
  std::string demangle(const std::string& s) {
    return demanger.Demangle(*mod, s);
  }

  /// A demangler
  Demangler demanger;
};
using TestHelper = TestHelperBase<testing::Test>;

template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TEST_HELPER_H_
