// Copyright 2021 The Tint Authors.
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

#include "src/tint/transform/single_entry_point.h"

#include <unordered_set>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::SingleEntryPoint);
TINT_INSTANTIATE_TYPEINFO(tint::transform::SingleEntryPoint::Config);

namespace tint::transform {

SingleEntryPoint::SingleEntryPoint() = default;

SingleEntryPoint::~SingleEntryPoint() = default;

void SingleEntryPoint::Run(CloneContext& ctx, const DataMap& inputs, DataMap&) const {
    auto* cfg = inputs.Get<Config>();
    if (cfg == nullptr) {
        ctx.dst->Diagnostics().add_error(
            diag::System::Transform, "missing transform data for " + std::string(TypeInfo().name));

        return;
    }

    // Find the target entry point.
    const ast::Function* entry_point = nullptr;
    for (auto* f : ctx.src->AST().Functions()) {
        if (!f->IsEntryPoint()) {
            continue;
        }
        if (ctx.src->Symbols().NameFor(f->symbol) == cfg->entry_point_name) {
            entry_point = f;
            break;
        }
    }
    if (entry_point == nullptr) {
        ctx.dst->Diagnostics().add_error(diag::System::Transform,
                                         "entry point '" + cfg->entry_point_name + "' not found");
        return;
    }

    auto& sem = ctx.src->Sem();

    // Build set of referenced module-scope variables for faster lookups later.
    std::unordered_set<const ast::Variable*> referenced_vars;
    for (auto* var : sem.Get(entry_point)->TransitivelyReferencedGlobals()) {
        referenced_vars.emplace(var->Declaration());
    }

    // Clone any module-scope variables, types, and functions that are statically referenced by the
    // target entry point.
    for (auto* decl : ctx.src->AST().GlobalDeclarations()) {
        Switch(
            decl,  //
            [&](const ast::TypeDecl* ty) {
                // TODO(jrprice): Strip unused types.
                ctx.dst->AST().AddTypeDecl(ctx.Clone(ty));
            },
            [&](const ast::Override* override) {
                if (referenced_vars.count(override)) {
                    if (!ast::HasAttribute<ast::IdAttribute>(override->attributes)) {
                        // If the override doesn't already have an @id() attribute, add one
                        // so that its allocated ID so that it won't be affected by other
                        // stripped away overrides
                        auto* global = sem.Get(override);
                        const auto* id = ctx.dst->Id(global->OverrideId());
                        ctx.InsertFront(override->attributes, id);
                    }
                    ctx.dst->AST().AddGlobalVariable(ctx.Clone(override));
                }
            },
            [&](const ast::Variable* v) {  // var, let
                if (referenced_vars.count(v)) {
                    ctx.dst->AST().AddGlobalVariable(ctx.Clone(v));
                }
            },
            [&](const ast::Function* func) {
                if (sem.Get(func)->HasAncestorEntryPoint(entry_point->symbol)) {
                    ctx.dst->AST().AddFunction(ctx.Clone(func));
                }
            },
            [&](const ast::Enable* ext) { ctx.dst->AST().AddEnable(ctx.Clone(ext)); },
            [&](Default) {
                TINT_UNREACHABLE(Transform, ctx.dst->Diagnostics())
                    << "unhandled global declaration: " << decl->TypeInfo().name;
            });
    }

    // Clone the entry point.
    ctx.dst->AST().AddFunction(ctx.Clone(entry_point));
}

SingleEntryPoint::Config::Config(std::string entry_point) : entry_point_name(entry_point) {}

SingleEntryPoint::Config::Config(const Config&) = default;
SingleEntryPoint::Config::~Config() = default;
SingleEntryPoint::Config& SingleEntryPoint::Config::operator=(const Config&) = default;

}  // namespace tint::transform
