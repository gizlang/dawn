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

#ifndef SRC_TINT_TRANSFORM_PAD_STRUCTS_H_
#define SRC_TINT_TRANSFORM_PAD_STRUCTS_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// This transform turns all explicit alignment and sizing into padding
/// members of structs. This is required for GLSL ES, since it not support
/// the offset= decoration.
///
/// @note This transform requires the CanonicalizeEntryPointIO transform to have been run first.
class PadStructs final : public Castable<PadStructs, Transform> {
  public:
    /// Constructor
    PadStructs();

    /// Destructor
    ~PadStructs() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_PAD_STRUCTS_H_
