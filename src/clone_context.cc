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

#include "src/clone_context.h"

#include "src/program.h"

namespace tint {

CloneContext::CloneContext(Program* to, Program const* from)
    : dst(to), src(from) {}
CloneContext::~CloneContext() = default;

Symbol CloneContext::Clone(const Symbol& s) const {
  return dst->RegisterSymbol(src->SymbolToName(s));
}

void CloneContext::Clone() {
  src->Clone(this);
}

}  // namespace tint
