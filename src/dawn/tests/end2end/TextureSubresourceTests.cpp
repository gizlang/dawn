// Copyright 2020 The Dawn Authors
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

#include "dawn/tests/DawnTest.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

class TextureSubresourceTest : public DawnTest {
  public:
    static constexpr uint32_t kSize = 4u;
    static constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::Texture CreateTexture(uint32_t mipLevelCount,
                                uint32_t arrayLayerCount,
                                wgpu::TextureUsage usage) {
        wgpu::TextureDescriptor texDesc;
        texDesc.dimension = wgpu::TextureDimension::e2D;
        texDesc.size = {kSize, kSize, arrayLayerCount};
        texDesc.sampleCount = 1;
        texDesc.mipLevelCount = mipLevelCount;
        texDesc.usage = usage;
        texDesc.format = kFormat;
        return device.CreateTexture(&texDesc);
    }

    wgpu::TextureView CreateTextureView(wgpu::Texture texture,
                                        uint32_t baseMipLevel,
                                        uint32_t baseArrayLayer) {
        wgpu::TextureViewDescriptor viewDesc;
        viewDesc.format = kFormat;
        viewDesc.baseArrayLayer = baseArrayLayer;
        viewDesc.arrayLayerCount = 1;
        viewDesc.baseMipLevel = baseMipLevel;
        viewDesc.mipLevelCount = 1;
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        return texture.CreateView(&viewDesc);
    }

    void DrawTriangle(const wgpu::TextureView& view) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0,  1.0),
                    vec2f(-1.0, -1.0),
                    vec2f( 1.0, -1.0));

                return vec4f(pos[VertexIndex], 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(1.0, 0.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        descriptor.cTargets[0].format = kFormat;

        wgpu::RenderPipeline rp = device.CreateRenderPipeline(&descriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor renderPassDesc({view});
        renderPassDesc.cColorAttachments[0].clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
        pass.SetPipeline(rp);
        pass.Draw(3);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    void SampleAndDraw(const wgpu::TextureView& samplerView, const wgpu::TextureView& renderView) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0, -1.0),
                    vec2f( 1.0,  1.0),
                    vec2f(-1.0,  1.0),
                    vec2f(-1.0, -1.0),
                    vec2f( 1.0, -1.0),
                    vec2f( 1.0,  1.0));

                return vec4f(pos[VertexIndex], 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var samp : sampler;
            @group(0) @binding(1) var tex : texture_2d<f32>;

            @fragment
            fn main(@builtin(position) FragCoord : vec4f) -> @location(0) vec4f {
                return textureSample(tex, samp, FragCoord.xy / vec2f(4.0, 4.0));
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        descriptor.cTargets[0].format = kFormat;

        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::RenderPipeline rp = device.CreateRenderPipeline(&descriptor);
        wgpu::BindGroupLayout bgl = rp.GetBindGroupLayout(0);
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, bgl, {{0, sampler}, {1, samplerView}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor renderPassDesc({renderView});
        renderPassDesc.cColorAttachments[0].clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
        pass.SetPipeline(rp);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(6);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }
};

// Test different mipmap levels
TEST_P(TextureSubresourceTest, MipmapLevelsTest) {
    // Create a texture with 2 mipmap levels and 1 layer
    wgpu::Texture texture =
        CreateTexture(2, 1,
                      wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment |
                          wgpu::TextureUsage::CopySrc);

    // Create two views on different mipmap levels.
    wgpu::TextureView samplerView = CreateTextureView(texture, 0, 0);
    wgpu::TextureView renderView = CreateTextureView(texture, 1, 0);

    // Draw a red triangle at the bottom-left half
    DrawTriangle(samplerView);

    // Sample from one subresource and draw into another subresource in the same texture
    SampleAndDraw(samplerView, renderView);

    // Verify that pixel at bottom-left corner is red, while pixel at top-right corner is background
    // black in render view (mip level 1).
    utils::RGBA8 topRight = utils::RGBA8::kBlack;
    utils::RGBA8 bottomLeft = utils::RGBA8::kRed;
    EXPECT_TEXTURE_EQ(&topRight, texture, {kSize / 2 - 1, 0}, {1, 1}, 1);
    EXPECT_TEXTURE_EQ(&bottomLeft, texture, {0, kSize / 2 - 1}, {1, 1}, 1);
}

// Test different array layers
TEST_P(TextureSubresourceTest, ArrayLayersTest) {
    // Create a texture with 1 mipmap level and 2 layers
    wgpu::Texture texture =
        CreateTexture(1, 2,
                      wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment |
                          wgpu::TextureUsage::CopySrc);

    // Create two views on different layers
    wgpu::TextureView samplerView = CreateTextureView(texture, 0, 0);
    wgpu::TextureView renderView = CreateTextureView(texture, 0, 1);

    // Draw a red triangle at the bottom-left half
    DrawTriangle(samplerView);

    // Sample from one subresource and draw into another subresource in the same texture
    SampleAndDraw(samplerView, renderView);

    // Verify that pixel at bottom-left corner is red, while pixel at top-right corner is background
    // black in render view (array layer 1).
    utils::RGBA8 topRight = utils::RGBA8::kBlack;
    utils::RGBA8 bottomLeft = utils::RGBA8::kRed;
    EXPECT_TEXTURE_EQ(&topRight, texture, {kSize - 1, 0, 1}, {1, 1});
    EXPECT_TEXTURE_EQ(&bottomLeft, texture, {0, kSize - 1, 1}, {1, 1});
}

DAWN_INSTANTIATE_TEST(TextureSubresourceTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
