
#include "stdafx.h"

#include "ParticleCS.h"

namespace CAULDRON_VK
{
    void ParticleCS::OnCreate(
        Device* pDevice,
        const std::string& shaderFilename,
        const std::string& shaderEntryPoint,
        const std::string& shaderCompilerParams,
        std::vector<VkDescriptorSetLayout>& array_descriptorSetLayout,  // VkDescriptorSetLayout
        uint32_t dwWidth, uint32_t dwHeight, uint32_t dwDepth,
        DefineList* pUserDefines    // DefineList : public std::map<const std::string, std::string>
    )
    {
        m_pDevice = pDevice;

        VkResult res;

        {
            // Compile shaders
            // VkPipelineShaderStageCreateInfo would be used by VkComputePipelineCreateInfo
            //
            VkPipelineShaderStageCreateInfo computeShader_pipelineShaderStageCreateInfo;  //
            DefineList defines;
            if (pUserDefines)
                defines = *pUserDefines;
            // Use DefineList* to add #define
            defines["WIDTH"] = std::to_string(dwWidth);
            defines["HEIGHT"] = std::to_string(dwHeight);
            defines["DEPTH"] = std::to_string(dwDepth);
            /*
            */

            //computeShader_pipelineShaderStageCreateInfo::member be set values by VKCompileFromFile()
            // Update computeShader{stage=VK_SHADER_STAGE_COMPUTE_BIT, module=ShaderModule(shaderFilename.c_str()), pName=shaderEntryPoint.c_str()} 
            // with defines by VKCompileFromFile()
            res = VKCompileFromFile(m_pDevice->GetDevice(), VK_SHADER_STAGE_COMPUTE_BIT, shaderFilename.c_str(), shaderEntryPoint.c_str(), shaderCompilerParams.c_str(), &defines, &computeShader_pipelineShaderStageCreateInfo);
            assert(res == VK_SUCCESS);

            m_shaderStage = computeShader_pipelineShaderStageCreateInfo;
            //

            // Create pipeline layout
            //  Compute_pipeline-layout
            //
            VkPipelineLayoutCreateInfo pipelineLayout_createInfo = {};
            pipelineLayout_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayout_createInfo.pNext = NULL;
            // push constant 可以看作一种轻量级的uniform变量
            pipelineLayout_createInfo.pushConstantRangeCount = 0;
            pipelineLayout_createInfo.pPushConstantRanges = NULL;
            //pipelineLayout_createInfo.setLayoutCount = 1;
            pipelineLayout_createInfo.setLayoutCount = (uint32_t)array_descriptorSetLayout.size();
            //pipelineLayout_createInfo.pSetLayouts = &descriptorSetLayout;
            pipelineLayout_createInfo.pSetLayouts = array_descriptorSetLayout.data();

            // m_pipelineLayout
            res = vkCreatePipelineLayout(pDevice->GetDevice(), &pipelineLayout_createInfo, NULL, &m_pipelineLayout);
            assert(res == VK_SUCCESS);
            //

            // Create pipeline
            // VkComputePipelineCreateInfo
            // VkGraphicsPipelineCreateInfo
            //  Compute_pipeline
            //
            VkComputePipelineCreateInfo computePipeline_createInfo = {};
            computePipeline_createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            computePipeline_createInfo.pNext = NULL;
            //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineCreateFlagBits.html
            computePipeline_createInfo.flags = 0;
            // VkPipelineLayout
            computePipeline_createInfo.layout = m_pipelineLayout;
            // VkPipelineShaderStageCreateInfo
            computePipeline_createInfo.stage = computeShader_pipelineShaderStageCreateInfo;
            computePipeline_createInfo.basePipelineHandle = VK_NULL_HANDLE;
            computePipeline_createInfo.basePipelineIndex = 0;

            // m_pipeline: compute_pipeline
            res = vkCreateComputePipelines(pDevice->GetDevice(), pDevice->GetPipelineCache(), 1, &computePipeline_createInfo, NULL, &m_pipeline);
            assert(res == VK_SUCCESS);
            //
        }
        //

        //SetResourceName

        //
    }

    void ParticleCS::OnDestroy()
    {
        vkDestroyPipeline(m_pDevice->GetDevice(), m_pipeline, nullptr);
        vkDestroyPipelineLayout(m_pDevice->GetDevice(), m_pipelineLayout, nullptr);

        //
        /*
        if (m_shaderStage.module != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(m_pDevice->GetDevice(), m_shaderStage.module, NULL);
        }
        m_shaderStage = VkPipelineShaderStageCreateInfo{};
        */
        //
    }

    void ParticleCS::Draw(VkCommandBuffer cmd_buf
        , std::vector<VkDescriptorSet>& array_storageDescriptorSet
        , VkDescriptorBufferInfo* pConstantBuffer, VkDescriptorSet constantDescriptorSet
        , uint32_t dispatchX, uint32_t dispatchY, uint32_t dispatchZ)
    {
        if (m_pipeline == VK_NULL_HANDLE)
            return;

        // Bind Descriptor sets
        //         
        uint32_t numStorageDescriptorSet = (uint32_t)array_storageDescriptorSet.size();
        if (numStorageDescriptorSet > 0)
        {
            vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout
                , 0, numStorageDescriptorSet, array_storageDescriptorSet.data()
                , 0, NULL);
        }

        uint32_t numUniformOffsets = 0;
        uint32_t uniformOffset = 0;
        // Fault of MSVC: Combine all result not one by one
        if (pConstantBuffer != NULL && pConstantBuffer->buffer != VK_NULL_HANDLE
            && constantDescriptorSet != VK_NULL_HANDLE)
        {
            numUniformOffsets = 1;
            uniformOffset = (uint32_t)pConstantBuffer->offset;
        }
        
        if (numUniformOffsets > 0)
        {
            VkDescriptorSet descritorSets[1] = { constantDescriptorSet };
            vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout
                , numStorageDescriptorSet, 1, descritorSets
                , numUniformOffsets, &uniformOffset);
        }

        // Bind Pipeline
        //
        vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);

        // Draw
        //
        // vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX,uint32_t groupCountY,uint32_t groupCountZ)
        vkCmdDispatch(cmd_buf, dispatchX, dispatchY, dispatchZ);
        // void vkCmdDispatchIndirect(VkCommandBuffer commandBuffer,    VkBuffer buffer,VkDeviceSize offset);// buffer containing dispatch parameters, buffer containing dispatch parameters
        // vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX,uint32_t baseGroupY,uint32_t baseGroupZ,    uint32_t groupCountX,uint32_t groupCountY,uint32_t groupCountZ);
        // vkCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
        /*
        layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

        layout (set = 0, binding = 0) buffer InputBuffer {
            int inputData[];
        } inputBuffer;
        // baseGroupX|Y|Z<->gl_WorkGroupSize.x|y|z => gl_BaseInstance;    groupCountX|Y|Z<->gl_NumWorkGroups.x|y|z
        uint globalID = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;
        uint localID = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
        uint groupID = gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x;
        uint baseGroupID = gl_BaseInstance / gl_WorkGroupSize.x;
        uint index = (groupID + baseGroupID) * gl_WorkGroupSize.x * gl_WorkGroupSize.y + localID;
        inputBuffer.inputData[index] = index;
        */
        // 

    }

}
