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

#include "gtest/gtest.h"
#include "src/ast/binary_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/int_literal.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

struct BinaryData {
  ast::BinaryOp op;
  std::string name;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
  out << data.op;
  return out;
}

using BinaryArithSignedIntegerTest = testing::TestWithParam<BinaryData>;
TEST_P(BinaryArithSignedIntegerTest, Scalar) {
  auto param = GetParam();

  ast::type::I32Type i32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 3));
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 4));

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 4u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 3
%3 = OpConstant %1 4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%4 = " + param.name + " %1 %2 %3\n");
}
TEST_P(BinaryArithSignedIntegerTest, Vector) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec3(&i32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  auto lhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  auto rhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%5 = " + param.name + " %1 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryArithSignedIntegerTest,
    testing::Values(
        BinaryData{ast::BinaryOp::kAdd, "OpIAdd"},
        BinaryData{ast::BinaryOp::kAnd, "OpBitwiseAnd"},
        BinaryData{ast::BinaryOp::kDivide, "OpSDiv"},
        BinaryData{ast::BinaryOp::kModulo, "OpSMod"},
        BinaryData{ast::BinaryOp::kMultiply, "OpIMul"},
        BinaryData{ast::BinaryOp::kOr, "OpBitwiseOr"},
        BinaryData{ast::BinaryOp::kShiftLeft, "OpShiftLeftLogical"},
        BinaryData{ast::BinaryOp::kShiftRight, "OpShiftRightLogical"},
        BinaryData{ast::BinaryOp::kShiftRightArith, "OpShiftRightArithmetic"},
        BinaryData{ast::BinaryOp::kSubtract, "OpISub"},
        BinaryData{ast::BinaryOp::kXor, "OpBitwiseXor"}));

using BinaryArithUnsignedIntegerTest = testing::TestWithParam<BinaryData>;
TEST_P(BinaryArithUnsignedIntegerTest, Scalar) {
  auto param = GetParam();

  ast::type::U32Type u32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 3));
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 4));

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 4u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 3
%3 = OpConstant %1 4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%4 = " + param.name + " %1 %2 %3\n");
}
TEST_P(BinaryArithUnsignedIntegerTest, Vector) {
  auto param = GetParam();

  ast::type::U32Type u32;
  ast::type::VectorType vec3(&u32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 1)));
  auto lhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 1)));
  auto rhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%5 = " + param.name + " %1 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryArithUnsignedIntegerTest,
    testing::Values(
        BinaryData{ast::BinaryOp::kAdd, "OpIAdd"},
        BinaryData{ast::BinaryOp::kAnd, "OpBitwiseAnd"},
        BinaryData{ast::BinaryOp::kDivide, "OpUDiv"},
        BinaryData{ast::BinaryOp::kModulo, "OpUMod"},
        BinaryData{ast::BinaryOp::kMultiply, "OpIMul"},
        BinaryData{ast::BinaryOp::kOr, "OpBitwiseOr"},
        BinaryData{ast::BinaryOp::kShiftLeft, "OpShiftLeftLogical"},
        BinaryData{ast::BinaryOp::kShiftRight, "OpShiftRightLogical"},
        BinaryData{ast::BinaryOp::kShiftRightArith, "OpShiftRightArithmetic"},
        BinaryData{ast::BinaryOp::kSubtract, "OpISub"},
        BinaryData{ast::BinaryOp::kXor, "OpBitwiseXor"}));

using BinaryArithFloatTest = testing::TestWithParam<BinaryData>;
TEST_P(BinaryArithFloatTest, Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.2f));
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 4.5f));

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 4u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 3.20000005
%3 = OpConstant %1 4.5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%4 = " + param.name + " %1 %2 %3\n");
}

TEST_P(BinaryArithFloatTest, Vector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  auto lhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  auto rhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%5 = " + param.name + " %1 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryArithFloatTest,
    testing::Values(BinaryData{ast::BinaryOp::kAdd, "OpFAdd"},
                    BinaryData{ast::BinaryOp::kDivide, "OpFDiv"},
                    BinaryData{ast::BinaryOp::kModulo, "OpFMod"},
                    BinaryData{ast::BinaryOp::kMultiply, "OpFMul"},
                    BinaryData{ast::BinaryOp::kSubtract, "OpFSub"}));

using BinaryCompareUnsignedIntegerTest = testing::TestWithParam<BinaryData>;
TEST_P(BinaryCompareUnsignedIntegerTest, Scalar) {
  auto param = GetParam();

  ast::type::U32Type u32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 3));
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 4));

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 4u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstant %1 3
%3 = OpConstant %1 4
%5 = OpTypeBool
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%4 = " + param.name + " %5 %2 %3\n");
}

TEST_P(BinaryCompareUnsignedIntegerTest, Vector) {
  auto param = GetParam();

  ast::type::U32Type u32;
  ast::type::VectorType vec3(&u32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 1)));
  auto lhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&u32, 1)));
  auto rhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
%7 = OpTypeBool
%6 = OpTypeVector %7 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%5 = " + param.name + " %6 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryCompareUnsignedIntegerTest,
    testing::Values(
        BinaryData{ast::BinaryOp::kEqual, "OpIEqual"},
        BinaryData{ast::BinaryOp::kGreaterThan, "OpUGreaterThan"},
        BinaryData{ast::BinaryOp::kGreaterThanEqual, "OpUGreaterThanEqual"},
        BinaryData{ast::BinaryOp::kLessThan, "OpULessThan"},
        BinaryData{ast::BinaryOp::kLessThanEqual, "OpULessThanEqual"},
        BinaryData{ast::BinaryOp::kNotEqual, "OpINotEqual"}));

using BinaryCompareSignedIntegerTest = testing::TestWithParam<BinaryData>;
TEST_P(BinaryCompareSignedIntegerTest, Scalar) {
  auto param = GetParam();

  ast::type::I32Type i32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 3));
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 4));

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 4u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 3
%3 = OpConstant %1 4
%5 = OpTypeBool
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%4 = " + param.name + " %5 %2 %3\n");
}

TEST_P(BinaryCompareSignedIntegerTest, Vector) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec3(&i32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  auto lhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  auto rhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
%7 = OpTypeBool
%6 = OpTypeVector %7 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%5 = " + param.name + " %6 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryCompareSignedIntegerTest,
    testing::Values(
        BinaryData{ast::BinaryOp::kEqual, "OpIEqual"},
        BinaryData{ast::BinaryOp::kGreaterThan, "OpSGreaterThan"},
        BinaryData{ast::BinaryOp::kGreaterThanEqual, "OpSGreaterThanEqual"},
        BinaryData{ast::BinaryOp::kLessThan, "OpSLessThan"},
        BinaryData{ast::BinaryOp::kLessThanEqual, "OpSLessThanEqual"},
        BinaryData{ast::BinaryOp::kNotEqual, "OpINotEqual"}));

using BinaryCompareFloatTest = testing::TestWithParam<BinaryData>;
TEST_P(BinaryCompareFloatTest, Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.2f));
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 4.5f));

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 4u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 3.20000005
%3 = OpConstant %1 4.5
%5 = OpTypeBool
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%4 = " + param.name + " %5 %2 %3\n");
}

TEST_P(BinaryCompareFloatTest, Vector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  auto lhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  auto rhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
%7 = OpTypeBool
%6 = OpTypeVector %7 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%5 = " + param.name + " %6 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryCompareFloatTest,
    testing::Values(
        BinaryData{ast::BinaryOp::kEqual, "OpFOrdEqual"},
        BinaryData{ast::BinaryOp::kGreaterThan, "OpFOrdGreaterThan"},
        BinaryData{ast::BinaryOp::kGreaterThanEqual, "OpFOrdGreaterThanEqual"},
        BinaryData{ast::BinaryOp::kLessThan, "OpFOrdLessThan"},
        BinaryData{ast::BinaryOp::kLessThanEqual, "OpFOrdLessThanEqual"},
        BinaryData{ast::BinaryOp::kNotEqual, "OpFOrdNotEqual"}));

TEST_F(BuilderTest, Binary_Multiply_VectorScalar) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  auto lhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply, std::move(lhs),
                             std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%5 = OpVectorTimesScalar %1 %4 %3\n");
}

TEST_F(BuilderTest, Binary_Multiply_ScalarVector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f));

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  auto rhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply, std::move(lhs),
                             std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 1
%3 = OpTypeVector %1 3
%4 = OpConstantComposite %3 %2 %2 %2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%5 = OpVectorTimesScalar %3 %4 %2\n");
}

TEST_F(BuilderTest, Binary_Multiply_MatrixScalar) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat3(&f32, 3, 3);

  auto var = std::make_unique<ast::Variable>(
      "mat", ast::StorageClass::kFunction, &mat3);
  auto lhs = std::make_unique<ast::IdentifierExpression>("mat");
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply, std::move(lhs),
                             std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 8u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%1 = OpVariable %2 Function
%7 = OpConstant %5 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%8 = OpMatrixTimesScalar %3 %6 %7
)");
}

TEST_F(BuilderTest, Binary_Multiply_ScalarMatrix) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat3(&f32, 3, 3);

  auto var = std::make_unique<ast::Variable>(
      "mat", ast::StorageClass::kFunction, &mat3);
  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f));
  auto rhs = std::make_unique<ast::IdentifierExpression>("mat");

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply, std::move(lhs),
                             std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 8u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%1 = OpVariable %2 Function
%6 = OpConstant %5 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%8 = OpMatrixTimesScalar %3 %7 %6
)");
}

TEST_F(BuilderTest, Binary_Multiply_MatrixVector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::MatrixType mat3(&f32, 3, 3);

  auto var = std::make_unique<ast::Variable>(
      "mat", ast::StorageClass::kFunction, &mat3);
  auto lhs = std::make_unique<ast::IdentifierExpression>("mat");

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  auto rhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply, std::move(lhs),
                             std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 9u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%1 = OpVariable %2 Function
%7 = OpConstant %5 1
%8 = OpConstantComposite %4 %7 %7 %7
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%9 = OpMatrixTimesVector %4 %6 %8
)");
}

TEST_F(BuilderTest, Binary_Multiply_VectorMatrix) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::MatrixType mat3(&f32, 3, 3);

  auto var = std::make_unique<ast::Variable>(
      "mat", ast::StorageClass::kFunction, &mat3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  auto lhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  auto rhs = std::make_unique<ast::IdentifierExpression>("mat");

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply, std::move(lhs),
                             std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 9u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%1 = OpVariable %2 Function
%6 = OpConstant %5 1
%7 = OpConstantComposite %4 %6 %6 %6
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%8 = OpLoad %3 %1
%9 = OpVectorTimesMatrix %4 %7 %8
)");
}

TEST_F(BuilderTest, Binary_Multiply_MatrixMatrix) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::MatrixType mat3(&f32, 3, 3);

  auto var = std::make_unique<ast::Variable>(
      "mat", ast::StorageClass::kFunction, &mat3);
  auto lhs = std::make_unique<ast::IdentifierExpression>("mat");
  auto rhs = std::make_unique<ast::IdentifierExpression>("mat");

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply, std::move(lhs),
                             std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 8u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Function %3
%1 = OpVariable %2 Function
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%7 = OpLoad %3 %1
%8 = OpMatrixTimesMatrix %3 %6 %7
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
