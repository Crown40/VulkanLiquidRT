
#pragma once

#include "Base/Device.h"
#include "Base/ShaderCompilerHelper.h"
//Deprecate: #include "PhysicsRT.h"
#include "../../libs/vectormath/vectormath.hpp"

namespace CAULDRON_VK
{
    struct ParticlePropertyComponentAlign
    {
        //float x; float y; float z; float align;
        math::Vector4 vec4; //data
    };

    struct ParticleProperty
    {
        ParticlePropertyComponentAlign position;
        ParticlePropertyComponentAlign velocity;
        ParticlePropertyComponentAlign acceleration;
    };

    //Deprecate: typedef struct,class
    struct ParticleData 
    {
    public:
        // RT|Render Type
        uint32_t                                m_numParticle;
        //uint32_t                                m_numProperty = 1;
        // m_particlePosition_descriptorBufferInfo be set at m_particlePropertySOA_descriptorBufferInfo[0]
        //VkDescriptorBufferInfo                  m_particlePosition_descriptorBufferInfo;
        //std::vector<VkDescriptorBufferInfo>     m_particleBufferAOS_descriptorBufferInfo;
        std::vector<VkDescriptorBufferInfo>     m_particlePropertySOA_descriptorBufferInfo = std::vector<VkDescriptorBufferInfo>(1);
        //std::map<std::string,VkDescriptorBufferInfo> m_particleBufferSOA_descriptorBufferInfo;
        VkDescriptorSetLayout				    m_particle_descriptorSetLayout;
        VkDescriptorSet						    m_particle_descriptorSet;
        VkDescriptorSetLayout				    m_particlePosition_descriptorSetLayout;
        VkDescriptorSet						    m_particlePosition_descriptorSet;
        //
        uint32_t GetNumParticle() { return m_numParticle; }
        uint32_t GetNumProperty() { return (uint32_t)m_particlePropertySOA_descriptorBufferInfo.size(); }
        //
        VkDescriptorBufferInfo GetPositionDescriptorBufferInfo() { return m_particlePropertySOA_descriptorBufferInfo[0]; }
        void SetPositionDescriptorBufferInfo(VkDescriptorBufferInfo		descriptorBufferInfo) { m_particlePropertySOA_descriptorBufferInfo[0] = descriptorBufferInfo; }
        std::vector<VkDescriptorBufferInfo> GetPropertyArrayDescriptorBufferInfo() { return m_particlePropertySOA_descriptorBufferInfo; }
        bool SetPropertyDescriptorBufferInfo(uint32_t property_index, VkDescriptorBufferInfo descriptorBufferInfo)
        {
            bool res = property_index < m_particlePropertySOA_descriptorBufferInfo.size();
            if (res)
            {
                m_particlePropertySOA_descriptorBufferInfo[property_index] = descriptorBufferInfo;
            }
            //
            return res;
            //
        }
        //void AddPropertyDescriptorBufferInfo(VkDescriptorBufferInfo descriptorBufferInfo) { m_particlePropertySOA_descriptorBufferInfo.push_back(descriptorBufferInfo); }
        //
        VkDescriptorSetLayout GetDescriptorSetLayout() { return m_particle_descriptorSetLayout; }
        void SetDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) { m_particle_descriptorSetLayout = descriptorSetLayout; }
        VkDescriptorSet GetDescriptorSet() { return m_particle_descriptorSet; }
        void SetDescriptorSet(VkDescriptorSet descriptorSet) { m_particle_descriptorSet = descriptorSet; }
        //
        VkDescriptorSetLayout GetPositionDescriptorSetLayout() { return m_particlePosition_descriptorSetLayout; }
        void SetPositionDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) { m_particlePosition_descriptorSetLayout = descriptorSetLayout; }
        VkDescriptorSet GetPositionDescriptorSet() { return m_particlePosition_descriptorSet; }
        void SePositiontDescriptorSet(VkDescriptorSet descriptorSet) { m_particlePosition_descriptorSet = descriptorSet; }
        //
        void OnInit(uint32_t numParticle, uint32_t numProperty)
        {
            //
            m_numParticle = numParticle;
            assert(numProperty > 0);
            m_particlePropertySOA_descriptorBufferInfo.resize(numProperty, {});
            //
        }
        //
        //void OnDestroy(VkDevice device, std::vector<VkDescriptorBufferInfo>* pOut_arrayDesciptorBufferInfo, std::vector<VkDescriptorSet>* pOut_arrayDesciptorSet)
        void OnDestroy(VkDevice device, std::vector<VkDescriptorSet>* pOut_arrayDesciptorSet)
        {
            //
            pOut_arrayDesciptorSet->push_back(m_particlePosition_descriptorSet);
            m_particlePosition_descriptorSet = VK_NULL_HANDLE;
            vkDestroyDescriptorSetLayout(device, m_particlePosition_descriptorSetLayout, NULL);
            m_particlePosition_descriptorSetLayout = VK_NULL_HANDLE;

            pOut_arrayDesciptorSet->push_back(m_particle_descriptorSet);
            m_particle_descriptorSet = VK_NULL_HANDLE;
            vkDestroyDescriptorSetLayout(device, m_particle_descriptorSetLayout, NULL);
            m_particle_descriptorSetLayout = VK_NULL_HANDLE;
            //
            //for (auto& ref : m_particlePropertySOA_descriptorBufferInfo)
            //{
            //    pOut_arrayDesciptorBufferInfo->push_back(ref);
            //}
            m_particlePropertySOA_descriptorBufferInfo.clear();
            //
            m_numParticle = 0;
            //
        }
        //
    };


    class ParticleCS
    {
    public:
        void OnCreate(
            Device* pDevice,
            const std::string &shaderFilename,
            const std::string &shaderEntryPoint,
            const std::string &shaderCompilerParams,
            //VkDescriptorSetLayout descriptorSetLayout,
            std::vector<VkDescriptorSetLayout>& array_descriptorSetLayout,
            uint32_t dwWidth, uint32_t dwHeight, uint32_t dwDepth,
            DefineList* pUserDefines = 0
        );
        void OnDestroy();
        //void Draw(VkCommandBuffer cmd_buf, VkDescriptorBufferInfo *pConstantBuffer, VkDescriptorSet descSet, uint32_t dispatchX, uint32_t dispatchY, uint32_t dispatchZ);
        void Draw(VkCommandBuffer cmd_buf
            , std::vector<VkDescriptorSet>& array_storageDescriptorSet
            , VkDescriptorBufferInfo* pConstantBuffer, VkDescriptorSet constantDescriptorSet
            , uint32_t dispatchX, uint32_t dispatchY, uint32_t dispatchZ);

        //Deprecate
        //void InitParticlePosition();
        //uint32_t GetParticleNum() { return m_particleData.GetNumParticle(); }
        //VkDescriptorBufferInfo GetParticlePosition() { return m_particleData.GetPositionDescriptorBufferInfo(); }
        //void OnCreateParticleData(ParticleData& particleData, uint32_t numParticle, uint32_t numProperty);

    private:
        //
        Device*             m_pDevice;

        // ComputePipeline
        VkPipelineShaderStageCreateInfo     m_shaderStage;
        VkPipeline          m_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout    m_pipelineLayout = VK_NULL_HANDLE;
        // Deprecate: GraphicsPipeline
        //VkPipeline        m_graphicsPipeline = VK_NULL_HANDLE;
        //VkPipelineLayout  m_graphicsPipelineLayout = VK_NULL_HANDLE;

        //Deprecate
        //ParticleData        m_particleData;
        //Deprecate
        //PhysicsRT           m_physicsRT;
        //
    };
}
