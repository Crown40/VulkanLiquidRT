
#pragma once


#include "../../libs/vectormath/vectormath.hpp"
#include "Misc/Camera.h"
//#include "Base/ResourceViewHeaps.h"
#include "Base/GBuffer.h"
//#include "TAA.h"
#include "ParticlePS.h"
#include "ParticleCS.h"
#include "PhysicsRT.h"

namespace CAULDRON_VK
{
    struct RenderParticlePSUniform
    {
        math::Matrix4 model;
        math::Matrix4 viewProjection;
    };

    class FluidParticleCS
    {
    public:
        //ParticlePropertyComponentAlign* m_particlePosPointer;
        //ParticlePropertyComponentAlign* m_particleVlcPointer;
        //ParticlePropertyComponentAlign* m_particleAccPointer;
        //void OnCreate(Device* pDevice, ResourceViewHeaps* pResourceViewHeaps, StaticBufferPool* pStaticBufferPool, DynamicBufferRing* pDynamicBufferRing, bool sharpening = true);
        void OnCreate(Device* pDevice, uint32_t numParticle, std::array<uint32_t, 3> numDimensions
            ,VkRenderPass renderPass
            //, ResourceViewHeaps* pResourceViewHeaps
            , HeapDescriptorSetAllocater* pDescriptorSetHeap
            //, StaticBufferPool* pStaticBufferPool, DynamicBufferRing* pDynamicBufferRing
            , DynamicBufferAllocater* pConstBufferDynamic_CPU2GPU, DynamicBufferAllocater* pRtBufferDynamic_GPU
            , StaticBufferAllocater* pStorageBufferStatic_GPU, StaticBufferAllocater* pRtBufferStatic_GPU
        );
        void OnDestroy();

        //void OnCreateWindowSizeDependentResources(uint32_t Width, uint32_t Height, GBuffer* pGBuffer);
        //void OnDestroyWindowSizeDependentResources();

        //void Draw(VkCommandBuffer cmd_buf);
        void DrawGraphics(VkCommandBuffer cmd_buf, const Camera& Cam);
        void DrawCompute(VkCommandBuffer cmd_buf, const Camera& Cam);
        //void BuildBatchList();
        //void DrawBatchList();
        
        //
        math::Matrix4 GetCurrentWolrdMat4() { return m_currentWolrdMat4; };
        math::Matrix4 GetPreviousWolrdMat4() { return m_previousWolrdMat4; };
        bool        GetbFirst() { return m_bFirst; }
        VkSemaphore GetGraphicsSemaphore() { return m_graphicsSemaphore; }
        VkSemaphore GetComputeSemaphore() { return m_computeSemaphore; }
        //

        //
        void OnCreateUniformDescriptorSet(uint32_t sizeByte
            , VkDescriptorSetLayout* pDescriptorSetLayout, VkDescriptorSet* pDescriptorSet
            , std::vector<VkDescriptorSetLayoutBinding>& array_descriptorSetLayoutBinding);
        void OnCreateParticleBuffer(ParticleData& particleData, uint32_t numParticle, uint32_t numProperty);

    private:
        //
        Device*                 m_pDevice;
        //ResourceViewHeaps*      m_pResourceViewHeaps;
        //StaticBufferPool*       m_pStaticBufferPool;
        //DynamicBufferRing*      m_pDynamicBufferRing;
        //
        HeapDescriptorSetAllocater* m_pDescriptorSetHeap;
        DynamicBufferAllocater* m_pConstBufferDynamic_CPU2GPU;
        DynamicBufferAllocater* m_pRtBufferDynamic_GPU;
        StaticBufferAllocater*  m_pStorageBufferStatic_GPU;
        StaticBufferAllocater*  m_pRtBufferStatic_GPU;

        //
        
        uint32_t                m_numPartilce;
        std::array<uint32_t, 3> m_numDimensions;
        ParticleData            m_particleData;
        //VkRenderPass            m_renderParticleRenderPass;
        ParticlePS              m_renderParticlePS;
        ParticleCS              m_mainParticleCS;
        bool                    m_bFirst = true;
        ParticleCS              m_firstParticleCS;
        PhysicsRT               m_physicsRT;
        //
        math::Matrix4           m_currentWolrdMat4;
        math::Matrix4           m_previousWolrdMat4;
        VkDescriptorSetLayout   m_uniformDescriptorSetLayout_particlePS;
        VkDescriptorSet         m_uniformDescriptorSet_particlePS;
        VkDescriptorSetLayout   m_uniformDescriptorSetLayout_particleCS;
        VkDescriptorSet         m_uniformDescriptorSet_particleCS;
        //
        VkSemaphore             m_graphicsSemaphore;
        VkSemaphore             m_computeSemaphore;
        
        //
        
    };
}

