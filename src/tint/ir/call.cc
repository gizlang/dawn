// Copyright 2023 The Tint Authors.
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

#include "src/tint/ir/call.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Call);

namespace tint::ir {

Call::Call(Value* result, utils::VectorRef<Value*> args) : Base(result), args_(args) {
    for (auto* arg : args) {
        arg->AddUsage(this);
    }
}

Call::~Call() = default;

void Call::EmitArgs(utils::StringStream& out, const SymbolTable& st) const {
    bool first = true;
    for (const auto* arg : args_) {
        if (!first) {
            out << ", ";
        }
        first = false;
        arg->ToString(out, st);
    }
}

}  // namespace tint::ir
