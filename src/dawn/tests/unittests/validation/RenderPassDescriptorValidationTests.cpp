// Copyright 2018 The Dawn Authors
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

#include <cmath>
#include <vector>

#include "dawn/tests/unittests/validation/DeprecatedAPITests.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"

#include "dawn/common/Constants.h"

#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {

class RenderPassDescriptorValidationTest : public ValidationTest {
  public:
    void AssertBeginRenderPassSuccess(const wgpu::RenderPassDescriptor* descriptor) {
        wgpu::CommandEncoder commandEncoder = TestBeginRenderPass(descriptor);
        commandEncoder.Finish();
    }
    void AssertBeginRenderPassError(const wgpu::RenderPassDescriptor* descriptor) {
        wgpu::CommandEncoder commandEncoder = TestBeginRenderPass(descriptor);
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

  private:
    wgpu::CommandEncoder TestBeginRenderPass(const wgpu::RenderPassDescriptor* descriptor) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(descriptor);
        renderPassEncoder.End();
        return commandEncoder;
    }
};

wgpu::Texture CreateTexture(wgpu::Device& device,
                            wgpu::TextureDimension dimension,
                            wgpu::TextureFormat format,
                            uint32_t width,
                            uint32_t height,
                            uint32_t arrayLayerCount,
                            uint32_t mipLevelCount,
                            uint32_t sampleCount = 1,
                            wgpu::TextureUsage usage = wgpu::TextureUsage::RenderAttachment) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = dimension;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depthOrArrayLayers = arrayLayerCount;
    descriptor.sampleCount = sampleCount;
    descriptor.format = format;
    descriptor.mipLevelCount = mipLevelCount;
    descriptor.usage = usage;

    return device.CreateTexture(&descriptor);
}

wgpu::TextureView Create2DAttachment(wgpu::Device& device,
                                     uint32_t width,
                                     uint32_t height,
                                     wgpu::TextureFormat format) {
    wgpu::Texture texture =
        CreateTexture(device, wgpu::TextureDimension::e2D, format, width, height, 1, 1);
    return texture.CreateView();
}

// Using BeginRenderPass with no attachments isn't valid
TEST_F(RenderPassDescriptorValidationTest, Empty) {
    utils::ComboRenderPassDescriptor renderPass({}, nullptr);
    AssertBeginRenderPassError(&renderPass);
}

// A render pass with only one color or one depth attachment is ok
TEST_F(RenderPassDescriptorValidationTest, OneAttachment) {
    // One color attachment
    {
        wgpu::TextureView color = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
        utils::ComboRenderPassDescriptor renderPass({color});

        AssertBeginRenderPassSuccess(&renderPass);
    }
    // One depth-stencil attachment
    {
        wgpu::TextureView depthStencil =
            Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24PlusStencil8);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencil);

        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// Test OOB color attachment indices are handled
TEST_F(RenderPassDescriptorValidationTest, ColorAttachmentOutOfBounds) {
    std::array<wgpu::RenderPassColorAttachment, kMaxColorAttachments + 1> colorAttachments;
    for (uint32_t i = 0; i < colorAttachments.size(); i++) {
        colorAttachments[i].view = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::R8Unorm);
        colorAttachments[i].resolveTarget = nullptr;
        colorAttachments[i].clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
        colorAttachments[i].loadOp = wgpu::LoadOp::Clear;
        colorAttachments[i].storeOp = wgpu::StoreOp::Store;
    }

    // Control case: kMaxColorAttachments is valid.
    {
        wgpu::RenderPassDescriptor renderPass;
        renderPass.colorAttachmentCount = kMaxColorAttachments;
        renderPass.colorAttachments = colorAttachments.data();
        renderPass.depthStencilAttachment = nullptr;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Error case: kMaxColorAttachments + 1 is an error.
    {
        wgpu::RenderPassDescriptor renderPass;
        renderPass.colorAttachmentCount = kMaxColorAttachments + 1;
        renderPass.colorAttachments = colorAttachments.data();
        renderPass.depthStencilAttachment = nullptr;
        AssertBeginRenderPassError(&renderPass);
    }
}

// Test sparse color attachment validations
TEST_F(RenderPassDescriptorValidationTest, SparseColorAttachment) {
    // Having sparse color attachment is valid.
    {
        std::array<wgpu::RenderPassColorAttachment, 2> colorAttachments;
        colorAttachments[0].view = nullptr;

        colorAttachments[1].view =
            Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
        colorAttachments[1].loadOp = wgpu::LoadOp::Load;
        colorAttachments[1].storeOp = wgpu::StoreOp::Store;

        wgpu::RenderPassDescriptor renderPass;
        renderPass.colorAttachmentCount = colorAttachments.size();
        renderPass.colorAttachments = colorAttachments.data();
        renderPass.depthStencilAttachment = nullptr;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // When all color attachments are null
    {
        std::array<wgpu::RenderPassColorAttachment, 2> colorAttachments;
        colorAttachments[0].view = nullptr;
        colorAttachments[1].view = nullptr;

        // Control case: depth stencil attachment is not null is valid.
        {
            wgpu::TextureView depthStencilView =
                Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24PlusStencil8);
            wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
            depthStencilAttachment.view = depthStencilView;
            depthStencilAttachment.depthClearValue = 1.0f;
            depthStencilAttachment.stencilClearValue = 0;
            depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
            depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
            depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
            depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;

            wgpu::RenderPassDescriptor renderPass;
            renderPass.colorAttachmentCount = colorAttachments.size();
            renderPass.colorAttachments = colorAttachments.data();
            renderPass.depthStencilAttachment = &depthStencilAttachment;
            AssertBeginRenderPassSuccess(&renderPass);
        }

        // Error case: depth stencil attachment being null is invalid.
        {
            wgpu::RenderPassDescriptor renderPass;
            renderPass.colorAttachmentCount = colorAttachments.size();
            renderPass.colorAttachments = colorAttachments.data();
            renderPass.depthStencilAttachment = nullptr;
            AssertBeginRenderPassError(&renderPass);
        }
    }
}

// Check that the render pass color attachment must have the RenderAttachment usage.
TEST_F(RenderPassDescriptorValidationTest, ColorAttachmentInvalidUsage) {
    // Control case: using a texture with RenderAttachment is valid.
    {
        wgpu::TextureView renderView =
            Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
        utils::ComboRenderPassDescriptor renderPass({renderView});
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Error case: using a texture with Sampled is invalid.
    {
        wgpu::TextureDescriptor texDesc;
        texDesc.usage = wgpu::TextureUsage::TextureBinding;
        texDesc.size = {1, 1, 1};
        texDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        wgpu::Texture sampledTex = device.CreateTexture(&texDesc);

        utils::ComboRenderPassDescriptor renderPass({sampledTex.CreateView()});
        AssertBeginRenderPassError(&renderPass);
    }
}

// Attachments must have the same size
TEST_F(RenderPassDescriptorValidationTest, SizeMustMatch) {
    wgpu::TextureView color1x1A = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
    wgpu::TextureView color1x1B = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
    wgpu::TextureView color2x2 = Create2DAttachment(device, 2, 2, wgpu::TextureFormat::RGBA8Unorm);

    wgpu::TextureView depthStencil1x1 =
        Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24PlusStencil8);
    wgpu::TextureView depthStencil2x2 =
        Create2DAttachment(device, 2, 2, wgpu::TextureFormat::Depth24PlusStencil8);

    // Control case: all the same size (1x1)
    {
        utils::ComboRenderPassDescriptor renderPass({color1x1A, color1x1B}, depthStencil1x1);
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // One of the color attachments has a different size
    {
        utils::ComboRenderPassDescriptor renderPass({color1x1A, color2x2});
        AssertBeginRenderPassError(&renderPass);
    }

    // The depth stencil attachment has a different size
    {
        utils::ComboRenderPassDescriptor renderPass({color1x1A, color1x1B}, depthStencil2x2);
        AssertBeginRenderPassError(&renderPass);
    }
}

// Attachments formats must match whether they are used for color or depth-stencil
TEST_F(RenderPassDescriptorValidationTest, FormatMismatch) {
    wgpu::TextureView color = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
    wgpu::TextureView depthStencil =
        Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24PlusStencil8);

    // Using depth-stencil for color
    {
        utils::ComboRenderPassDescriptor renderPass({depthStencil});
        AssertBeginRenderPassError(&renderPass);
    }

    // Using color for depth-stencil
    {
        utils::ComboRenderPassDescriptor renderPass({}, color);
        AssertBeginRenderPassError(&renderPass);
    }
}

// Depth and stencil storeOps can be different
TEST_F(RenderPassDescriptorValidationTest, DepthStencilStoreOpMismatch) {
    constexpr uint32_t kArrayLayers = 1;
    constexpr uint32_t kLevelCount = 1;
    constexpr uint32_t kSize = 32;
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr wgpu::TextureFormat kDepthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;

    wgpu::Texture colorTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat,
                                               kSize, kSize, kArrayLayers, kLevelCount);
    wgpu::Texture depthStencilTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kDepthStencilFormat, kSize, kSize,
                      kArrayLayers, kLevelCount);

    wgpu::TextureViewDescriptor descriptor;
    descriptor.dimension = wgpu::TextureViewDimension::e2D;
    descriptor.baseArrayLayer = 0;
    descriptor.arrayLayerCount = kArrayLayers;
    descriptor.baseMipLevel = 0;
    descriptor.mipLevelCount = kLevelCount;
    wgpu::TextureView colorTextureView = colorTexture.CreateView(&descriptor);
    wgpu::TextureView depthStencilView = depthStencilTexture.CreateView(&descriptor);

    // Base case: StoreOps match so render pass is a success
    {
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Store;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Base case: StoreOps match so render pass is a success
    {
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Discard;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Discard;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // StoreOps mismatch still is a success
    {
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Discard;
        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// Currently only texture views with arrayLayerCount == 1 are allowed to be color and depth
// stencil attachments
TEST_F(RenderPassDescriptorValidationTest, TextureViewLayerCountForColorAndDepthStencil) {
    constexpr uint32_t kLevelCount = 1;
    constexpr uint32_t kSize = 32;
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr wgpu::TextureFormat kDepthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;

    constexpr uint32_t kArrayLayers = 10;

    wgpu::Texture colorTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat,
                                               kSize, kSize, kArrayLayers, kLevelCount);
    wgpu::Texture depthStencilTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kDepthStencilFormat, kSize, kSize,
                      kArrayLayers, kLevelCount);

    wgpu::TextureViewDescriptor baseDescriptor;
    baseDescriptor.dimension = wgpu::TextureViewDimension::e2DArray;
    baseDescriptor.baseArrayLayer = 0;
    baseDescriptor.arrayLayerCount = kArrayLayers;
    baseDescriptor.baseMipLevel = 0;
    baseDescriptor.mipLevelCount = kLevelCount;

    // Using 2D array texture view with arrayLayerCount > 1 is not allowed for color
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.arrayLayerCount = 5;

        wgpu::TextureView colorTextureView = colorTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({colorTextureView});
        AssertBeginRenderPassError(&renderPass);
    }

    // Using 2D array texture view with arrayLayerCount > 1 is not allowed for depth stencil
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.arrayLayerCount = 5;

        wgpu::TextureView depthStencilView = depthStencilTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        AssertBeginRenderPassError(&renderPass);
    }

    // Using 2D array texture view that covers the first layer of the texture is OK for color
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.baseArrayLayer = 0;
        descriptor.arrayLayerCount = 1;

        wgpu::TextureView colorTextureView = colorTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({colorTextureView});
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using 2D array texture view that covers the first layer is OK for depth stencil
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.baseArrayLayer = 0;
        descriptor.arrayLayerCount = 1;

        wgpu::TextureView depthStencilView = depthStencilTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using 2D array texture view that covers the last layer is OK for color
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.baseArrayLayer = kArrayLayers - 1;
        descriptor.arrayLayerCount = 1;

        wgpu::TextureView colorTextureView = colorTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({colorTextureView});
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using 2D array texture view that covers the last layer is OK for depth stencil
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.baseArrayLayer = kArrayLayers - 1;
        descriptor.arrayLayerCount = 1;

        wgpu::TextureView depthStencilView = depthStencilTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// Check that the render pass depth attachment must have the RenderAttachment usage.
TEST_F(RenderPassDescriptorValidationTest, DepthAttachmentInvalidUsage) {
    // Control case: using a texture with RenderAttachment is valid.
    {
        wgpu::TextureView renderView =
            Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth32Float);
        utils::ComboRenderPassDescriptor renderPass({}, renderView);
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Error case: using a texture with Sampled is invalid.
    {
        wgpu::TextureDescriptor texDesc;
        texDesc.usage = wgpu::TextureUsage::TextureBinding;
        texDesc.size = {1, 1, 1};
        texDesc.format = wgpu::TextureFormat::Depth32Float;
        wgpu::Texture sampledTex = device.CreateTexture(&texDesc);
        wgpu::TextureView sampledView = sampledTex.CreateView();

        utils::ComboRenderPassDescriptor renderPass({}, sampledView);
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassError(&renderPass);
    }
}

// Only 2D texture views with mipLevelCount == 1 are allowed to be color attachments
TEST_F(RenderPassDescriptorValidationTest, TextureViewLevelCountForColorAndDepthStencil) {
    constexpr uint32_t kArrayLayers = 1;
    constexpr uint32_t kSize = 32;
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr wgpu::TextureFormat kDepthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;

    constexpr uint32_t kLevelCount = 4;

    wgpu::Texture colorTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat,
                                               kSize, kSize, kArrayLayers, kLevelCount);
    wgpu::Texture depthStencilTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kDepthStencilFormat, kSize, kSize,
                      kArrayLayers, kLevelCount);

    wgpu::TextureViewDescriptor baseDescriptor;
    baseDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    baseDescriptor.baseArrayLayer = 0;
    baseDescriptor.arrayLayerCount = kArrayLayers;
    baseDescriptor.baseMipLevel = 0;
    baseDescriptor.mipLevelCount = kLevelCount;

    // Using 2D texture view with mipLevelCount > 1 is not allowed for color
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.mipLevelCount = 2;

        wgpu::TextureView colorTextureView = colorTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({colorTextureView});
        AssertBeginRenderPassError(&renderPass);
    }

    // Using 2D texture view with mipLevelCount > 1 is not allowed for depth stencil
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.mipLevelCount = 2;

        wgpu::TextureView depthStencilView = depthStencilTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        AssertBeginRenderPassError(&renderPass);
    }

    // Using 2D texture view that covers the first level of the texture is OK for color
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.baseMipLevel = 0;
        descriptor.mipLevelCount = 1;

        wgpu::TextureView colorTextureView = colorTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({colorTextureView});
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using 2D texture view that covers the first level is OK for depth stencil
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.baseMipLevel = 0;
        descriptor.mipLevelCount = 1;

        wgpu::TextureView depthStencilView = depthStencilTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using 2D texture view that covers the last level is OK for color
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.baseMipLevel = kLevelCount - 1;
        descriptor.mipLevelCount = 1;

        wgpu::TextureView colorTextureView = colorTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({colorTextureView});
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using 2D texture view that covers the last level is OK for depth stencil
    {
        wgpu::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.baseMipLevel = kLevelCount - 1;
        descriptor.mipLevelCount = 1;

        wgpu::TextureView depthStencilView = depthStencilTexture.CreateView(&descriptor);
        utils::ComboRenderPassDescriptor renderPass({}, depthStencilView);
        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// It is not allowed to set resolve target when the color attachment is non-multisampled.
TEST_F(RenderPassDescriptorValidationTest, NonMultisampledColorWithResolveTarget) {
    static constexpr uint32_t kArrayLayers = 1;
    static constexpr uint32_t kLevelCount = 1;
    static constexpr uint32_t kSize = 32;
    static constexpr uint32_t kSampleCount = 1;
    static constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::Texture colorTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
                      kLevelCount, kSampleCount);
    wgpu::Texture resolveTargetTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
                      kLevelCount, kSampleCount);
    wgpu::TextureView colorTextureView = colorTexture.CreateView();
    wgpu::TextureView resolveTargetTextureView = resolveTargetTexture.CreateView();

    utils::ComboRenderPassDescriptor renderPass({colorTextureView});
    renderPass.cColorAttachments[0].resolveTarget = resolveTargetTextureView;
    AssertBeginRenderPassError(&renderPass);
}

// drawCount must not exceed maxDrawCount
TEST_F(RenderPassDescriptorValidationTest, MaxDrawCount) {
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr uint64_t kMaxDrawCount = 16;

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    wgpu::TextureDescriptor colorTextureDescriptor;
    colorTextureDescriptor.size = {1, 1};
    colorTextureDescriptor.format = kColorFormat;
    colorTextureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture colorTexture = device.CreateTexture(&colorTextureDescriptor);

    utils::ComboRenderBundleEncoderDescriptor bundleEncoderDescriptor;
    bundleEncoderDescriptor.colorFormatsCount = 1;
    bundleEncoderDescriptor.cColorFormats[0] = kColorFormat;

    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {0, 1, 2});
    wgpu::Buffer indirectBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, {3, 1, 0, 0});
    wgpu::Buffer indexedIndirectBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, {3, 1, 0, 0, 0});

    wgpu::RenderPassDescriptorMaxDrawCount maxDrawCount;
    maxDrawCount.maxDrawCount = kMaxDrawCount;

    // Valid. drawCount is less than the default maxDrawCount.

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.Draw(3);
        }

        renderPass.End();
        encoder.Finish();
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);
        renderPass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.DrawIndexed(3);
        }

        renderPass.End();
        encoder.Finish();
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.DrawIndirect(indirectBuffer, 0);
        }

        renderPass.End();
        encoder.Finish();
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);
        renderPass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.DrawIndexedIndirect(indexedIndirectBuffer, 0);
        }

        renderPass.End();
        encoder.Finish();
    }

    {
        wgpu::RenderBundleEncoder renderBundleEncoder =
            device.CreateRenderBundleEncoder(&bundleEncoderDescriptor);
        renderBundleEncoder.SetPipeline(pipeline);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderBundleEncoder.Draw(3);
        }

        wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.ExecuteBundles(1, &renderBundle);
        renderPass.End();
        encoder.Finish();
    }

    // Invalid. drawCount counts up with draw calls and
    // it is greater than maxDrawCount.

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        renderPassDescriptor.nextInChain = &maxDrawCount;
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.Draw(3);
        }

        renderPass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        renderPassDescriptor.nextInChain = &maxDrawCount;
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);
        renderPass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.DrawIndexed(3);
        }

        renderPass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        renderPassDescriptor.nextInChain = &maxDrawCount;
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.DrawIndirect(indirectBuffer, 0);
        }

        renderPass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        renderPassDescriptor.nextInChain = &maxDrawCount;
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(pipeline);
        renderPass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderPass.DrawIndexedIndirect(indexedIndirectBuffer, 0);
        }

        renderPass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::RenderBundleEncoder renderBundleEncoder =
            device.CreateRenderBundleEncoder(&bundleEncoderDescriptor);
        renderBundleEncoder.SetPipeline(pipeline);

        for (uint64_t i = 0; i <= kMaxDrawCount; i++) {
            renderBundleEncoder.Draw(3);
        }

        wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});
        renderPassDescriptor.nextInChain = &maxDrawCount;
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.ExecuteBundles(1, &renderBundle);
        renderPass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

class MultisampledRenderPassDescriptorValidationTest : public RenderPassDescriptorValidationTest {
  public:
    utils::ComboRenderPassDescriptor CreateMultisampledRenderPass() {
        return utils::ComboRenderPassDescriptor({CreateMultisampledColorTextureView()});
    }

    wgpu::TextureView CreateMultisampledColorTextureView() {
        return CreateColorTextureView(kSampleCount);
    }

    wgpu::TextureView CreateNonMultisampledColorTextureView() { return CreateColorTextureView(1); }

    static constexpr uint32_t kArrayLayers = 1;
    static constexpr uint32_t kLevelCount = 1;
    static constexpr uint32_t kSize = 32;
    static constexpr uint32_t kSampleCount = 4;
    static constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;

  private:
    wgpu::TextureView CreateColorTextureView(uint32_t sampleCount) {
        wgpu::Texture colorTexture =
            CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize,
                          kArrayLayers, kLevelCount, sampleCount);

        return colorTexture.CreateView();
    }
};

// Tests on the use of multisampled textures as color attachments
TEST_F(MultisampledRenderPassDescriptorValidationTest, MultisampledColorAttachments) {
    wgpu::TextureView colorTextureView = CreateNonMultisampledColorTextureView();
    wgpu::TextureView resolveTargetTextureView = CreateNonMultisampledColorTextureView();
    wgpu::TextureView multisampledColorTextureView = CreateMultisampledColorTextureView();

    // It is allowed to use a multisampled color attachment without setting resolve target.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // It is not allowed to use multiple color attachments with different sample counts.
    {
        utils::ComboRenderPassDescriptor renderPass(
            {multisampledColorTextureView, colorTextureView});
        AssertBeginRenderPassError(&renderPass);
    }
}

// It is not allowed to use a multisampled resolve target.
TEST_F(MultisampledRenderPassDescriptorValidationTest, MultisampledResolveTarget) {
    wgpu::TextureView multisampledResolveTargetView = CreateMultisampledColorTextureView();

    utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].resolveTarget = multisampledResolveTargetView;
    AssertBeginRenderPassError(&renderPass);
}

// It is not allowed to use a resolve target with array layer count > 1.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ResolveTargetArrayLayerMoreThanOne) {
    constexpr uint32_t kArrayLayers2 = 2;
    wgpu::Texture resolveTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat,
                                                 kSize, kSize, kArrayLayers2, kLevelCount);
    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.dimension = wgpu::TextureViewDimension::e2DArray;
    wgpu::TextureView resolveTextureView = resolveTexture.CreateView(&viewDesc);

    utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].resolveTarget = resolveTextureView;
    AssertBeginRenderPassError(&renderPass);
}

// It is not allowed to use a resolve target with mipmap level count > 1.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ResolveTargetMipmapLevelMoreThanOne) {
    constexpr uint32_t kLevelCount2 = 2;
    wgpu::Texture resolveTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat,
                                                 kSize, kSize, kArrayLayers, kLevelCount2);
    wgpu::TextureView resolveTextureView = resolveTexture.CreateView();

    utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].resolveTarget = resolveTextureView;
    AssertBeginRenderPassError(&renderPass);
}

// It is not allowed to use a resolve target which is created from a texture whose usage does
// not include wgpu::TextureUsage::RenderAttachment.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ResolveTargetUsageNoRenderAttachment) {
    constexpr wgpu::TextureUsage kUsage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
    wgpu::Texture nonColorUsageResolveTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers,
                      kLevelCount, 1, kUsage);
    wgpu::TextureView nonColorUsageResolveTextureView = nonColorUsageResolveTexture.CreateView();

    utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].resolveTarget = nonColorUsageResolveTextureView;
    AssertBeginRenderPassError(&renderPass);
}

// It is not allowed to use a resolve target which is in error state.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ResolveTargetInErrorState) {
    wgpu::Texture resolveTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat,
                                                 kSize, kSize, kArrayLayers, kLevelCount);
    wgpu::TextureViewDescriptor errorTextureView;
    errorTextureView.dimension = wgpu::TextureViewDimension::e2D;
    errorTextureView.format = kColorFormat;
    errorTextureView.baseArrayLayer = kArrayLayers + 1;
    ASSERT_DEVICE_ERROR(wgpu::TextureView errorResolveTarget =
                            resolveTexture.CreateView(&errorTextureView));

    utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].resolveTarget = errorResolveTarget;
    AssertBeginRenderPassError(&renderPass);
}

// It is allowed to use a multisampled color attachment and a non-multisampled resolve target.
TEST_F(MultisampledRenderPassDescriptorValidationTest, MultisampledColorWithResolveTarget) {
    wgpu::TextureView resolveTargetTextureView = CreateNonMultisampledColorTextureView();

    utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].resolveTarget = resolveTargetTextureView;
    AssertBeginRenderPassSuccess(&renderPass);
}

// It is not allowed to use a resolve target in a format different from the color attachment.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ResolveTargetDifferentFormat) {
    constexpr wgpu::TextureFormat kColorFormat2 = wgpu::TextureFormat::BGRA8Unorm;
    wgpu::Texture resolveTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat2,
                                                 kSize, kSize, kArrayLayers, kLevelCount);
    wgpu::TextureView resolveTextureView = resolveTexture.CreateView();

    utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
    renderPass.cColorAttachments[0].resolveTarget = resolveTextureView;
    AssertBeginRenderPassError(&renderPass);
}

// Tests on the size of the resolve target.
TEST_F(MultisampledRenderPassDescriptorValidationTest,
       ColorAttachmentResolveTargetDimensionMismatch) {
    constexpr uint32_t kSize2 = kSize * 2;
    wgpu::Texture resolveTexture = CreateTexture(device, wgpu::TextureDimension::e2D, kColorFormat,
                                                 kSize2, kSize2, kArrayLayers, kLevelCount + 1);

    wgpu::TextureViewDescriptor textureViewDescriptor;
    textureViewDescriptor.nextInChain = nullptr;
    textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    textureViewDescriptor.format = kColorFormat;
    textureViewDescriptor.mipLevelCount = 1;
    textureViewDescriptor.baseArrayLayer = 0;
    textureViewDescriptor.arrayLayerCount = 1;

    {
        wgpu::TextureViewDescriptor firstMipLevelDescriptor = textureViewDescriptor;
        firstMipLevelDescriptor.baseMipLevel = 0;

        wgpu::TextureView resolveTextureView = resolveTexture.CreateView(&firstMipLevelDescriptor);

        utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
        renderPass.cColorAttachments[0].resolveTarget = resolveTextureView;
        AssertBeginRenderPassError(&renderPass);
    }

    {
        wgpu::TextureViewDescriptor secondMipLevelDescriptor = textureViewDescriptor;
        secondMipLevelDescriptor.baseMipLevel = 1;

        wgpu::TextureView resolveTextureView = resolveTexture.CreateView(&secondMipLevelDescriptor);

        utils::ComboRenderPassDescriptor renderPass = CreateMultisampledRenderPass();
        renderPass.cColorAttachments[0].resolveTarget = resolveTextureView;
        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// Tests the texture format of the resolve target must support being used as resolve target.
TEST_F(MultisampledRenderPassDescriptorValidationTest, ResolveTargetFormat) {
    for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
        if (!utils::TextureFormatSupportsMultisampling(format) ||
            utils::IsDepthOrStencilFormat(format)) {
            continue;
        }

        wgpu::Texture colorTexture =
            CreateTexture(device, wgpu::TextureDimension::e2D, format, kSize, kSize, kArrayLayers,
                          kLevelCount, kSampleCount);
        wgpu::Texture resolveTarget = CreateTexture(device, wgpu::TextureDimension::e2D, format,
                                                    kSize, kSize, kArrayLayers, kLevelCount, 1);

        utils::ComboRenderPassDescriptor renderPass({colorTexture.CreateView()});
        renderPass.cColorAttachments[0].resolveTarget = resolveTarget.CreateView();
        if (utils::TextureFormatSupportsResolveTarget(format)) {
            AssertBeginRenderPassSuccess(&renderPass);
        } else {
            AssertBeginRenderPassError(&renderPass);
        }
    }
}

// Tests on the sample count of depth stencil attachment.
TEST_F(MultisampledRenderPassDescriptorValidationTest, DepthStencilAttachmentSampleCount) {
    constexpr wgpu::TextureFormat kDepthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;
    wgpu::Texture multisampledDepthStencilTexture =
        CreateTexture(device, wgpu::TextureDimension::e2D, kDepthStencilFormat, kSize, kSize,
                      kArrayLayers, kLevelCount, kSampleCount);
    wgpu::TextureView multisampledDepthStencilTextureView =
        multisampledDepthStencilTexture.CreateView();

    // It is not allowed to use a depth stencil attachment whose sample count is different from
    // the one of the color attachment.
    {
        wgpu::Texture depthStencilTexture =
            CreateTexture(device, wgpu::TextureDimension::e2D, kDepthStencilFormat, kSize, kSize,
                          kArrayLayers, kLevelCount);
        wgpu::TextureView depthStencilTextureView = depthStencilTexture.CreateView();

        utils::ComboRenderPassDescriptor renderPass({CreateMultisampledColorTextureView()},
                                                    depthStencilTextureView);
        AssertBeginRenderPassError(&renderPass);
    }

    {
        utils::ComboRenderPassDescriptor renderPass({CreateNonMultisampledColorTextureView()},
                                                    multisampledDepthStencilTextureView);
        AssertBeginRenderPassError(&renderPass);
    }

    // It is allowed to use a multisampled depth stencil attachment whose sample count is equal
    // to the one of the color attachment.
    {
        utils::ComboRenderPassDescriptor renderPass({CreateMultisampledColorTextureView()},
                                                    multisampledDepthStencilTextureView);
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // It is allowed to use a multisampled depth stencil attachment while there is no color
    // attachment.
    {
        utils::ComboRenderPassDescriptor renderPass({}, multisampledDepthStencilTextureView);
        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// Tests that NaN cannot be accepted as a valid color or depth clear value and INFINITY is valid
// in both color and depth clear values.
TEST_F(RenderPassDescriptorValidationTest, UseNaNOrINFINITYAsColorOrDepthClearValue) {
    wgpu::TextureView color = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);

    // Tests that NaN cannot be used in clearColor.
    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.r = NAN;
        AssertBeginRenderPassError(&renderPass);
    }

    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.g = NAN;
        AssertBeginRenderPassError(&renderPass);
    }

    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.b = NAN;
        AssertBeginRenderPassError(&renderPass);
    }

    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.a = NAN;
        AssertBeginRenderPassError(&renderPass);
    }

    // Tests that INFINITY can be used in clearColor.
    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.r = INFINITY;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.g = INFINITY;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.b = INFINITY;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    {
        utils::ComboRenderPassDescriptor renderPass({color}, nullptr);
        renderPass.cColorAttachments[0].clearValue.a = INFINITY;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Tests that NaN cannot be used in depthClearValue.
    {
        wgpu::TextureView depth =
            Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24Plus);
        utils::ComboRenderPassDescriptor renderPass({color}, depth);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthClearValue = NAN;
        AssertBeginRenderPassError(&renderPass);
    }

    // Tests that INFINITY cannot be used in depthClearValue.
    {
        wgpu::TextureView depth =
            Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24Plus);
        utils::ComboRenderPassDescriptor renderPass({color}, depth);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthClearValue = INFINITY;
        AssertBeginRenderPassError(&renderPass);
    }

    // TODO(https://crbug.com/dawn/666): Add a test case for clearStencil for stencilOnly
    // once stencil8 is supported.
}

// Tests that depth clear values mut be between 0 and 1, inclusive.
TEST_F(RenderPassDescriptorValidationTest, ValidateDepthClearValueRange) {
    wgpu::TextureView depth = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24Plus);

    utils::ComboRenderPassDescriptor renderPass({}, depth);
    renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
    renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

    // 0, 1, and any value in between are be valid.
    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 0;
    AssertBeginRenderPassSuccess(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 0.1;
    AssertBeginRenderPassSuccess(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 0.5;
    AssertBeginRenderPassSuccess(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 0.82;
    AssertBeginRenderPassSuccess(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 1;
    AssertBeginRenderPassSuccess(&renderPass);

    // Values less than 0 or greater than 1 are invalid.
    renderPass.cDepthStencilAttachmentInfo.depthClearValue = -1;
    AssertBeginRenderPassError(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 2;
    AssertBeginRenderPassError(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = -0.001;
    AssertBeginRenderPassError(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 1.001;
    AssertBeginRenderPassError(&renderPass);

    // Clear values are not validated if the depthLoadOp is Load.
    renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = -1;
    AssertBeginRenderPassSuccess(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 2;
    AssertBeginRenderPassSuccess(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = -0.001;
    AssertBeginRenderPassSuccess(&renderPass);

    renderPass.cDepthStencilAttachmentInfo.depthClearValue = 1.001;
    AssertBeginRenderPassSuccess(&renderPass);
}

// Tests that default depthClearValue is required if attachment has a depth aspect and depthLoadOp
// is clear.
TEST_F(RenderPassDescriptorValidationTest, DefaultDepthClearValue) {
    wgpu::TextureView depthView =
        Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24Plus);
    wgpu::TextureView stencilView = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Stencil8);

    wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;

    wgpu::RenderPassDescriptor renderPassDescriptor;
    renderPassDescriptor.colorAttachmentCount = 0;
    renderPassDescriptor.colorAttachments = nullptr;
    renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;

    // Default depthClearValue should be accepted if attachment doesn't have
    // a depth aspect.
    depthStencilAttachment.view = stencilView;
    depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Load;
    depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
    AssertBeginRenderPassSuccess(&renderPassDescriptor);

    // Default depthClearValue should be accepted if depthLoadOp is not clear.
    depthStencilAttachment.view = depthView;
    depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Undefined;
    depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Undefined;
    depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Load;
    depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
    AssertBeginRenderPassSuccess(&renderPassDescriptor);

    // Default depthClearValue should fail the validation
    // if attachment has a depth aspect and depthLoadOp is clear.
    depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
    AssertBeginRenderPassError(&renderPassDescriptor);

    // The validation should pass if valid depthClearValue is provided.
    depthStencilAttachment.depthClearValue = 0.0f;
    AssertBeginRenderPassSuccess(&renderPassDescriptor);
}

TEST_F(RenderPassDescriptorValidationTest, ValidateDepthStencilReadOnly) {
    wgpu::TextureView colorView = Create2DAttachment(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
    wgpu::TextureView depthStencilView =
        Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24PlusStencil8);
    wgpu::TextureView depthStencilViewNoStencil =
        Create2DAttachment(device, 1, 1, wgpu::TextureFormat::Depth24Plus);

    // Tests that a read-only pass with depthReadOnly set to true succeeds.
    {
        utils::ComboRenderPassDescriptor renderPass({colorView}, depthStencilView);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthReadOnly = true;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilReadOnly = true;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Tests that a pass with mismatched depthReadOnly and stencilReadOnly values passes when
    // there is no stencil component in the format (deprecated).
    {
        utils::ComboRenderPassDescriptor renderPass({colorView}, depthStencilViewNoStencil);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthReadOnly = true;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Load;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;
        renderPass.cDepthStencilAttachmentInfo.stencilReadOnly = false;
        AssertBeginRenderPassError(&renderPass);
    }

    // Tests that a pass with mismatched depthReadOnly and stencilReadOnly values fails when
    // there there is no stencil component in the format and stencil loadOp/storeOp are passed.
    {
        utils::ComboRenderPassDescriptor renderPass({colorView}, depthStencilViewNoStencil);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthReadOnly = true;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Clear;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;
        renderPass.cDepthStencilAttachmentInfo.stencilReadOnly = false;
        AssertBeginRenderPassError(&renderPass);

        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;
        AssertBeginRenderPassError(&renderPass);

        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Clear;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilReadOnly = false;
        AssertBeginRenderPassError(&renderPass);
    }

    // Tests that a pass with depthReadOnly=true and stencilReadOnly=true can pass
    // when there is only depth component in the format. We actually enable readonly
    // depth/stencil attachment in this case.
    {
        utils::ComboRenderPassDescriptor renderPass({colorView}, depthStencilViewNoStencil);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthReadOnly = true;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilReadOnly = true;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Tests that a pass with depthReadOnly=false and stencilReadOnly=true can pass
    // when there is only depth component in the format. We actually don't enable readonly
    // depth/stencil attachment in this case.
    {
        utils::ComboRenderPassDescriptor renderPass({colorView}, depthStencilViewNoStencil);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Store;
        renderPass.cDepthStencilAttachmentInfo.depthReadOnly = false;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilReadOnly = true;
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // TODO(https://crbug.com/dawn/666): Add a test case for stencil-only once stencil8 is
    // supported (depthReadOnly and stencilReadOnly mismatch but no depth component).

    // Tests that a pass with mismatched depthReadOnly and stencilReadOnly values fails when
    // both depth and stencil components exist.
    {
        utils::ComboRenderPassDescriptor renderPass({colorView}, depthStencilView);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthReadOnly = true;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Load;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;
        renderPass.cDepthStencilAttachmentInfo.stencilReadOnly = false;
        AssertBeginRenderPassError(&renderPass);
    }

    // Tests that a pass with loadOp set to clear and readOnly set to true fails.
    {
        utils::ComboRenderPassDescriptor renderPass({colorView}, depthStencilView);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Store;
        renderPass.cDepthStencilAttachmentInfo.depthReadOnly = true;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Clear;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;
        renderPass.cDepthStencilAttachmentInfo.stencilReadOnly = true;
        AssertBeginRenderPassError(&renderPass);
    }

    // Tests that a pass with storeOp set to discard and readOnly set to true fails.
    {
        utils::ComboRenderPassDescriptor renderPass({colorView}, depthStencilView);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Discard;
        renderPass.cDepthStencilAttachmentInfo.depthReadOnly = true;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Load;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Discard;
        renderPass.cDepthStencilAttachmentInfo.stencilReadOnly = true;
        AssertBeginRenderPassError(&renderPass);
    }

    // Tests that a pass with only depthLoadOp set to load and readOnly set to true fails.
    {
        utils::ComboRenderPassDescriptor renderPass({colorView}, depthStencilView);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthReadOnly = true;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilReadOnly = true;
        AssertBeginRenderPassError(&renderPass);
    }

    // Tests that a pass with only depthStoreOp set to store and readOnly set to true fails.
    {
        utils::ComboRenderPassDescriptor renderPass({colorView}, depthStencilView);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Store;
        renderPass.cDepthStencilAttachmentInfo.depthReadOnly = true;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilReadOnly = true;
        AssertBeginRenderPassError(&renderPass);
    }

    // Tests that a pass with only stencilLoadOp set to load and readOnly set to true fails.
    {
        utils::ComboRenderPassDescriptor renderPass({colorView}, depthStencilView);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthReadOnly = true;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Load;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilReadOnly = true;
        AssertBeginRenderPassError(&renderPass);
    }

    // Tests that a pass with only stencilStoreOp set to store and readOnly set to true fails.
    {
        utils::ComboRenderPassDescriptor renderPass({colorView}, depthStencilView);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthReadOnly = true;
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;
        renderPass.cDepthStencilAttachmentInfo.stencilReadOnly = true;
        AssertBeginRenderPassError(&renderPass);
    }
}

// Check that the depth stencil attachment must use all aspects.
TEST_F(RenderPassDescriptorValidationTest, ValidateDepthStencilAllAspects) {
    wgpu::TextureDescriptor texDesc;
    texDesc.usage = wgpu::TextureUsage::RenderAttachment;
    texDesc.size = {1, 1, 1};

    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 1;

    // Using all aspects of a depth+stencil texture is allowed.
    {
        texDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.format = wgpu::TextureFormat::Undefined;
        viewDesc.aspect = wgpu::TextureAspect::All;

        wgpu::TextureView view = device.CreateTexture(&texDesc).CreateView(&viewDesc);
        utils::ComboRenderPassDescriptor renderPass({}, view);
        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using only depth of a depth+stencil texture is an error, case without format
    // reinterpretation.
    {
        texDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.format = wgpu::TextureFormat::Undefined;
        viewDesc.aspect = wgpu::TextureAspect::DepthOnly;

        wgpu::TextureView view = device.CreateTexture(&texDesc).CreateView(&viewDesc);
        utils::ComboRenderPassDescriptor renderPass({}, view);
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassError(&renderPass);
    }

    // Using only depth of a depth+stencil texture is an error, case with format reinterpretation.
    {
        texDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.format = wgpu::TextureFormat::Depth24Plus;
        viewDesc.aspect = wgpu::TextureAspect::DepthOnly;

        wgpu::TextureView view = device.CreateTexture(&texDesc).CreateView(&viewDesc);
        utils::ComboRenderPassDescriptor renderPass({}, view);
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassError(&renderPass);
    }

    // Using only stencil of a depth+stencil texture is an error, case without format
    // reinterpration.
    {
        texDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.format = wgpu::TextureFormat::Undefined;
        viewDesc.aspect = wgpu::TextureAspect::StencilOnly;

        wgpu::TextureView view = device.CreateTexture(&texDesc).CreateView(&viewDesc);
        utils::ComboRenderPassDescriptor renderPass({}, view);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassError(&renderPass);
    }

    // Using only stencil of a depth+stencil texture is an error, case with format reinterpretation.
    {
        texDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.format = wgpu::TextureFormat::Stencil8;
        viewDesc.aspect = wgpu::TextureAspect::StencilOnly;

        wgpu::TextureView view = device.CreateTexture(&texDesc).CreateView(&viewDesc);
        utils::ComboRenderPassDescriptor renderPass({}, view);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassError(&renderPass);
    }

    // Using DepthOnly of a depth only texture is allowed.
    {
        texDesc.format = wgpu::TextureFormat::Depth24Plus;
        viewDesc.format = wgpu::TextureFormat::Undefined;
        viewDesc.aspect = wgpu::TextureAspect::DepthOnly;

        wgpu::TextureView view = device.CreateTexture(&texDesc).CreateView(&viewDesc);
        utils::ComboRenderPassDescriptor renderPass({}, view);
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassSuccess(&renderPass);
    }

    // Using StencilOnly of a stencil only texture is allowed.
    {
        texDesc.format = wgpu::TextureFormat::Stencil8;
        viewDesc.format = wgpu::TextureFormat::Undefined;
        viewDesc.aspect = wgpu::TextureAspect::StencilOnly;

        wgpu::TextureView view = device.CreateTexture(&texDesc).CreateView(&viewDesc);
        utils::ComboRenderPassDescriptor renderPass({}, view);
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;

        AssertBeginRenderPassSuccess(&renderPass);
    }
}

// Tests validation for per-pixel accounting for render targets. The tests currently assume that the
// default maxColorAttachmentBytesPerSample limit of 32 is used.
TEST_P(DeprecationTests, RenderPassColorAttachmentBytesPerSample) {
    struct TestCase {
        std::vector<wgpu::TextureFormat> formats;
        bool success;
    };
    static std::vector<TestCase> kTestCases = {
        // Simple 1 format cases.

        // R8Unorm take 1 byte and are aligned to 1 byte so we can have 8 (max).
        {{wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm},
         true},
        // RGBA8Uint takes 4 bytes and are aligned to 1 byte so we can have 8 (max).
        {{wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint},
         true},
        // RGBA8Unorm takes 8 bytes (special case) and are aligned to 1 byte so only 4 allowed.
        {{wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm},
         true},
        {{wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm},
         false},
        // RGBA32Float takes 16 bytes and are aligned to 4 bytes so only 2 are allowed.
        {{wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::RGBA32Float}, true},
        {{wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::RGBA32Float,
          wgpu::TextureFormat::RGBA32Float},
         false},

        // Different format alignment cases.

        // Alignment causes the first 1 byte R8Unorm to become 4 bytes. So even though 1+4+8+16+1 <
        // 32, the 4 byte alignment requirement of R32Float makes the first R8Unorm become 4 and
        // 4+4+8+16+1 > 32. Re-ordering this so the R8Unorm's are at the end, however is allowed:
        // 4+8+16+1+1 < 32.
        {{wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R32Float,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA32Float,
          wgpu::TextureFormat::R8Unorm},
         false},
        {{wgpu::TextureFormat::R32Float, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm},
         true},
    };

    for (const TestCase& testCase : kTestCases) {
        std::vector<wgpu::TextureView> colorAttachmentInfo;
        for (size_t i = 0; i < testCase.formats.size(); i++) {
            colorAttachmentInfo.push_back(Create2DAttachment(device, 1, 1, testCase.formats.at(i)));
        }
        utils::ComboRenderPassDescriptor descriptor(colorAttachmentInfo);
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        if (testCase.success) {
            wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&descriptor);
            renderPassEncoder.End();
            commandEncoder.Finish();
        } else {
            EXPECT_DEPRECATION_WARNING_ONLY(commandEncoder.BeginRenderPass(&descriptor));
        }
    }
}

// TODO(cwallez@chromium.org): Constraints on attachment aliasing?

}  // anonymous namespace
