

#include "stdafx.h"
#include "Base/ExtDebugUtils.h"
#include "Base/DynamicBufferRing.h"
#include "Base/StaticBufferPool.h"
#include "Base/ResourceViewHeaps.h"
#include "Base/ShaderCompilerHelper.h"
#include "Base/UploadHeap.h"
#include "Base/Texture.h"
#include "Misc/ThreadPool.h"

#include "ParticlePS.h"

namespace CAULDRON_VK
{
    //--------------------------------------------------------------------------------------
    //
    // OnCreate
    //
    //--------------------------------------------------------------------------------------    
    void ParticlePS::OnCreate(
        Device* pDevice,
        VkRenderPass renderPass,
        //const std::string& shaderFilename,
        //const std::string& shaderEntryPoint,
        //const std::string& shaderCompilerParams,
        std::array<uint32_t,3> numDimensions,
        ShaderCompileParameter vS_particlePsComplieParameter,
        ShaderCompileParameter fS_particlePsComplieParameter,
        ParticleVertexInputStateParameter* pParticleVertexInputStateParameter,
        //StaticBufferPool* pStaticBufferPool,
        StaticBufferAllocater* pStaticBufferAllocater,
        //DynamicBufferRing* pDynamicBufferRing,
        DynamicBufferAllocater* pDynamicBufferAllocater,
        //VkDescriptorSetLayout descriptorSetLayout,
        std::vector<VkDescriptorSetLayout>& array_descriptorSetLayout,
        VkPipelineColorBlendStateCreateInfo* pBlendDesc,
        VkSampleCountFlagBits sampleDescCount
    )
    {
        m_pDevice = pDevice;

        /*
        // Create the vertex shader
        static const char* vertexShader =
            "static const float4 FullScreenVertsPos[3] = { float4(-1, 1, 1, 1), float4(3, 1, 1, 1), float4(-1, -3, 1, 1) };\
            static const float2 FullScreenVertsUVs[3] = { float2(0, 0), float2(2, 0), float2(0, 2) };\
            struct VERTEX_OUT\
            {\
                float2 vTexture : TEXCOORD;\
                float4 vPosition : SV_POSITION;\
            };\
            VERTEX_OUT mainVS(uint vertexId : SV_VertexID)\
            {\
                VERTEX_OUT Output;\
                Output.vPosition = FullScreenVertsPos[vertexId];\
                Output.vTexture = FullScreenVertsUVs[vertexId];\
                return Output;\
            }";
        */

        VkResult res;

        // Compile shaders
        //
        DefineList attributeDefines;
        attributeDefines["WIDTH"] = std::to_string(numDimensions[0]);
        attributeDefines["HEIGHT"] = std::to_string(numDimensions[1]);
        attributeDefines["DEPTH"] = std::to_string(numDimensions[2]);

        //VKCompileFromString()
        /*
        VkPipelineShaderStageCreateInfo m_vertexShader;
#ifdef _DEBUG
        std::string CompileFlagsVS("-T vs_6_0 -Zi -Od");
#else
        std::string CompileFlagsVS("-T vs_6_0");
#endif // _DEBUG
        res = VKCompileFromString(m_pDevice->GetDevice(), SST_HLSL, VK_SHADER_STAGE_VERTEX_BIT, vertexShader, "mainVS", CompileFlagsVS.c_str(), &attributeDefines, &m_vertexShader);
        assert(res == VK_SUCCESS);
        */


        //VertexShader
        //  VKCompileFromFile()
        //m_vertexShaderName = std::string("main");
        m_vertexShaderName = std::string(vS_particlePsComplieParameter.pShaderEntryPoint);
        //VkPipelineShaderStageCreateInfo m_vertexShader;
        VkPipelineShaderStageCreateInfo vertexShader;
        //res = VKCompileFromFile(m_pDevice->GetDevice(), VK_SHADER_STAGE_VERTEX_BIT, "ParticlPS_VS.glsl", "main", "", &attributeDefines, &m_vertexShader);
        res = VKCompileFromFile(m_pDevice->GetDevice(), VK_SHADER_STAGE_VERTEX_BIT, vS_particlePsComplieParameter.pShaderFilename, vS_particlePsComplieParameter.pShaderEntryPoint, vS_particlePsComplieParameter.pShaderCompilerParams
            , &attributeDefines, &vertexShader);
        assert(res == VK_SUCCESS);

        //FragmentShader
        //  VKCompileFromFile()
        //m_fragmentShaderName = shaderEntryPoint;
        m_fragmentShaderName = std::string(fS_particlePsComplieParameter.pShaderEntryPoint);
        VkPipelineShaderStageCreateInfo fragmentShader;
        //res = VKCompileFromFile(m_pDevice->GetDevice(), VK_SHADER_STAGE_FRAGMENT_BIT, shaderFilename.c_str(), m_fragmentShaderName.c_str(), shaderCompilerParams.c_str(), &attributeDefines, &m_fragmentShader);
        res = VKCompileFromFile(m_pDevice->GetDevice(), VK_SHADER_STAGE_FRAGMENT_BIT, fS_particlePsComplieParameter.pShaderFilename, fS_particlePsComplieParameter.pShaderEntryPoint, fS_particlePsComplieParameter.pShaderCompilerParams
            , &attributeDefines, &fragmentShader);
        assert(res == VK_SUCCESS);

        m_shaderStages.clear();
        //m_shaderStages.push_back(m_vertexShader);
        m_shaderStages.push_back(vertexShader);
        //m_shaderStages.push_back(m_fragmentShader);
        m_shaderStages.push_back(fragmentShader);

        // Create pipeline layout
        //
        VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
        pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pPipelineLayoutCreateInfo.pNext = NULL;
        pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
        //pPipelineLayoutCreateInfo.setLayoutCount = 1;
        pPipelineLayoutCreateInfo.setLayoutCount = (uint32_t)array_descriptorSetLayout.size();
        //pPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
        pPipelineLayoutCreateInfo.pSetLayouts = array_descriptorSetLayout.data();

        res = vkCreatePipelineLayout(pDevice->GetDevice(), &pPipelineLayoutCreateInfo, NULL, &m_pipelineLayout);
        assert(res == VK_SUCCESS);

        //UpdatePipeline(renderPass, pBlendDesc, sampleDescCount);
        UpdatePipeline(renderPass, pParticleVertexInputStateParameter, pBlendDesc, sampleDescCount);
        //

        //SetResourceName

    }

    //--------------------------------------------------------------------------------------
    //
    // UpdatePipeline
    //
    //--------------------------------------------------------------------------------------
    void ParticlePS::UpdatePipeline(VkRenderPass renderPass, ParticleVertexInputStateParameter* pParticleVertexInputStateParameter
        ,VkPipelineColorBlendStateCreateInfo* pBlendDesc, VkSampleCountFlagBits sampleDescCount)
    {
        if (renderPass == VK_NULL_HANDLE)
            return;

        if (m_pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(m_pDevice->GetDevice(), m_pipeline, nullptr);
            m_pipeline = VK_NULL_HANDLE;
        }

        VkResult res;

        // input assembly state and layout

        //VkPipelineVertexInputStateCreateInfo
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineVertexInputStateCreateInfo.html
        VkPipelineVertexInputStateCreateInfo vi = {};
        vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vi.pNext = NULL;
        vi.flags = 0;
        //
        vi.vertexBindingDescriptionCount = (uint32_t)pParticleVertexInputStateParameter->array_vertexInputBindingDescription.size();
        // return NULL|nullptr if size()==0
        vi.pVertexBindingDescriptions = pParticleVertexInputStateParameter->array_vertexInputBindingDescription.data();
        //
        vi.vertexAttributeDescriptionCount = pParticleVertexInputStateParameter->array_vertexInputAttributeDescription.size();
        // return NULL|nullptr if size()==0
        vi.pVertexAttributeDescriptions = pParticleVertexInputStateParameter->array_vertexInputAttributeDescription.data();
        /*
        vi.vertexBindingDescriptionCount = 0;
        vi.pVertexBindingDescriptions = nullptr;
        vi.vertexAttributeDescriptionCount = 0;
        vi.pVertexAttributeDescriptions = nullptr;
        */

        //VkPipelineInputAssemblyStateCreateInfo
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineInputAssemblyStateCreateInfo.html
        VkPipelineInputAssemblyStateCreateInfo ia;
        ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        ia.pNext = NULL;
        ia.flags = 0;
        //controls whether a special vertex index value is treated as restarting the assembly of primitives. 
        //  This enable only applies to indexed draws (vkCmdDrawIndexed, vkCmdDrawMultiIndexedEXT, and vkCmdDrawIndexedIndirect)
        ia.primitiveRestartEnable = VK_FALSE;
        //ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        ia.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

        // rasterizer state

        //VkPipelineRasterizationStateCreateInfo
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineRasterizationStateCreateInfo.html
        VkPipelineRasterizationStateCreateInfo rs;
        rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rs.pNext = NULL;
        rs.flags = 0;
        //  VK_POLYGON_MODE_FILL,VK_POLYGON_MODE_POINT,  VK_POLYGON_MODE_LINE ,VK_POLYGON_MODE_FILL_RECTANGLE_NV
        rs.polygonMode = VK_POLYGON_MODE_FILL;
        //  NONE,    FRONT,BACK,FRONT_AND_BACK
        rs.cullMode = VK_CULL_MODE_NONE;
        //  COUNTER_CLOCKWISE=0,  CLOCKWISE
        rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        //
        rs.depthClampEnable = VK_FALSE;
        rs.rasterizerDiscardEnable = VK_FALSE;
        //  depthBias
        rs.depthBiasEnable = VK_FALSE;
        rs.depthBiasConstantFactor = 0;
        rs.depthBiasClamp = 0;
        rs.depthBiasSlopeFactor = 0;
        //  Width of rasterized line segments.
        rs.lineWidth = 1.0f;

        //No Exist Depth-AttachmentState, Since Only one Depth-Attachment xor Empty

        //VkPipelineColorBlendAttachmentState
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineColorBlendAttachmentState.html
        VkPipelineColorBlendAttachmentState att_state[1];
        // 
        att_state[0].colorWriteMask = 0xf;
        //
        /*
        // Altered
        att_state[0].blendEnable = VK_TRUE;
        //
        att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
        //att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        //att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        //
        att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
        //att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        //att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
        att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        */
        //  AlphaBlend, ColorBlend
        /*
        att_state[0].blendEnable = VK_FALSE;
        att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
        att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
        att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        */
        att_state[0].blendEnable = VK_FALSE;
        att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
        att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
        att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        /*
        */

        // Color blend state

        //VkPipelineColorBlendStateCreateInfo
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineColorBlendStateCreateInfo.html
        VkPipelineColorBlendStateCreateInfo cb;
        cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        cb.flags = 0;
        cb.pNext = NULL;
        // Delta: Should match the attachmentCount of Pipeline
        cb.attachmentCount = 1;
        cb.pAttachments = att_state;
        //
        cb.logicOpEnable = VK_FALSE;
        //VkLogicOp: NO,AND,OR,CLEAR,...
        cb.logicOp = VK_LOGIC_OP_NO_OP;
        //
        cb.blendConstants[0] = 1.0f;
        cb.blendConstants[1] = 1.0f;
        cb.blendConstants[2] = 1.0f;
        cb.blendConstants[3] = 1.0f;

        //VkDynamicState
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDynamicState.html
        std::vector<VkDynamicState> dynamicStateEnables = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            //VK_DYNAMIC_STATE_BLEND_CONSTANTS
        };
        //VkPipelineDynamicStateCreateInfo
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineDynamicStateCreateInfo.html
        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pNext = NULL;
        dynamicState.flags = 0;
        dynamicState.dynamicStateCount = (uint32_t)dynamicStateEnables.size();
        dynamicState.pDynamicStates = dynamicStateEnables.data();

        // view port state

        //VkPipelineViewportStateCreateInfo
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineViewportStateCreateInfo.html
        VkPipelineViewportStateCreateInfo vp = {};
        vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        vp.pNext = NULL;
        vp.flags = 0;
        // If the multiViewport feature is not enabled, viewportCount must not be greater than 1
        vp.viewportCount = 1;
        // If the multiViewport feature is not enabled, scissorCount must not be greater than 1
        vp.scissorCount = 1;
        //
        vp.pScissors = NULL;
        vp.pViewports = NULL;

        // depth stencil state

        //VkPipelineDepthStencilStateCreateInfo
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineDepthStencilStateCreateInfo.html
        VkPipelineDepthStencilStateCreateInfo ds;
        ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        ds.pNext = NULL;
        ds.flags = 0;
        //
        /*
        ds.depthTestEnable = VK_FALSE;
        ds.depthWriteEnable = VK_FALSE;
        ds.depthCompareOp = VK_COMPARE_OP_ALWAYS;
        ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
        */
        //
        /*
        ds.depthTestEnable = VK_TRUE;
        ds.depthWriteEnable = VK_FALSE;
        ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        ds.depthBoundsTestEnable = VK_FALSE;
        ds.stencilTestEnable = VK_FALSE;
        ds.back.failOp = VK_STENCIL_OP_KEEP;
        ds.back.passOp = VK_STENCIL_OP_KEEP;
        ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
        ds.back.compareMask = 0;
        ds.back.reference = 0;
        ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
        ds.back.writeMask = 0;
        ds.minDepthBounds = 0;
        ds.maxDepthBounds = 0;
        ds.stencilTestEnable = VK_FALSE;
        ds.front = ds.back;
        */
        //
        ds.depthTestEnable = VK_TRUE;
        ds.depthWriteEnable = VK_TRUE;
        ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        ds.back.failOp = VK_STENCIL_OP_KEEP;
        ds.back.passOp = VK_STENCIL_OP_KEEP;
        ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
        ds.back.compareMask = 0;
        ds.back.reference = 0;
        ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
        ds.back.writeMask = 0;
        ds.depthBoundsTestEnable = VK_FALSE;
        ds.minDepthBounds = 0;
        ds.maxDepthBounds = 0;
        ds.stencilTestEnable = VK_FALSE;
        ds.front = ds.back;
        /*
        */

        // multi sample state

        //VkPipelineMultisampleStateCreateInfo
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineMultisampleStateCreateInfo.html
        VkPipelineMultisampleStateCreateInfo ms;
        ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        ms.pNext = NULL;
        ms.flags = 0;
        ms.pSampleMask = NULL;
        ms.rasterizationSamples = sampleDescCount;
        ms.sampleShadingEnable = VK_FALSE;
        ms.alphaToCoverageEnable = VK_FALSE;
        ms.alphaToOneEnable = VK_FALSE;
        ms.minSampleShading = 0.0;

        // create pipeline 

        //
        VkGraphicsPipelineCreateInfo pipeline = {};
        pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline.pNext = NULL;
        pipeline.layout = m_pipelineLayout;
        pipeline.basePipelineHandle = VK_NULL_HANDLE;
        pipeline.basePipelineIndex = 0;
        pipeline.flags = 0;
        pipeline.pVertexInputState = &vi;
        pipeline.pInputAssemblyState = &ia;
        pipeline.pRasterizationState = &rs;
        pipeline.pColorBlendState = (pBlendDesc == NULL) ? &cb : pBlendDesc;
        pipeline.pTessellationState = NULL;
        pipeline.pMultisampleState = &ms;
        pipeline.pDynamicState = &dynamicState;
        pipeline.pViewportState = &vp;
        pipeline.pDepthStencilState = &ds;
        pipeline.stageCount = (uint32_t)m_shaderStages.size();
        pipeline.pStages = m_shaderStages.data();
        pipeline.renderPass = renderPass;
        // index 
        pipeline.subpass = 0;

        res = vkCreateGraphicsPipelines(m_pDevice->GetDevice(), m_pDevice->GetPipelineCache(), 1, &pipeline, NULL, &m_pipeline);
        assert(res == VK_SUCCESS);
        SetResourceName(m_pDevice->GetDevice(), VK_OBJECT_TYPE_PIPELINE, (uint64_t)m_pipeline, "PaticlePS");
    }

    //--------------------------------------------------------------------------------------
    //
    // OnDestroy
    //
    //--------------------------------------------------------------------------------------    
    void ParticlePS::OnDestroy()
    {
        if (m_pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(m_pDevice->GetDevice(), m_pipeline, nullptr);
            m_pipeline = VK_NULL_HANDLE;
        }

        if (m_pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(m_pDevice->GetDevice(), m_pipelineLayout, nullptr);
            m_pipelineLayout = VK_NULL_HANDLE;
        }

        //
        /*
        for (auto& ref : m_shaderStages)
        {
            if (ref.module != VK_NULL_HANDLE)
            {
                vkDestroyShaderModule(m_pDevice->GetDevice(), ref.module, NULL);
            }
        }
        m_shaderStages.clear();
        */
        //
    }

    //--------------------------------------------------------------------------------------
    //
    // OnDraw
    //
    //--------------------------------------------------------------------------------------    
    void ParticlePS::Draw(VkCommandBuffer cmd_buf
        , VkDescriptorBufferInfo* pConstantBuffer, VkDescriptorSet constantDescriptorSet
        , VkDescriptorBufferInfo* pVertexBuffer, uint32_t numVertex)
    {
        if (m_pipeline == VK_NULL_HANDLE)
            return;

        // Bind VertexBufer
        vkCmdBindVertexBuffers(cmd_buf, 0, 1, &pVertexBuffer->buffer, &pVertexBuffer->offset);

        // Bind Descriptor sets
        //                
        int numUniformOffsets = 0;
        uint32_t uniformOffset = 0;
        if (pConstantBuffer != NULL && pConstantBuffer->buffer != NULL)
        {
            numUniformOffsets = 1;
            uniformOffset = (uint32_t)pConstantBuffer->offset;
        }

        if (numUniformOffsets > 0)
        {
            VkDescriptorSet descritorSets[1] = { constantDescriptorSet };
            vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout
                , 0, 1
                , descritorSets
                , numUniformOffsets, &uniformOffset);
        }

        //
        
        // Bind Pipeline
        //
        vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

        // Draw
        //
        //vkCmdDraw(cmd_buf, 3, 1, 0, 0);
        vkCmdDraw(cmd_buf, numVertex, 1, 0, 0);

    }
}