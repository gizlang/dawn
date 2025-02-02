// Copyright 2017 The Dawn Authors
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

#include "dawn/native/SwapChain.h"

#include "dawn/common/Constants.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/Surface.h"
#include "dawn/native/Texture.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

namespace {

class ErrorSwapChain final : public SwapChainBase {
  public:
    explicit ErrorSwapChain(DeviceBase* device) : SwapChainBase(device, ObjectBase::kError) {}

  private:
    void APIConfigure(wgpu::TextureFormat format,
                      wgpu::TextureUsage allowedUsage,
                      uint32_t width,
                      uint32_t height) override {
        GetDevice()->HandleError(DAWN_VALIDATION_ERROR("%s is an error swapchain.", this));
    }

    TextureViewBase* APIGetCurrentTextureView() override {
        GetDevice()->HandleError(DAWN_VALIDATION_ERROR("%s is an error swapchain.", this));
        return TextureViewBase::MakeError(GetDevice());
    }

    void APIPresent() override {
        GetDevice()->HandleError(DAWN_VALIDATION_ERROR("%s is an error swapchain.", this));
    }
};

}  // anonymous namespace

MaybeError ValidateSwapChainDescriptor(const DeviceBase* device,
                                       const Surface* surface,
                                       const SwapChainDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->implementation != 0,
                    "Implementation-based swapchains are no longer supported.");

    DAWN_INVALID_IF(surface == nullptr, "At least one of surface or implementation must be set");
    DAWN_INVALID_IF(surface->IsError(), "[Surface] is invalid.");

    DAWN_TRY(ValidatePresentMode(descriptor->presentMode));

// TODO(crbug.com/dawn/160): Lift this restriction once wgpu::Instance::GetPreferredSurfaceFormat is
// implemented.
// TODO(dawn:286):
#if DAWN_PLATFORM_IS(ANDROID)
    constexpr wgpu::TextureFormat kRequireSwapChainFormat = wgpu::TextureFormat::RGBA8Unorm;
#else
    constexpr wgpu::TextureFormat kRequireSwapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
#endif  // !DAWN_PLATFORM_IS(ANDROID)
    DAWN_INVALID_IF(descriptor->format != kRequireSwapChainFormat,
                    "Format (%s) is not %s, which is (currently) the only accepted format.",
                    descriptor->format, kRequireSwapChainFormat);

    DAWN_INVALID_IF(descriptor->usage != wgpu::TextureUsage::RenderAttachment,
                    "Usage (%s) is not %s, which is (currently) the only accepted usage.",
                    descriptor->usage, wgpu::TextureUsage::RenderAttachment);

    DAWN_INVALID_IF(descriptor->width == 0 || descriptor->height == 0,
                    "Swap Chain size (width: %u, height: %u) is empty.", descriptor->width,
                    descriptor->height);

    DAWN_INVALID_IF(
        descriptor->width > device->GetLimits().v1.maxTextureDimension2D ||
            descriptor->height > device->GetLimits().v1.maxTextureDimension2D,
        "Swap Chain size (width: %u, height: %u) is greater than the maximum 2D texture "
        "size (width: %u, height: %u).",
        descriptor->width, descriptor->height, device->GetLimits().v1.maxTextureDimension2D,
        device->GetLimits().v1.maxTextureDimension2D);

    return {};
}

TextureDescriptor GetSwapChainBaseTextureDescriptor(NewSwapChainBase* swapChain) {
    TextureDescriptor desc;
    desc.usage = swapChain->GetUsage();
    desc.dimension = wgpu::TextureDimension::e2D;
    desc.size = {swapChain->GetWidth(), swapChain->GetHeight(), 1};
    desc.format = swapChain->GetFormat();
    desc.mipLevelCount = 1;
    desc.sampleCount = 1;

    return desc;
}

// SwapChainBase

SwapChainBase::SwapChainBase(DeviceBase* device) : ApiObjectBase(device, kLabelNotImplemented) {
    GetObjectTrackingList()->Track(this);
}

SwapChainBase::SwapChainBase(DeviceBase* device, ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag) {}

SwapChainBase::~SwapChainBase() {}

void SwapChainBase::DestroyImpl() {}

// static
SwapChainBase* SwapChainBase::MakeError(DeviceBase* device) {
    return new ErrorSwapChain(device);
}

ObjectType SwapChainBase::GetType() const {
    return ObjectType::SwapChain;
}

// Implementation of NewSwapChainBase

NewSwapChainBase::NewSwapChainBase(DeviceBase* device,
                                   Surface* surface,
                                   const SwapChainDescriptor* descriptor)
    : SwapChainBase(device),
      mAttached(false),
      mWidth(descriptor->width),
      mHeight(descriptor->height),
      mFormat(descriptor->format),
      mUsage(descriptor->usage),
      mPresentMode(descriptor->presentMode),
      mSurface(surface) {}

NewSwapChainBase::~NewSwapChainBase() {
    if (mCurrentTextureView != nullptr) {
        ASSERT(mCurrentTextureView->GetTexture()->GetTextureState() ==
               TextureBase::TextureState::Destroyed);
    }

    ASSERT(!mAttached);
}

void NewSwapChainBase::DetachFromSurface() {
    if (mAttached) {
        DetachFromSurfaceImpl();
        mSurface = nullptr;
        mAttached = false;
    }
}

void NewSwapChainBase::SetIsAttached() {
    mAttached = true;
}

void NewSwapChainBase::APIConfigure(wgpu::TextureFormat format,
                                    wgpu::TextureUsage allowedUsage,
                                    uint32_t width,
                                    uint32_t height) {
    GetDevice()->HandleError(
        DAWN_VALIDATION_ERROR("Configure is invalid for surface-based swapchains."));
}

TextureViewBase* NewSwapChainBase::APIGetCurrentTextureView() {
    Ref<TextureViewBase> result;
    if (GetDevice()->ConsumedError(GetCurrentTextureView(), &result,
                                   "calling %s.GetCurrentTextureView()", this)) {
        return TextureViewBase::MakeError(GetDevice());
    }
    return result.Detach();
}

ResultOrError<Ref<TextureViewBase>> NewSwapChainBase::GetCurrentTextureView() {
    DAWN_TRY(ValidateGetCurrentTextureView());

    if (mCurrentTextureView != nullptr) {
        // Calling GetCurrentTextureView always returns a new reference.
        return mCurrentTextureView;
    }

    DAWN_TRY_ASSIGN(mCurrentTextureView, GetCurrentTextureViewImpl());

    // Check that the return texture view matches exactly what was given for this descriptor.
    ASSERT(mCurrentTextureView->GetTexture()->GetFormat().format == mFormat);
    ASSERT(IsSubset(mUsage, mCurrentTextureView->GetTexture()->GetUsage()));
    ASSERT(mCurrentTextureView->GetLevelCount() == 1);
    ASSERT(mCurrentTextureView->GetLayerCount() == 1);
    ASSERT(mCurrentTextureView->GetDimension() == wgpu::TextureViewDimension::e2D);
    ASSERT(mCurrentTextureView->GetTexture()
               ->GetMipLevelSingleSubresourceVirtualSize(mCurrentTextureView->GetBaseMipLevel())
               .width == mWidth);
    ASSERT(mCurrentTextureView->GetTexture()
               ->GetMipLevelSingleSubresourceVirtualSize(mCurrentTextureView->GetBaseMipLevel())
               .height == mHeight);

    return mCurrentTextureView;
}

void NewSwapChainBase::APIPresent() {
    if (GetDevice()->ConsumedError(ValidatePresent())) {
        return;
    }

    if (GetDevice()->ConsumedError(PresentImpl())) {
        return;
    }

    ASSERT(mCurrentTextureView->GetTexture()->GetTextureState() ==
           TextureBase::TextureState::Destroyed);
    mCurrentTextureView = nullptr;
}

uint32_t NewSwapChainBase::GetWidth() const {
    return mWidth;
}

uint32_t NewSwapChainBase::GetHeight() const {
    return mHeight;
}

wgpu::TextureFormat NewSwapChainBase::GetFormat() const {
    return mFormat;
}

wgpu::TextureUsage NewSwapChainBase::GetUsage() const {
    return mUsage;
}

wgpu::PresentMode NewSwapChainBase::GetPresentMode() const {
    return mPresentMode;
}

Surface* NewSwapChainBase::GetSurface() const {
    return mSurface;
}

bool NewSwapChainBase::IsAttached() const {
    return mAttached;
}

wgpu::BackendType NewSwapChainBase::GetBackendType() const {
    return GetDevice()->GetAdapter()->GetBackendType();
}

MaybeError NewSwapChainBase::ValidatePresent() const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));

    DAWN_INVALID_IF(!mAttached, "Cannot call Present called on detached %s.", this);

    DAWN_INVALID_IF(
        mCurrentTextureView == nullptr,
        "GetCurrentTextureView was not called on %s this frame prior to calling Present.", this);

    return {};
}

MaybeError NewSwapChainBase::ValidateGetCurrentTextureView() const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));

    DAWN_INVALID_IF(!mAttached, "Cannot call GetCurrentTextureView on detached %s.", this);

    return {};
}

}  // namespace dawn::native
