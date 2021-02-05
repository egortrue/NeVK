#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <resourcemanager.h>
#include "paramblock.h"
#include <glm/gtx/compatibility.hpp>

namespace nevk
{
class RenderPass
{
private:
    struct UniformBufferObject
    {
        alignas(16) glm::mat4 modelViewProj;
    };

    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;
    VkPipeline mPipeline;
    VkPipelineLayout mPipelineLayout;
    VkRenderPass mRenderPass;
    VkDevice mDevice;

    VkShaderModule mVS, mPS;

    ResourceManager* mResMngr;
    VkDescriptorPool mDescriptorPool;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    VkImageView mTextureImageView;
    VkSampler mTextureSampler;
    VkBuffer mMaterialBuffer;
    uint32_t mMaterialCount = 0;

    void createRenderPass();

    void createDescriptorSetLayout();
    void createDescriptorSets(VkDescriptorPool& descriptorPool);

    void createUniformBuffers();

    VkDescriptorSetLayout mDescriptorSetLayout;
    std::vector<VkDescriptorSet> mDescriptorSets;

    std::vector<VkDescriptorSet> mParamDescSet;

    std::vector<VkFramebuffer> mFrameBuffers;

    VkFormat mFrameBufferFormat;
    VkFormat mDepthBufferFormat;
    uint32_t mWidth, mHeight;

    VkVertexInputBindingDescription getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();

    VkShaderModule createShaderModule(const char* code, const uint32_t codeSize);

public:
    void createGraphicsPipeline(VkShaderModule& vertShaderModule, VkShaderModule& fragShaderModule, uint32_t width, uint32_t height);

    void createFrameBuffers(std::vector<VkImageView>& imageViews, VkImageView& depthImageView, uint32_t width, uint32_t height);

    void setFrameBufferFormat(VkFormat format)
    {
        mFrameBufferFormat = format;
    }

    void setDepthBufferFormat(VkFormat format)
    {
        mDepthBufferFormat = format;
    }

    void setTextureImageView(VkImageView textureImageView);
    void setTextureSampler(VkSampler textureSampler);

    void setMaterialBuffer(VkBuffer buff, uint32_t count)
    {
        mMaterialBuffer = buff;
        mMaterialCount = count;
    }

    void init(VkDevice& device, const char* vsCode, uint32_t vsCodeSize, const char* psCode, uint32_t psCodeSize, VkDescriptorPool descpool, ResourceManager* resMngr, uint32_t width, uint32_t height)
    {
        mDevice = device;
        mResMngr = resMngr;
        mDescriptorPool = descpool;
        mWidth = width;
        mHeight = height;
        mVS = createShaderModule(vsCode, vsCodeSize);
        mPS = createShaderModule(psCode, psCodeSize);
        createUniformBuffers();

        createRenderPass();
        createDescriptorSetLayout();
        createDescriptorSets(mDescriptorPool);
        createGraphicsPipeline(mVS, mPS, width, height);
    }

    void onResize(std::vector<VkImageView>& imageViews, VkImageView& depthImageView, uint32_t width, uint32_t height);

    void onDestroy();

    void updateUniformBuffer(uint32_t currentImage, const glm::float4x4& perspective, const glm::float4x4& view);

    RenderPass(/* args */);
    ~RenderPass();

    void record(VkCommandBuffer& cmd, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indicesCount, uint32_t width, uint32_t height, uint32_t imageIndex);
};
} // namespace nevk
