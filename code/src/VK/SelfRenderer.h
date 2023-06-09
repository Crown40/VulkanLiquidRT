
#pragma once

#include "stdafx.h"

//#include "base/Device.h"
#include "base/GBuffer.h"
#include "PostProc/MagnifierPS.h"
#include "HeapDescriptorSetAllocater.h"
#include "DynamicBufferAllocater.h"
#include "StaticBufferAllocater.h"
#include "FluidParticleCS.h"

// We are queuing (backBufferCount + 0.5) frames, so we need to triple buffer the resources that get modified each frame
static const int self_backBufferCount = 3;  //  3;

using namespace CAULDRON_VK;

struct UIState;

//
// Renderer class is responsible for rendering resources management and recording command buffers.
class SelfRenderer
{
public:
    void OnCreate(Device* pDevice, SwapChain* pSwapChain, float FontSize);
    void _OnCreate(Device* pDevice, SwapChain* pSwapChain, float FontSize);
    void OnDestroy();
    void _OnDestroy();

    void OnCreateWindowSizeDependentResources(SwapChain* pSwapChain, uint32_t Width, uint32_t Height);
    void OnDestroyWindowSizeDependentResources();

    void OnUpdateDisplayDependentResources(SwapChain* pSwapChain, bool bUseMagnifier);
    void OnUpdateLocalDimmingChangedResources(SwapChain* pSwapChain);

    int LoadScene(GLTFCommon* pGLTFCommon, int Stage = 0);
    void UnloadScene();

    void AllocateShadowMaps(GLTFCommon* pGLTFCommon);

    const std::vector<TimeStamp>& GetTimingValues() { return m_TimeStamps; }

    void OnRender(const UIState* pState, const Camera& Cam, SwapChain* pSwapChain);

private:
    Device* m_pDevice;

    uint32_t                        m_Width;
    uint32_t                        m_Height;

    VkRect2D                        m_RectScissor;
    VkViewport                      m_Viewport;

    // Initialize helper classes
    // Descriptor Pool For Descriptor Set Layouts(DSL) and Descriptor
    ResourceViewHeaps               m_ResourceViewHeaps;
    // Upload resources to the GPU memory: For convenience this class comes with it's own command list & submit (FlushAndFinish)
    UploadHeap                      m_UploadHeap;
    // Dynamice Allocation need to allocate Buffer Each-Frame for Usage and limited-GPUmem
    // Fake-Allocate
    // 'dynamic' constant buffer: ConstantBuffer: 'Uniforms': 可以使用 Vulkan Memory Allocator(VMA):一个开源的专门为Vulkan API设计的内存分配器库{-> VmaAllocator}
    // &m_ConstantBufferRing would be Passed 2 Imgui,PostProcess,Gltf,... by calling Alloc...() under Draw(),SetPerFrameConstants()
    DynamicBufferRing               m_ConstantBufferRing;
    // Static Allocation only allocate Once{Mose at OnCreate(),Load...(),...}
    // Fake-Allocate
    // 2560size of Bytes
    // 'static' pool for vertices and indices{Yes: ordinary Geometries}: VidMemBuffer: 'StaticGeom'
    StaticBufferPool                m_VidMemBufferPool;
    // 'static' pool for vertices and indices in system memory{Yes: but no-used Actually}: SysMemBuffer: 'PostProcGeom'
    StaticBufferPool                m_SysMemBufferPool;
    //
    
    //DescriptorSet: Heap
    HeapDescriptorSetAllocater      m_DescriptorSetHeap;
    //Buffer: Dynamic
    //  For Const:Uniform|Vertex|Index   to Pass: {See Imgui::Draw() at row-605]
    DynamicBufferAllocater          m_ConstBufferDynamic_CPU2GPU;
    //  For AccelerationStructure-Build/Update{Scratch} | Staging-Buffer{Storage-buffer}
    //  Only be used For Build|Update AS with reset-buffer
    DynamicBufferAllocater          m_RtBufferDynamic_GPU;
    //Buffer: Static
    //  Maybe Merge m_PrimitiveBufferStatic_GPU and m_OnlyStorageBufferStatic_GPU one day
    StaticBufferAllocater           m_StorageBufferStatic_GPU;
    //StaticBufferAllocater           m_PrimitiveBufferStatic_GPU;
    //StaticBufferAllocater           m_OnlyStorageBufferStatic_GPU;
    //  MayFor InstanceBuffer,AABB,...  {Overlap Allocate?}
    StaticBufferAllocater           m_RtBufferStatic_GPU;
    //
    FluidParticleCS                 m_FluidParticeCS_SPH;
    //

    //Compute
    CommandListRing                 m_ComputeCommandListRing;
    //Graphics
    CommandListRing                 m_CommandListRing;
    GPUTimestamps                   m_GPUTimer;

    //gltf passes
    GltfPbrPass*                    m_GLTFPBR;
    GltfBBoxPass*                   m_GLTFBBox;
    GltfDepthPass*                  m_GLTFDepth;
    GLTFTexturesAndBuffers*         m_pGLTFTexturesAndBuffers;

    // effects
    Bloom                           m_Bloom;
    SkyDome                         m_SkyDome;
    DownSamplePS                    m_DownSample;
    SkyDomeProc                     m_SkyDomeProc;
    ToneMapping                     m_ToneMappingPS;
    ToneMappingCS                   m_ToneMappingCS;
    ColorConversionPS               m_ColorConversionPS;
    TAA                             m_TAA;
    MagnifierPS                     m_MagnifierPS;
    bool                            m_bMagResourceReInit = false;

    // GUI
    ImGUI                           m_ImGUI;

    // GBuffer and render passes
    GBuffer                         m_GBuffer;
    GBufferRenderPass               m_RenderPassFullGBufferWithClear;
    GBufferRenderPass               m_RenderPassJustDepthAndHdr;
    GBufferRenderPass               m_RenderPassFullGBuffer;

    // shadowmaps
    VkRenderPass                    m_Render_pass_shadow;

    typedef struct {
        Texture         ShadowMap;
        uint32_t        ShadowIndex;
        uint32_t        ShadowResolution;
        uint32_t        LightIndex;
        VkImageView     ShadowDSV;
        VkFramebuffer   ShadowFrameBuffer;
    } SceneShadowInfo;

    std::vector<SceneShadowInfo>    m_shadowMapPool;
    std::vector< VkImageView>       m_ShadowSRVPool;

    // widgets
    Wireframe                       m_Wireframe;
    WireframeBox                    m_WireframeBox;

    std::vector<TimeStamp>          m_TimeStamps;

    AsyncPool                       m_AsyncPool;
    
};
