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
#include "src/ast/identifier_expression.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/multisampled_texture_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/sampled_texture_type.h"
#include "src/ast/type/sampler_type.h"
#include "src/ast/type/storage_texture_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/texture_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest_Type = TestHelper;

TEST_F(BuilderTest_Type, GenerateAlias) {
  ast::type::F32 f32;
  ast::type::Alias alias_type(mod->RegisterSymbol("my_type"), "my_type", &f32);

  auto id = b.GenerateTypeIfNeeded(&alias_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedAlias) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Alias alias_type(mod->RegisterSymbol("my_type"), "my_type", &f32);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&alias_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&alias_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateRuntimeArray) {
  ast::type::I32 i32;
  ast::type::Array ary(&i32, 0, ast::ArrayDecorationList{});

  auto id = b.GenerateTypeIfNeeded(&ary);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeRuntimeArray %2
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedRuntimeArray) {
  ast::type::I32 i32;
  ast::type::Array ary(&i32, 0, ast::ArrayDecorationList{});

  EXPECT_EQ(b.GenerateTypeIfNeeded(&ary), 1u);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&ary), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeRuntimeArray %2
)");
}

TEST_F(BuilderTest_Type, GenerateArray) {
  ast::type::I32 i32;
  ast::type::Array ary(&i32, 4, ast::ArrayDecorationList{});

  auto id = b.GenerateTypeIfNeeded(&ary);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 4
%1 = OpTypeArray %2 %4
)");
}

TEST_F(BuilderTest_Type, GenerateArray_WithStride) {
  ast::type::I32 i32;

  ast::type::Array ary(&i32, 4,
                       ast::ArrayDecorationList{
                           create<ast::StrideDecoration>(Source{}, 16u),
                       });

  auto id = b.GenerateTypeIfNeeded(&ary);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 ArrayStride 16
)");

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 4
%1 = OpTypeArray %2 %4
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedArray) {
  ast::type::I32 i32;
  ast::type::Array ary(&i32, 4, ast::ArrayDecorationList{});

  EXPECT_EQ(b.GenerateTypeIfNeeded(&ary), 1u);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&ary), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 4
%1 = OpTypeArray %2 %4
)");
}

TEST_F(BuilderTest_Type, GenerateBool) {
  ast::type::Bool bool_type;

  auto id = b.GenerateTypeIfNeeded(&bool_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeBool
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedBool) {
  ast::type::I32 i32;
  ast::type::Bool bool_type;

  EXPECT_EQ(b.GenerateTypeIfNeeded(&bool_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&bool_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateF32) {
  ast::type::F32 f32;

  auto id = b.GenerateTypeIfNeeded(&f32);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeFloat 32
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedF32) {
  ast::type::I32 i32;
  ast::type::F32 f32;

  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateI32) {
  ast::type::I32 i32;

  auto id = b.GenerateTypeIfNeeded(&i32);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeInt 32 1
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedI32) {
  ast::type::I32 i32;
  ast::type::F32 f32;

  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateMatrix) {
  ast::type::F32 f32;
  ast::type::Matrix mat_type(&f32, 3, 2);

  auto id = b.GenerateTypeIfNeeded(&mat_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(b.types().size(), 3u);
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 2
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedMatrix) {
  ast::type::I32 i32;
  ast::type::Matrix mat_type(&i32, 3, 4);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&mat_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 3u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&mat_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GeneratePtr) {
  ast::type::I32 i32;
  ast::type::Pointer ptr(&i32, ast::StorageClass::kOutput);

  auto id = b.GenerateTypeIfNeeded(&ptr);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypePointer Output %2
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedPtr) {
  ast::type::I32 i32;
  ast::type::Pointer ptr(&i32, ast::StorageClass::kOutput);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&ptr), 1u);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&ptr), 1u);
}

TEST_F(BuilderTest_Type, GenerateStruct_Empty) {
  auto* s = create<ast::Struct>(Source{}, ast::StructMemberList{},
                                ast::StructDecorationList{});
  ast::type::Struct s_type(mod->RegisterSymbol("S"), "S", s);

  auto id = b.GenerateTypeIfNeeded(&s_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "S"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeStruct
)");
}

TEST_F(BuilderTest_Type, GenerateStruct) {
  auto* s = create<ast::Struct>(ast::StructMemberList{Member("a", ty.f32)},
                                ast::StructDecorationList{});
  ast::type::Struct s_type(mod->RegisterSymbol("my_struct"), "my_struct", s);

  auto id = b.GenerateTypeIfNeeded(&s_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeStruct %2
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "my_struct"
OpMemberName %1 0 "a"
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_Decorated) {
  ast::StructDecorationList struct_decos;
  struct_decos.push_back(create<ast::StructBlockDecoration>(Source{}));

  auto* s = create<ast::Struct>(ast::StructMemberList{Member("a", ty.f32)},
                                struct_decos);
  ast::type::Struct s_type(mod->RegisterSymbol("my_struct"), "my_struct", s);

  auto id = b.GenerateTypeIfNeeded(&s_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeStruct %2
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "my_struct"
OpMemberName %1 0 "a"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 Block
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_DecoratedMembers) {
  auto* s = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.f32, {MemberOffset(0)}),
                            Member("b", ty.f32, {MemberOffset(8)})},
      ast::StructDecorationList{});
  ast::type::Struct s_type(mod->RegisterSymbol("S"), "S", s);

  auto id = b.GenerateTypeIfNeeded(&s_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeStruct %2 %2
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "S"
OpMemberName %1 0 "a"
OpMemberName %1 1 "b"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %1 0 Offset 0
OpMemberDecorate %1 1 Offset 8
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_NonLayout_Matrix) {
  auto* s =
      create<ast::Struct>(ast::StructMemberList{Member("a", ty.mat2x2<f32>()),
                                                Member("b", ty.mat2x3<f32>()),
                                                Member("c", ty.mat4x4<f32>())},
                          ast::StructDecorationList{});
  ast::type::Struct s_type(mod->RegisterSymbol("S"), "S", s);

  auto id = b.GenerateTypeIfNeeded(&s_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 2
%2 = OpTypeMatrix %3 2
%6 = OpTypeVector %4 3
%5 = OpTypeMatrix %6 2
%8 = OpTypeVector %4 4
%7 = OpTypeMatrix %8 4
%1 = OpTypeStruct %2 %5 %7
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "S"
OpMemberName %1 0 "a"
OpMemberName %1 1 "b"
OpMemberName %1 2 "c"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), "");
}

TEST_F(BuilderTest_Type, GenerateStruct_DecoratedMembers_LayoutMatrix) {
  // We have to infer layout for matrix when it also has an offset.
  auto* s = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.mat2x2<f32>(), {MemberOffset(0)}),
                            Member("b", ty.mat2x3<f32>(), {MemberOffset(16)}),
                            Member("c", ty.mat4x4<f32>(), {MemberOffset(48)})},
      ast::StructDecorationList{});
  ast::type::Struct s_type(mod->RegisterSymbol("S"), "S", s);

  auto id = b.GenerateTypeIfNeeded(&s_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 2
%2 = OpTypeMatrix %3 2
%6 = OpTypeVector %4 3
%5 = OpTypeMatrix %6 2
%8 = OpTypeVector %4 4
%7 = OpTypeMatrix %8 4
%1 = OpTypeStruct %2 %5 %7
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "S"
OpMemberName %1 0 "a"
OpMemberName %1 1 "b"
OpMemberName %1 2 "c"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %1 0 Offset 0
OpMemberDecorate %1 0 ColMajor
OpMemberDecorate %1 0 MatrixStride 8
OpMemberDecorate %1 1 Offset 16
OpMemberDecorate %1 1 ColMajor
OpMemberDecorate %1 1 MatrixStride 16
OpMemberDecorate %1 2 Offset 48
OpMemberDecorate %1 2 ColMajor
OpMemberDecorate %1 2 MatrixStride 16
)");
}

TEST_F(BuilderTest_Type, GenerateStruct_DecoratedMembers_LayoutArraysOfMatrix) {
  // We have to infer layout for matrix when it also has an offset.
  // The decoration goes on the struct member, even if the matrix is buried
  // in levels of arrays.
  ast::type::Array arr_mat2x2(
      ty.mat2x2<f32>(), 1, ast::ArrayDecorationList{});  // Singly nested array

  ast::type::Array arr_mat2x3(ty.mat2x3<f32>(), 1, ast::ArrayDecorationList{});
  ast::type::Array arr_arr_mat2x3(
      ty.mat2x3<f32>(), 1, ast::ArrayDecorationList{});  // Doubly nested array

  ast::type::Array rtarr_mat4x4(ty.mat4x4<f32>(), 0,
                                ast::ArrayDecorationList{});  // Runtime array

  auto* s = create<ast::Struct>(
      ast::StructMemberList{Member("a", &arr_mat2x2, {MemberOffset(0)}),
                            Member("b", &arr_arr_mat2x3, {MemberOffset(16)}),
                            Member("c", &rtarr_mat4x4, {MemberOffset(48)})},
      ast::StructDecorationList{});
  ast::type::Struct s_type(mod->RegisterSymbol("S"), "S", s);

  auto id = b.GenerateTypeIfNeeded(&s_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 2
%3 = OpTypeMatrix %4 2
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%2 = OpTypeArray %3 %7
%10 = OpTypeVector %5 3
%9 = OpTypeMatrix %10 2
%8 = OpTypeArray %9 %7
%13 = OpTypeVector %5 4
%12 = OpTypeMatrix %13 4
%11 = OpTypeRuntimeArray %12
%1 = OpTypeStruct %2 %8 %11
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "S"
OpMemberName %1 0 "a"
OpMemberName %1 1 "b"
OpMemberName %1 2 "c"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %1 0 Offset 0
OpMemberDecorate %1 0 ColMajor
OpMemberDecorate %1 0 MatrixStride 8
OpMemberDecorate %1 1 Offset 16
OpMemberDecorate %1 1 ColMajor
OpMemberDecorate %1 1 MatrixStride 16
OpMemberDecorate %1 2 Offset 48
OpMemberDecorate %1 2 ColMajor
OpMemberDecorate %1 2 MatrixStride 16
)");
}

TEST_F(BuilderTest_Type, GenerateU32) {
  ast::type::U32 u32;

  auto id = b.GenerateTypeIfNeeded(&u32);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeInt 32 0
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedU32) {
  ast::type::U32 u32;
  ast::type::F32 f32;

  EXPECT_EQ(b.GenerateTypeIfNeeded(&u32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&f32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&u32), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateVector) {
  ast::type::F32 f32;
  ast::type::Vector vec_type(&f32, 3);

  auto id = b.GenerateTypeIfNeeded(&vec_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  EXPECT_EQ(b.types().size(), 2u);
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedVector) {
  ast::type::I32 i32;
  ast::type::Vector vec_type(&i32, 3);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&vec_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&vec_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest_Type, GenerateVoid) {
  ast::type::Void void_type;

  auto id = b.GenerateTypeIfNeeded(&void_type);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(id, 1u);

  ASSERT_EQ(b.types().size(), 1u);
  EXPECT_EQ(DumpInstruction(b.types()[0]), R"(%1 = OpTypeVoid
)");
}

TEST_F(BuilderTest_Type, ReturnsGeneratedVoid) {
  ast::type::I32 i32;
  ast::type::Void void_type;

  EXPECT_EQ(b.GenerateTypeIfNeeded(&void_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&i32), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&void_type), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

struct PtrData {
  ast::StorageClass ast_class;
  SpvStorageClass result;
};
inline std::ostream& operator<<(std::ostream& out, PtrData data) {
  out << data.ast_class;
  return out;
}
using PtrDataTest = TestParamHelper<PtrData>;
TEST_P(PtrDataTest, ConvertStorageClass) {
  auto params = GetParam();

  EXPECT_EQ(b.ConvertStorageClass(params.ast_class), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest_Type,
    PtrDataTest,
    testing::Values(
        PtrData{ast::StorageClass::kNone, SpvStorageClassMax},
        PtrData{ast::StorageClass::kInput, SpvStorageClassInput},
        PtrData{ast::StorageClass::kOutput, SpvStorageClassOutput},
        PtrData{ast::StorageClass::kUniform, SpvStorageClassUniform},
        PtrData{ast::StorageClass::kWorkgroup, SpvStorageClassWorkgroup},
        PtrData{ast::StorageClass::kUniformConstant,
                SpvStorageClassUniformConstant},
        PtrData{ast::StorageClass::kStorageBuffer,
                SpvStorageClassStorageBuffer},
        PtrData{ast::StorageClass::kImage, SpvStorageClassImage},
        PtrData{ast::StorageClass::kPrivate, SpvStorageClassPrivate},
        PtrData{ast::StorageClass::kFunction, SpvStorageClassFunction}));

TEST_F(BuilderTest_Type, DepthTexture_Generate_2d) {
  ast::type::DepthTexture two_d(ast::type::TextureDimension::k2d);

  auto id_two_d = b.GenerateTypeIfNeeded(&two_d);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_two_d);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 1 0 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, DepthTexture_Generate_2dArray) {
  ast::type::DepthTexture two_d_array(ast::type::TextureDimension::k2dArray);

  auto id_two_d_array = b.GenerateTypeIfNeeded(&two_d_array);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_two_d_array);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 1 1 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, DepthTexture_Generate_Cube) {
  ast::type::DepthTexture cube(ast::type::TextureDimension::kCube);

  auto id_cube = b.GenerateTypeIfNeeded(&cube);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_cube);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 1 0 0 1 Unknown
)");
  EXPECT_EQ(DumpInstructions(b.capabilities()), "");
}

TEST_F(BuilderTest_Type, DepthTexture_Generate_CubeArray) {
  ast::type::DepthTexture cube_array(ast::type::TextureDimension::kCubeArray);

  auto id_cube_array = b.GenerateTypeIfNeeded(&cube_array);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(1u, id_cube_array);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 1 1 0 1 Unknown
)");
  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability SampledCubeArray
)");
}

TEST_F(BuilderTest_Type, MultisampledTexture_Generate_2d_i32) {
  ast::type::I32 i32;
  ast::type::MultisampledTexture ms(ast::type::TextureDimension::k2d, &i32);

  EXPECT_EQ(1u, b.GenerateTypeIfNeeded(&ms));
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeImage %2 2D 0 0 1 1 Unknown
)");
}

TEST_F(BuilderTest_Type, MultisampledTexture_Generate_2d_u32) {
  ast::type::U32 u32;
  ast::type::MultisampledTexture ms(ast::type::TextureDimension::k2d, &u32);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&ms), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeInt 32 0
%1 = OpTypeImage %2 2D 0 0 1 1 Unknown
)");
}

TEST_F(BuilderTest_Type, MultisampledTexture_Generate_2d_f32) {
  ast::type::F32 f32;
  ast::type::MultisampledTexture ms(ast::type::TextureDimension::k2d, &f32);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&ms), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 1 1 Unknown
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_1d_i32) {
  ast::type::I32 i32;
  ast::type::SampledTexture s(ast::type::TextureDimension::k1d, &i32);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeInt 32 1
%1 = OpTypeImage %2 1D 0 0 0 1 Unknown
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Sampled1D
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_1d_u32) {
  ast::type::U32 u32;
  ast::type::SampledTexture s(ast::type::TextureDimension::k1d, &u32);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeInt 32 0
%1 = OpTypeImage %2 1D 0 0 0 1 Unknown
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Sampled1D
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_1d_f32) {
  ast::type::F32 f32;
  ast::type::SampledTexture s(ast::type::TextureDimension::k1d, &f32);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 1D 0 0 0 1 Unknown
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Sampled1D
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_1dArray) {
  ast::type::F32 f32;
  ast::type::SampledTexture s(ast::type::TextureDimension::k1dArray, &f32);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 1D 0 1 0 1 Unknown
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Sampled1D
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_2d) {
  ast::type::F32 f32;
  ast::type::SampledTexture s(ast::type::TextureDimension::k2d, &f32);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_2d_array) {
  ast::type::F32 f32;
  ast::type::SampledTexture s(ast::type::TextureDimension::k2dArray, &f32);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 1 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_3d) {
  ast::type::F32 f32;
  ast::type::SampledTexture s(ast::type::TextureDimension::k3d, &f32);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 3D 0 0 0 1 Unknown
)");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_Cube) {
  ast::type::F32 f32;
  ast::type::SampledTexture s(ast::type::TextureDimension::kCube, &f32);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 0 0 0 1 Unknown
)");
  EXPECT_EQ(DumpInstructions(b.capabilities()), "");
}

TEST_F(BuilderTest_Type, SampledTexture_Generate_CubeArray) {
  ast::type::F32 f32;
  ast::type::SampledTexture s(ast::type::TextureDimension::kCubeArray, &f32);

  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 Cube 0 1 0 1 Unknown
)");
  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability SampledCubeArray
)");
}

TEST_F(BuilderTest_Type, StorageTexture_GenerateReadonly_1d_R16Float) {
  ast::type::StorageTexture s(ast::type::TextureDimension::k1d,
                              ast::AccessControl::kReadOnly,
                              ast::type::ImageFormat::kR16Float);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 1D 0 0 0 2 R16f
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Image1D
OpCapability StorageImageExtendedFormats
)");
}

TEST_F(BuilderTest_Type, StorageTexture_GenerateReadonly_1d_R8SNorm) {
  ast::type::StorageTexture s(ast::type::TextureDimension::k1d,
                              ast::AccessControl::kReadOnly,
                              ast::type::ImageFormat::kR8Snorm);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 1D 0 0 0 2 R8Snorm
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Image1D
OpCapability StorageImageExtendedFormats
)");
}

TEST_F(BuilderTest_Type, StorageTexture_GenerateReadonly_1d_R8UNorm) {
  ast::type::StorageTexture s(ast::type::TextureDimension::k1d,
                              ast::AccessControl::kReadOnly,
                              ast::type::ImageFormat::kR8Unorm);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 1D 0 0 0 2 R8
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Image1D
OpCapability StorageImageExtendedFormats
)");
}

TEST_F(BuilderTest_Type, StorageTexture_GenerateReadonly_1d_R8Uint) {
  ast::type::StorageTexture s(ast::type::TextureDimension::k1d,
                              ast::AccessControl::kReadOnly,
                              ast::type::ImageFormat::kR8Uint);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%1 = OpTypeImage %2 1D 0 0 0 2 R8ui
)");
}

TEST_F(BuilderTest_Type, StorageTexture_GenerateReadonly_1d_R8Sint) {
  ast::type::StorageTexture s(ast::type::TextureDimension::k1d,
                              ast::AccessControl::kReadOnly,
                              ast::type::ImageFormat::kR8Sint);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeImage %2 1D 0 0 0 2 R8i
)");
}

TEST_F(BuilderTest_Type, StorageTexture_GenerateReadonly_1d_array) {
  ast::type::StorageTexture s(ast::type::TextureDimension::k1dArray,
                              ast::AccessControl::kReadOnly,
                              ast::type::ImageFormat::kR16Float);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 1D 0 1 0 2 R16f
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Image1D
OpCapability StorageImageExtendedFormats
)");
}

TEST_F(BuilderTest_Type, StorageTexture_GenerateReadonly_2d) {
  ast::type::StorageTexture s(ast::type::TextureDimension::k2d,
                              ast::AccessControl::kReadOnly,
                              ast::type::ImageFormat::kR16Float);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 0 0 2 R16f
)");
}

TEST_F(BuilderTest_Type, StorageTexture_GenerateReadonly_2dArray) {
  ast::type::StorageTexture s(ast::type::TextureDimension::k2dArray,
                              ast::AccessControl::kReadOnly,
                              ast::type::ImageFormat::kR16Float);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 2D 0 1 0 2 R16f
)");
}

TEST_F(BuilderTest_Type, StorageTexture_GenerateReadonly_3d) {
  ast::type::StorageTexture s(ast::type::TextureDimension::k3d,
                              ast::AccessControl::kReadOnly,
                              ast::type::ImageFormat::kR16Float);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeImage %2 3D 0 0 0 2 R16f
)");
}

TEST_F(BuilderTest_Type, StorageTexture_GenerateWriteonly_1d) {
  ast::type::StorageTexture s(ast::type::TextureDimension::k1d,
                              ast::AccessControl::kWriteOnly,
                              ast::type::ImageFormat::kR16Float);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeImage %2 1D 0 0 0 2 R16f
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Image1D
OpCapability StorageImageExtendedFormats
)");
}

TEST_F(BuilderTest_Type, StorageTexture_GenerateWriteonly_1dArray) {
  ast::type::StorageTexture s(ast::type::TextureDimension::k1dArray,
                              ast::AccessControl::kWriteOnly,
                              ast::type::ImageFormat::kR16Float);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeImage %2 1D 0 1 0 2 R16f
)");

  EXPECT_EQ(DumpInstructions(b.capabilities()),
            R"(OpCapability Image1D
OpCapability StorageImageExtendedFormats
)");
}

TEST_F(BuilderTest_Type, StorageTexture_GenerateWriteonly_2d) {
  ast::type::StorageTexture s(ast::type::TextureDimension::k2d,
                              ast::AccessControl::kWriteOnly,
                              ast::type::ImageFormat::kR16Float);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeImage %2 2D 0 0 0 2 R16f
)");
}

TEST_F(BuilderTest_Type, StorageTexture_GenerateWriteonly_2dArray) {
  ast::type::StorageTexture s(ast::type::TextureDimension::k2dArray,
                              ast::AccessControl::kWriteOnly,
                              ast::type::ImageFormat::kR16Float);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeImage %2 2D 0 1 0 2 R16f
)");
}

TEST_F(BuilderTest_Type, StorageTexture_GenerateWriteonly_3d) {
  ast::type::StorageTexture s(ast::type::TextureDimension::k3d,
                              ast::AccessControl::kWriteOnly,
                              ast::type::ImageFormat::kR16Float);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();
  EXPECT_EQ(b.GenerateTypeIfNeeded(&s), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeImage %2 3D 0 0 0 2 R16f
)");
}

TEST_F(BuilderTest_Type, Sampler) {
  ast::type::Sampler sampler(ast::type::SamplerKind::kSampler);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&sampler), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), "%1 = OpTypeSampler\n");
}

TEST_F(BuilderTest_Type, ComparisonSampler) {
  ast::type::Sampler sampler(ast::type::SamplerKind::kComparisonSampler);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&sampler), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), "%1 = OpTypeSampler\n");
}

TEST_F(BuilderTest_Type, Dedup_Sampler_And_ComparisonSampler) {
  ast::type::Sampler comp_sampler(ast::type::SamplerKind::kComparisonSampler);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&comp_sampler), 1u);

  ast::type::Sampler sampler(ast::type::SamplerKind::kSampler);
  EXPECT_EQ(b.GenerateTypeIfNeeded(&sampler), 1u);

  ASSERT_FALSE(b.has_error()) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), "%1 = OpTypeSampler\n");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
