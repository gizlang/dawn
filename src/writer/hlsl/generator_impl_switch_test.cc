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

#include <memory>

#include "src/ast/break_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/switch_statement.h"
#include "src/program.h"
#include "src/type/i32_type.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Switch = TestHelper;

TEST_F(HlslGeneratorImplTest_Switch, Emit_Switch) {
  auto* def_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::BreakStatement>(),
  });
  auto* def = create<ast::CaseStatement>(ast::CaseSelectorList{}, def_body);

  ast::CaseSelectorList case_val;
  case_val.push_back(Literal(5));

  auto* case_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::BreakStatement>(),
  });

  auto* case_stmt = create<ast::CaseStatement>(case_val, case_body);

  ast::CaseStatementList body;
  body.push_back(case_stmt);
  body.push_back(def);

  auto* cond = Expr("cond");
  auto* s = create<ast::SwitchStatement>(cond, body);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, s)) << gen.error();
  EXPECT_EQ(result(), R"(  switch(cond) {
    case 5: {
      break;
    }
    default: {
      break;
    }
  }
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
