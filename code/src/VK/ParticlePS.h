
#pragma once

//#include "base/StaticBufferPool.h"
//#include "base/DynamicBufferRing.h"
//#include "base/ResourceViewHeaps.h"
//#include "base/Device.h"
#include "HeapDescriptorSetAllocater.h"
#include "DynamicBufferAllocater.h"
#include "StaticBufferAllocater.h"

namespace CAULDRON_VK
{
    struct ShaderCompileParameter
    {
        const char* pShaderFilename;
        const char* pShaderEntryPoint;
        const char* pShaderCompilerParams;
    };

    struct ParticleVertexInputStateParameter
    {
        std::vector<VkVertexInputBindingDescription>   array_vertexInputBindingDescription;
        std::vector<VkVertexInputAttributeDescription> array_vertexInputAttributeDescription;
    };


    class ParticlePS
    {
    public:
        void OnCreate(
            Device* pDevice,
            VkRenderPass renderPass,
            //const std::string& shaderFilename,
            //const std::string& shaderEntryPoint,
            //const std::string& shaderCompilerParams,
            std::array<uint32_t, 3> numDimensions,
            ShaderCompileParameter vS_particlePsCreateInfo,
            ShaderCompileParameter fS_particlePsCreateInfo,
            ParticleVertexInputStateParameter* pParticleVertexInputStateParameter,
            //StaticBufferPool* pStaticBufferPool,
            StaticBufferAllocater* pStaticBufferAllocater,
            //DynamicBufferRing* pDynamicBufferRing,
            DynamicBufferAllocater* pDynamicBufferAllocater,
            //VkDescriptorSetLayout descriptorSetLayout,
            std::vector<VkDescriptorSetLayout>& array_descriptorSetLayout,
            VkPipelineColorBlendStateCreateInfo* pBlendDesc = NULL,
            VkSampleCountFlagBits sampleDescCount = VK_SAMPLE_COUNT_1_BIT
        );
        void OnDestroy();
        void UpdatePipeline(VkRenderPass renderPass, ParticleVertexInputStateParameter* pParticleVertexInputStateParameter
            , VkPipelineColorBlendStateCreateInfo* pBlendDesc = NULL, VkSampleCountFlagBits sampleDescCount = VK_SAMPLE_COUNT_1_BIT);
        //void Draw(VkCommandBuffer cmd_buf, VkDescriptorBufferInfo* pConstantBuffer, VkDescriptorSet descriptorSet = NULL);
        void Draw(VkCommandBuffer cmd_buf, VkDescriptorBufferInfo* pConstantBuffer, VkDescriptorSet constantDescriptorSet
            , VkDescriptorBufferInfo* pVertexBuffer, uint32_t numVertex);

    private:
        Device* m_pDevice;
        std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;
        std::string m_vertexShaderName;
        std::string m_fragmentShaderName;

        // all bounding boxes of all the meshes use the same geometry, shaders and pipelines.
        uint32_t m_NumIndices;
        VkIndexType m_indexType;
        VkDescriptorBufferInfo m_IBV;

        VkPipeline m_pipeline = VK_NULL_HANDLE;
        //VkRenderPass m_renderPass = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

        //

    };
}
