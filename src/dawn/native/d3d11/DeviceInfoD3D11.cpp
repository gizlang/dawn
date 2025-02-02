// Copyright 2023 The Dawn Authors
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

#include "dawn/native/d3d11/DeviceInfoD3D11.h"

#include <utility>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/AdapterD3D11.h"
#include "dawn/native/d3d11/PlatformFunctionsD3D11.h"

namespace dawn::native::d3d11 {

ResultOrError<DeviceInfo> GatherDeviceInfo(const Adapter& adapter) {
    DeviceInfo info = {};

    D3D11_FEATURE_DATA_D3D11_OPTIONS2 options2;
    DAWN_TRY(CheckHRESULT(adapter.GetD3D11Device()->CheckFeatureSupport(
                              D3D11_FEATURE_D3D11_OPTIONS2, &options2, sizeof(options2)),
                          "D3D11_FEATURE_D3D11_OPTIONS2"));

    info.isUMA = options2.UnifiedMemoryArchitecture;

    info.shaderModel = 50;
    // Profiles are always <stage>s_<minor>_<major> so we build the s_<minor>_major and add
    // it to each of the stage's suffix.
    info.shaderProfiles[SingleShaderStage::Vertex] = L"vs_5_0";
    info.shaderProfiles[SingleShaderStage::Fragment] = L"ps_5_0";
    info.shaderProfiles[SingleShaderStage::Compute] = L"cs_5_0";

    return std::move(info);
}

}  // namespace dawn::native::d3d11
