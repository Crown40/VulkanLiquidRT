
#include "SelfRenderer.h"
#include "UI.h"

//--------------------------------------------------------------------------------------
//
// OnCreate
//
//--------------------------------------------------------------------------------------
void SelfRenderer::OnCreate(Device* pDevice, SwapChain* pSwapChain, float FontSize)
{
	m_pDevice = pDevice;

    // Create all the heaps for the resources views
    //ResourceViewHeaps
    // m_ResourceViewHeaps.m_descriptorPool的 poolSizeCount{bufferTypes_Count} 和 pPoolSizes{bufferType,bufferTypeCount}
    // Delta: VkDescriptorPoolSize{type:enum, descriptorCount:uint32_t[对应type的Descriptor的Count]} []={{},...,{}} => vkCreateDescriptorPool
    //          descriptorCount>= descriptorAmountOf 'VkWriteDescriptorSet'[单个描述符集合中的描述符绑定] + descriptorAmountOf 'VkCopyDescriptorSet' + descriptorAmountOf 'VkUpdateDescriptorSets'[同时批量更新多个描述符集合中的描述符绑定]
    //  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC|VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    const uint32_t cbvDescriptorCount = 2000;
    //  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    const uint32_t srvDescriptorCount = 8000;
    //  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    const uint32_t uavDescriptorCount = 10;
    //  VK_DESCRIPTOR_TYPE_SAMPLER
    const uint32_t samplerDescriptorCount = 20;
    // create a DescriptorPool and allows **allocating, freeing and initializing DescriptorSetLayouts(DSL)** from this pool. 
    m_ResourceViewHeaps.OnCreate(pDevice, cbvDescriptorCount, srvDescriptorCount, uavDescriptorCount, samplerDescriptorCount);
    //
    //
    // 从 m_ResourceViewHeaps的DescriptorPool 用 **一|多个** VkDescriptorSetLayoutBinding{binding,descriptorType,descriptorCount,stageFlags,pImmutableSamplers}
    // 创建{Create}的 **一个[|多个]** VkDescriptorSetLayout{bindingCount,pBindings:VkDescriptorSetLayoutBinding*}
    // 申请分配{Allocate} **一|多个** VkDescriptorSet
    // 因此在一个 VkDescriptorSetLayout 中，每个描述符绑定的 binding 必须是唯一的。   VkDescriptorSetLayoutBinding 数组中的各个元素的 binding 字段不能相同。如果有多个 VkDescriptorSetLayoutBinding 结构体的 binding 字段相同，则会导致 Vulkan API 的行为不可预测。因此，在创建 VkDescriptorSetLayout 时，应该确保每个描述符绑定在描述符集合中的绑定点索引 binding 都是唯一的。
    /*
    // Create 一个 DescriptorPool: m_descriptorPool
    void OnCreate(Device *pDevice, uint32_t cbvDescriptorCount, uint32_t srvDescriptorCount, uint32_t uavDescriptorCount, uint32_t samplerDescriptorCount);
    // Destroy m_descriptorPool
    void OnDestroy();
    // 用 VkDescriptorSetLayout 从 m_descriptorPool 申请分配 一个[|多个] VkDescriptorSet
    bool AllocDescriptor(VkDescriptorSetLayout descriptorLayout, VkDescriptorSet *pDescriptor);
    //
    // vector<> **size个{一|多}** 带VkSampler* 的VkDescriptorSetLayoutBinding数组{1个descriptorCount} 再 Create 一个 VkDescriptorSetLayout 来从 m_descriptorPool 申请Allocate 一个 VkDescriptorSet
    // -> CreateDescriptorSetLayoutAndAllocDescriptorSet() -> AllocDescriptor(VkDescriptorSetLayout, VkDescriptorSet*) 上
    bool AllocDescriptor(int size, const VkSampler *pSamplers, VkDescriptorSetLayout *descriptorLayout, VkDescriptorSet *pDescriptor);
    // vector<> **descriptorCounts.size()个{一|多}** 带VkSampler* 的VkDescriptorSetLayoutBinding数组{1|多个 不同的descriptorCount} 再 Create 一个 VkDescriptorSetLayout 来从 m_descriptorPool 申请Allocate 一个 VkDescriptorSet
    bool AllocDescriptor(std::vector<uint32_t> &descriptorCounts, const VkSampler* pSamplers, VkDescriptorSetLayout* descriptorLayout, VkDescriptorSet* pDescriptor);
    //
    // Create pDescriptorLayoutBinding->size()个 VkDescriptorSetLayoutBinding的 一个 VkDescriptorSetLayout
    // **vkCreateDescriptorSetLayout 一次只能 Create 一个 VkDescriptorSetLayout**
    bool CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> *pDescriptorLayoutBinding, VkDescriptorSetLayout *pDescSetLayout);
    // CreateDescriptorSetLayoutAndAllocDescriptorSet() -> AllocDescriptor(VkDescriptorSetLayout, VkDescriptorSet*)
    bool CreateDescriptorSetLayoutAndAllocDescriptorSet(std::vector<VkDescriptorSetLayoutBinding> *pDescriptorLayoutBinding, VkDescriptorSetLayout *descriptorLayout, VkDescriptorSet *pDescriptor);
    //
    // Free VkDescriptorSet
    void FreeDescriptor(VkDescriptorSet descriptorSet);
    */



    // Create a commandlist ring for the Direct queue
    //Delta
    //CommandListRing: OnBeginFrame
    //commandListsPerBackBuffer: CommandBuffersPerFrame 
    uint32_t commandListsPerBackBuffer = 8;
    // onCreate|Creation allocates a number of command lists. 
    // Using a ring buffer{环形缓存，首尾相连}， these commandLists are recycled when they are no longer used by the GPU. 
    // See the 'ring.h' for more details on allocation and recycling
    //
    //Delta: frameCountInFlight:backBufferCount==3: 
    // queuing (backBufferCount + 0.5) frames, so we need to triple buffer the resources that get modified each frame
    m_CommandListRing.OnCreate(pDevice, self_backBufferCount, commandListsPerBackBuffer);

    // Create a 'dynamic' constant buffer
    //Delta
    //DynamicBufferRing: OnBeginFrame
    const uint32_t constantBuffersMemSize = 200 * 1024 * 1024;
    // This class mimics the behaviour or the DX11 dynamic buffers. I **can hold uniforms, index and vertex buffers**.
    // It does so **by suballocating memory from a huge buffer**. The buffer is used in a ring fashion.  
    // Allocated memory is taken from the tail, freed memory makes the head advance;
    // See 'ring.h' to get more details on the ring buffer.
    //
    // The class knows when to free memory by just knowing:
    //    1) the amount of memory used per frame
    //    2) the number of backbuffers 
    //    3) **When a new frame just started ( indicated by OnBeginFrame() )**
    //         - This will free the data of the oldest frame so it can be reused for the new frame
    //
    // Note than in this ring an allocated chuck of memory has to be contiguous in memory, that is it cannot spawn accross the tail and the head.
    // This class takes care of that.
    //For 'Uniforms'
    //TODO: Annotate
    m_ConstantBufferRing.OnCreate(pDevice, self_backBufferCount, constantBuffersMemSize, "Uniforms");

    // Create a 'static' pool for vertices and indices 
    //Delta
    //StaticBufferPool
    const uint32_t staticGeometryMemSize = (1 * 128) * 1024 * 1024;
    // Simulates DX11 style static buffers. For dynamic buffers please see 'DynamicBufferRingDX12.h'
    //
    // This class allows suballocating small chuncks of memory from a huge buffer that is allocated on creation 
    // This class is specialized in vertex and index buffers. 
    //For "StaticGeom"
    // create the buffer, allocate it in SYSTEM memory, bind it and map it: m_deviceMemory
    //bUseVidMem==true: 用于创建和配置 GPU 内存分配: VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    //true => |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    // true=> // create the buffer, allocate it in VIDEO memory and bind it: m_deviceMemoryVid
    //TODO: Annotate
    m_VidMemBufferPool.OnCreate(pDevice, staticGeometryMemSize, true, "StaticGeom");

    // Create a 'static' pool for vertices and indices in system memory
    //Delta
    //StaticBufferPool
    const uint32_t systemGeometryMemSize = 32 * 1024;
    //For "PostProcGeom"
    //TODO: Annotate
    m_SysMemBufferPool.OnCreate(pDevice, systemGeometryMemSize, false, "PostProcGeom");

    // initialize the GPU time stamps module
    //GPUTimestamps: OnBeginFrame
    // This class helps **insert queries in the command buffer and readback the results**.
    // The **tricky part in fact is reading back the results without stalling the GPU**. 
    // For that it splits the readback heap in <numberOfBackBuffers> pieces and it reads 
    // from the last used chuck.
    //vkCreateQueryPool(pDevice->GetDevice(), &queryPoolCreateInfo, NULL, &m_QueryPool);
    //m_QueryPool:VkQueryPool:用于收集在渲染过程中对 GPU 性能和状态信息进行查询的结果，并将这些结果返回到应用程序以供分析和优化
    m_GPUTimer.OnCreate(pDevice, self_backBufferCount);
    //

    // Quick helper to upload resources, it has it's own commandList and uses suballocation.
    //Delta
    //UploadHeap
    const uint32_t uploadHeapMemSize = 1000 * 1024 * 1024;
    // This class **shows the most efficient way to upload resources to the GPU memory**. 
    // The idea is to **create just one upload heap and suballocate memory from it**.
    // For **convenience this class comes with it's own command list & submit (FlushAndFinish)**
    //
    m_UploadHeap.OnCreate(pDevice, uploadHeapMemSize);    // initialize an upload heap (uses suballocation for faster results)

    /*
    Example of GBuffer and GBufferRenderPass
    //一个 GBuffer{m_GBuffer} -pass GBuffer* to> 一|多个 RenderPass{}
    //GBufferFlagBits包含 多个  数值为2^? 的 枚举, 用uint32_t|GBufferFlags 来标识
    */
    // Create GBuffer and render passes
    //
    {
        // GBuffer::OnCreate(Device *pDevice, ResourceViewHeaps *pHeaps, const std::map<GBufferFlags, VkFormat> &formats {{GBufferFlagBits,VkFormat:GBuffer的数据格式},...}
        //                  , int sampleCount=1 {使用shadow-map})
        m_GBuffer.OnCreate(
            pDevice,
            &m_ResourceViewHeaps,
            {
                { GBUFFER_DEPTH, VK_FORMAT_D32_SFLOAT},
                { GBUFFER_FORWARD, VK_FORMAT_R16G16B16A16_SFLOAT},
                { GBUFFER_MOTION_VECTORS, VK_FORMAT_R16G16_SFLOAT},
            },
            1
            );

        GBufferFlags fullGBuffer = GBUFFER_DEPTH | GBUFFER_FORWARD | GBUFFER_MOTION_VECTORS;
        bool bClear = true;
        //GBufferRenderPass{GBuffer's RenderPass-es}
        // Clear All|Full GBuffers: GBUFFER_DEPTH | GBUFFER_FORWARD | GBUFFER_MOTION_VECTORS
        m_RenderPassFullGBufferWithClear.OnCreate(&m_GBuffer, fullGBuffer, bClear, "m_RenderPassFullGBufferWithClear");
        // Main All|Full GBuffers -RenderPass: GBUFFER_DEPTH | GBUFFER_FORWARD | GBUFFER_MOTION_VECTORS;
        m_RenderPassFullGBuffer.OnCreate(&m_GBuffer, fullGBuffer, !bClear, "m_RenderPassFullGBuffer");
        // Depth+Hdr -RnderPass: GBUFFER_DEPTH | GBUFFER_FORWARD
        m_RenderPassJustDepthAndHdr.OnCreate(&m_GBuffer, GBUFFER_DEPTH | GBUFFER_FORWARD, !bClear, "m_RenderPassJustDepthAndHdr");
    }

    // Special
    // Create render pass shadow, will clear contents
    {
        VkAttachmentDescription depthAttachments;
        AttachClearBeforeUse(VK_FORMAT_D32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &depthAttachments);
        // Depth{Shadow} -RenderPass: GBUFFER_DEPTH{SHADER_READ_ONLY_OPTIMAL}: Generate shadow-map
        m_Render_pass_shadow = CreateRenderPassOptimal(m_pDevice->GetDevice(), 0, NULL, &depthAttachments);
    }

    //Use m_ResourceViewHeaps to allocate Memory for Buffer
    
    // Create PostProcess + Wireframe
    m_SkyDome.OnCreate(pDevice, m_RenderPassJustDepthAndHdr.GetRenderPass(), &m_UploadHeap, VK_FORMAT_R16G16B16A16_SFLOAT, &m_ResourceViewHeaps, &m_ConstantBufferRing, &m_VidMemBufferPool, "..\\media\\cauldron-media\\envmaps\\papermill\\diffuse.dds", "..\\media\\cauldron-media\\envmaps\\papermill\\specular.dds", VK_SAMPLE_COUNT_1_BIT);
    m_SkyDomeProc.OnCreate(pDevice, m_RenderPassJustDepthAndHdr.GetRenderPass(), &m_UploadHeap, VK_FORMAT_R16G16B16A16_SFLOAT, &m_ResourceViewHeaps, &m_ConstantBufferRing, &m_VidMemBufferPool, VK_SAMPLE_COUNT_1_BIT);
    m_Wireframe.OnCreate(pDevice, m_RenderPassJustDepthAndHdr.GetRenderPass(), &m_ResourceViewHeaps, &m_ConstantBufferRing, &m_VidMemBufferPool, VK_SAMPLE_COUNT_1_BIT);
    //  Draw() through &m_Wireframe
    m_WireframeBox.OnCreate(pDevice, &m_ResourceViewHeaps, &m_ConstantBufferRing, &m_VidMemBufferPool);
    m_DownSample.OnCreate(pDevice, &m_ResourceViewHeaps, &m_ConstantBufferRing, &m_VidMemBufferPool, VK_FORMAT_R16G16B16A16_SFLOAT);
    m_Bloom.OnCreate(pDevice, &m_ResourceViewHeaps, &m_ConstantBufferRing, &m_VidMemBufferPool, VK_FORMAT_R16G16B16A16_SFLOAT);
    m_TAA.OnCreate(pDevice, &m_ResourceViewHeaps, &m_VidMemBufferPool, &m_ConstantBufferRing);
    m_MagnifierPS.OnCreate(pDevice, &m_ResourceViewHeaps, &m_ConstantBufferRing, &m_VidMemBufferPool, VK_FORMAT_R16G16B16A16_SFLOAT);

    // Create tonemapping pass
    m_ToneMappingCS.OnCreate(pDevice, &m_ResourceViewHeaps, &m_ConstantBufferRing);
    m_ToneMappingPS.OnCreate(m_pDevice, pSwapChain->GetRenderPass(), &m_ResourceViewHeaps, &m_VidMemBufferPool, &m_ConstantBufferRing);
    m_ColorConversionPS.OnCreate(pDevice, pSwapChain->GetRenderPass(), &m_ResourceViewHeaps, &m_VidMemBufferPool, &m_ConstantBufferRing);

    // Initialize UI rendering resources
    m_ImGUI.OnCreate(m_pDevice, pSwapChain->GetRenderPass(), &m_UploadHeap, &m_ConstantBufferRing, FontSize);

    // Make sure upload heap has finished uploading before continuing
    // StaticBufferPool::UploadData(VkCommandBuffer=m_UploadHeap.GetCommandList()): Record Copy-OP in CommandBuffer
    m_VidMemBufferPool.UploadData(m_UploadHeap.GetCommandList());
    m_UploadHeap.FlushAndFinish();
    // After each stage of Allocate|SubAllocate needing m_UploadHeap.FlushAndFinish();
    //
    _OnCreate(pDevice, pSwapChain, FontSize);
    //
}



//--------------------------------------------------------------------------------------
//
// _OnCreate
//
//--------------------------------------------------------------------------------------
void SelfRenderer::_OnCreate(Device* pDevice, SwapChain* pSwapChain, float FontSize)
{
    m_pDevice = pDevice;

    //CommandPool+CommandBuffer: Ring
    //Compute
    {
        uint32_t commandListsPerBackBuffer = 8;
        bool bCompute = true;

        m_ComputeCommandListRing.OnCreate(m_pDevice, self_backBufferCount, commandListsPerBackBuffer, bCompute);
    }

    //DescriptorSet: Heap
    //Finished: m_DescriptorSetHeap.OnCreate(m_pDevice)
    //
    {
        //From DX: https://zhuanlan.zhihu.com/p/108618547
        //
        const uint32_t numMaxDescriptorSet = 500;
        //  CBV(ConstantBufferView: ReadOnly- Uniform|Buffer?): VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC|VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
        //const uint32_t cbvDescriptorCount = 2000;
        const uint32_t cbvDescriptorCount = 200;
        //VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE 
        //  SRV(ShaderResourceView: ReadOnly- Texture): VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        //const uint32_t srvDescriptorCount = 8000;
        const uint32_t srvDescriptorCount = 500;
        //VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC 
        //  UAV(UnorderedAccessView: AccessRandom- Buffer|Texture|Uniform?): VK_DESCRIPTOR_TYPE_STORAGE_BUFFER|VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC 
        const uint32_t uavDescriptorCount = 10;
        //  Sampler: VK_DESCRIPTOR_TYPE_SAMPLER
        const uint32_t samplerDescriptorCount = 20;
        //  RTV(RenderTargetView: WriteOnly- RenderTarget)
        //  DSV(DepthStencilView: WriteOnly- DepthStencil)
        //
        std::vector<VkDescriptorPoolSize> array_descriptorPoolSize = 
        {
             {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ,cbvDescriptorCount}
            ,{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER         ,cbvDescriptorCount}
            ,{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ,uavDescriptorCount}
            ,{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER         ,uavDescriptorCount}
        };
        //
        m_DescriptorSetHeap.OnCreate(m_pDevice, numMaxDescriptorSet, array_descriptorPoolSize);
        //
    }


    //
    //queueFamilyIndices for Buffer
    uint32_t queue_family_count, queueFamilyIndex_bitmask = 0;
    std::vector<uint32_t> queueFamilyIndices;
    //queueFamilyIndexCount, queueFamilyIndices
    {
        vkGetPhysicalDeviceQueueFamilyProperties(m_pDevice->GetPhysicalDevice(), &queue_family_count, NULL);
        assert(queue_family_count >= 1);
        //TODO: queueFamilyIndex_bitmask|= RtQueueFamiltIndex
        queueFamilyIndex_bitmask = m_pDevice->GetGraphicsQueueFamilyIndex() | m_pDevice->GetPresentQueueFamilyIndex() | m_pDevice->GetComputeQueueFamilyIndex();
        for (uint32_t i = 0; i < queue_family_count; ++i)
        {
            if (queueFamilyIndex_bitmask & 1)
            {
                //++queueFamilyIndexCount;
                queueFamilyIndices.push_back(i);
            }
            queueFamilyIndex_bitmask >>= 1;
        }
        queueFamilyIndex_bitmask = m_pDevice->GetGraphicsQueueFamilyIndex() | m_pDevice->GetPresentQueueFamilyIndex() | m_pDevice->GetComputeQueueFamilyIndex();
    }
    
    //Buffer: Dynamic
    //m_ConstBufferDynamic_CPU2GPU
    {
        const uint32_t constBuffersDynamicMemSize = 1024 * 1024;
        VkBufferCreateInfo  buffer_createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        PointerConstBufferMemoryAllocationInfo  p_constMemoryAllocateInfo;
#ifdef USE_VMA
        //
        buffer_createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        //TODO
        buffer_createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        //buffer_createInfo.sharingMode
        //buffer_createInfo.queueFamilyIndexCount
        //buffer_createInfo.pQueueFamilyIndices
        //
        std::string     userDataName{ "Uniform|Const" };
        VmaAllocationCreateInfo     vmaAllocation_createInfo{};
        //
        vmaAllocation_createInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        //
        vmaAllocation_createInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
        vmaAllocation_createInfo.pUserData = (void *)userDataName.c_str();
        //
        p_constMemoryAllocateInfo.pVmaAllocationInfo = &vmaAllocation_createInfo;
        //
#else
        //
        buffer_createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        //TODO
        buffer_createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        //
        buffer_createInfo.sharingMode = (queueFamilyIndices.size() > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        buffer_createInfo.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
        buffer_createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        //
        VkMemoryAllocateInfo    vkMemory_allocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        //
        vkMemory_allocateInfo.memoryTypeIndex = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        //
        p_constMemoryAllocateInfo.pVkMemoryAllocateInfo = &vkMemory_allocateInfo;
        //
#endif // USE_VMA
        //
        bool bUseVidMem = false;
        m_ConstBufferDynamic_CPU2GPU.OnCreate(m_pDevice, self_backBufferCount, constBuffersDynamicMemSize
            , &buffer_createInfo, p_constMemoryAllocateInfo
            , bUseVidMem, NULL);
        //
    }
    //

    //
    //m_RtBufferDynamic_GPU
    {
        const uint32_t rtBuffersDynamicMemSize = 32 * 1024 * 1024;
        VkBufferCreateInfo  buffer_createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        PointerConstBufferMemoryAllocationInfo  p_constMemoryAllocateInfo;
#ifdef USE_VMA
        //
        buffer_createInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        //buffer_createInfo.sharingMode
        //buffer_createInfo.queueFamilyIndexCount
        //buffer_createInfo.pQueueFamilyIndices
        //
        std::string     userDataName{ "AccelerationStructure|ScratchBuffer" };
        VmaAllocationCreateInfo     vmaAllocation_createInfo{};
        //
        vmaAllocation_createInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        //
        vmaAllocation_createInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
        vmaAllocation_createInfo.pUserData = (void*)userDataName.c_str();
        //
        p_constMemoryAllocateInfo.pVmaAllocationInfo = &vmaAllocation_createInfo;

#else
        //
        buffer_createInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;;
        buffer_createInfo.sharingMode = (queueFamilyIndices.size() > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        buffer_createInfo.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
        buffer_createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        //
        VkMemoryAllocateInfo    vkMemory_allocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        //
        //VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD
        vkMemory_allocateInfo.memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        //
        p_constMemoryAllocateInfo.pVkMemoryAllocateInfo = &vkMemory_allocateInfo;
        //

#endif // USE_VMA
        //
        bool bUseVidMem = false;
        m_RtBufferDynamic_GPU.OnCreate(m_pDevice, self_backBufferCount, rtBuffersDynamicMemSize
            , &buffer_createInfo, p_constMemoryAllocateInfo
            , bUseVidMem, NULL);
        //
    }
    //


    //Buffer: Static
    //m_StorageBufferStatic_GPU<=m_PrimitiveBufferStatic_GPU+m_OnlyStorageBufferStatic_GPU
    {
        const uint32_t primitiveBuffersStaticMemSize = 16 * 1024 * 1024;
        VkBufferCreateInfo  buffer_createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        PointerConstBufferMemoryAllocationInfo  p_constMemoryAllocateInfo;
#ifdef USE_VMA
        //
        buffer_createInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        //buffer_createInfo.sharingMode
        //buffer_createInfo.queueFamilyIndexCount
        //buffer_createInfo.pQueueFamilyIndices
        //
        std::string     userDataName{ "Primitive|ParticlePosition" };
        VmaAllocationCreateInfo     vmaAllocation_createInfo{};
        //
        vmaAllocation_createInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        //vmaAllocation_createInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
        //
        vmaAllocation_createInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
        vmaAllocation_createInfo.pUserData = (void*)userDataName.c_str();
        //
        p_constMemoryAllocateInfo.pVmaAllocationInfo = &vmaAllocation_createInfo;
        //
#else
        //
        buffer_createInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        buffer_createInfo.sharingMode = (queueFamilyIndices.size() > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        buffer_createInfo.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
        buffer_createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        //
        VkMemoryAllocateInfo    vkMemory_allocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        ////VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD
        vkMemory_allocateInfo.memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        //
        p_constMemoryAllocateInfo.pVkMemoryAllocateInfo = &vkMemory_allocateInfo;
        //
#endif // USE_VMA
        //
        bool bUseVidMem = false;
        //m_PrimitiveBufferStatic_GPU.OnCreate(m_pDevice, primitiveBuffersStaticMemSize
        m_StorageBufferStatic_GPU.OnCreate(m_pDevice, primitiveBuffersStaticMemSize
            , &buffer_createInfo, NULL
            , p_constMemoryAllocateInfo, { NULL }
            , bUseVidMem, NULL);
        //
    }
    //

    //
    //m_OnlyStorageBufferStatic_GPU
    /*
    {
        const uint32_t onlyStorageBuffersStaticMemSize = 32 * 1024 * 1024;
        VkBufferCreateInfo  buffer_createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        PointerConstBufferMemoryAllocationInfo  p_constMemoryAllocateInfo;
#ifdef USE_VMA
        //
        buffer_createInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        //buffer_createInfo.sharingMode
        //buffer_createInfo.queueFamilyIndexCount
        //buffer_createInfo.pQueueFamilyIndices
        //
        std::string     userDataName{ "Storage|ParticleData" };
        VmaAllocationCreateInfo     vmaAllocation_createInfo{};
        //
        vmaAllocation_createInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        //
        vmaAllocation_createInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
        vmaAllocation_createInfo.pUserData = (void*)userDataName.c_str();
        //
        p_constMemoryAllocateInfo.pVmaAllocationInfo = &vmaAllocation_createInfo;
        //
#else
        //
        buffer_createInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        buffer_createInfo.sharingMode = (queueFamilyIndices.size() > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        buffer_createInfo.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
        buffer_createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        //
        VkMemoryAllocateInfo    vkMemory_allocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        ////VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD
        vkMemory_allocateInfo.memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        //
        p_constMemoryAllocateInfo.pVkMemoryAllocateInfo = &vkMemory_allocateInfo;
        //
#endif // USE_VMA
        //
        bool bUseVidMem = false;
        m_OnlyStorageBufferStatic_GPU.OnCreate(m_pDevice, onlyStorageBuffersStaticMemSize
            , &buffer_createInfo, NULL
            , p_constMemoryAllocateInfo, { NULL }
        , bUseVidMem, NULL);
        //
    }
    */
    //

    //
    //m_RtBufferStatic_GPU
    {
        const uint32_t RtBuffersStaticMemSize = 16 * 1024 * 1024;
        VkBufferCreateInfo  buffer_createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        PointerConstBufferMemoryAllocationInfo  p_constMemoryAllocateInfo;
#ifdef USE_VMA
        //
        buffer_createInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
        //TODO
        buffer_createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        //buffer_createInfo.sharingMode
        //buffer_createInfo.queueFamilyIndexCount
        //buffer_createInfo.pQueueFamilyIndices
        //
        std::string     userDataName{ "RtStorage|AaabbBuffer|InstanceBuffer|SbtBuffer" };
        VmaAllocationCreateInfo     vmaAllocation_createInfo{};
        //
        vmaAllocation_createInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        //
        vmaAllocation_createInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
        vmaAllocation_createInfo.pUserData = (void*)userDataName.c_str();
        //
        p_constMemoryAllocateInfo.pVmaAllocationInfo = &vmaAllocation_createInfo;
        //
#else
        //
        buffer_createInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;;
        //TODO
        buffer_createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        //
        buffer_createInfo.sharingMode = (queueFamilyIndices.size() > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        buffer_createInfo.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
        buffer_createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        //
        VkMemoryAllocateInfo    vkMemory_allocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        ////VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD
        vkMemory_allocateInfo.memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        //
        p_constMemoryAllocateInfo.pVkMemoryAllocateInfo = &vkMemory_allocateInfo;
        //
#endif // USE_VMA
        //
        bool bUseVidMem = false;
        m_RtBufferStatic_GPU.OnCreate(m_pDevice, RtBuffersStaticMemSize
            , &buffer_createInfo, NULL
            , p_constMemoryAllocateInfo, { NULL }
        , bUseVidMem, NULL);
        //
    }
    //


    //m_FluidParticeCS_SPH
    m_FluidParticeCS_SPH.OnCreate(m_pDevice, 125000, { 50,50,50 }
        , m_RenderPassJustDepthAndHdr.GetRenderPass()
        , &m_DescriptorSetHeap, &m_ConstBufferDynamic_CPU2GPU, &m_RtBufferDynamic_GPU
        , &m_StorageBufferStatic_GPU, &m_RtBufferStatic_GPU);
    //


    //

}



//--------------------------------------------------------------------------------------
//
// OnDestroy 
//
//--------------------------------------------------------------------------------------
void SelfRenderer::OnDestroy()
{
    m_AsyncPool.Flush();

    m_ImGUI.OnDestroy();
    m_ColorConversionPS.OnDestroy();
    m_ToneMappingPS.OnDestroy();
    m_ToneMappingCS.OnDestroy();
    m_TAA.OnDestroy();
    m_Bloom.OnDestroy();
    m_DownSample.OnDestroy();
    m_MagnifierPS.OnDestroy();
    m_WireframeBox.OnDestroy();
    m_Wireframe.OnDestroy();
    m_SkyDomeProc.OnDestroy();
    m_SkyDome.OnDestroy();

    m_RenderPassFullGBufferWithClear.OnDestroy();
    m_RenderPassJustDepthAndHdr.OnDestroy();
    m_RenderPassFullGBuffer.OnDestroy();
    m_GBuffer.OnDestroy();

    vkDestroyRenderPass(m_pDevice->GetDevice(), m_Render_pass_shadow, nullptr);

    m_UploadHeap.OnDestroy();
    m_GPUTimer.OnDestroy();
    m_VidMemBufferPool.OnDestroy();
    m_SysMemBufferPool.OnDestroy();
    m_ConstantBufferRing.OnDestroy();
    m_ResourceViewHeaps.OnDestroy();
    m_CommandListRing.OnDestroy();

    //
    _OnDestroy();
    //
}

void SelfRenderer::_OnDestroy()
{
    
    //
    m_FluidParticeCS_SPH.OnDestroy();
    //
    m_ConstBufferDynamic_CPU2GPU.OnDestroy();
    m_RtBufferDynamic_GPU.OnDestroy();
    m_StorageBufferStatic_GPU.OnDestroy();
    m_RtBufferStatic_GPU.OnDestroy();
    //
    m_DescriptorSetHeap.OnDestroy();
    m_ComputeCommandListRing.OnDestroy();
    //
}

//--------------------------------------------------------------------------------------
//
// OnCreateWindowSizeDependentResources
//
//--------------------------------------------------------------------------------------
void SelfRenderer::OnCreateWindowSizeDependentResources(SwapChain* pSwapChain, uint32_t Width, uint32_t Height)
{
    m_Width = Width;
    m_Height = Height;

    // Set the viewport
    //
    m_Viewport.x = 0;
    m_Viewport.y = (float)Height;
    m_Viewport.width = (float)Width;
    m_Viewport.height = -(float)(Height);
    m_Viewport.minDepth = (float)0.0f;
    m_Viewport.maxDepth = (float)1.0f;

    // Create scissor rectangle
    //
    m_RectScissor.extent.width = Width;
    m_RectScissor.extent.height = Height;
    m_RectScissor.offset.x = 0;
    m_RectScissor.offset.y = 0;

    // Create GBuffer
    //
    m_GBuffer.OnCreateWindowSizeDependentResources(pSwapChain, Width, Height);

    // Create frame buffers for the GBuffer render passes
    //
    //Contian m_pGBuffer
    m_RenderPassFullGBufferWithClear.OnCreateWindowSizeDependentResources(Width, Height);
    m_RenderPassJustDepthAndHdr.OnCreateWindowSizeDependentResources(Width, Height);
    m_RenderPassFullGBuffer.OnCreateWindowSizeDependentResources(Width, Height);

    // Update PostProcessing passes
    //
    m_DownSample.OnCreateWindowSizeDependentResources(Width, Height, &m_GBuffer.m_HDR, 6); //downsample the HDR texture 6 times
    m_Bloom.OnCreateWindowSizeDependentResources(Width / 2, Height / 2, m_DownSample.GetTexture(), 6, &m_GBuffer.m_HDR);
    m_TAA.OnCreateWindowSizeDependentResources(Width, Height, &m_GBuffer);
    m_MagnifierPS.OnCreateWindowSizeDependentResources(&m_GBuffer.m_HDR);
    m_bMagResourceReInit = true;
}


//--------------------------------------------------------------------------------------
//
// OnDestroyWindowSizeDependentResources
//
//--------------------------------------------------------------------------------------
void SelfRenderer::OnDestroyWindowSizeDependentResources()
{
    m_Bloom.OnDestroyWindowSizeDependentResources();
    m_DownSample.OnDestroyWindowSizeDependentResources();
    m_TAA.OnDestroyWindowSizeDependentResources();
    m_MagnifierPS.OnDestroyWindowSizeDependentResources();

    m_RenderPassFullGBufferWithClear.OnDestroyWindowSizeDependentResources();
    m_RenderPassJustDepthAndHdr.OnDestroyWindowSizeDependentResources();
    m_RenderPassFullGBuffer.OnDestroyWindowSizeDependentResources();
    m_GBuffer.OnDestroyWindowSizeDependentResources();
}


void SelfRenderer::OnUpdateDisplayDependentResources(SwapChain* pSwapChain, bool bUseMagnifier)
{
    // Update the pipelines if the swapchain render pass has changed (for example when the format of the swapchain changes)
    //
    m_ColorConversionPS.UpdatePipelines(pSwapChain->GetRenderPass(), pSwapChain->GetDisplayMode());
    m_ToneMappingPS.UpdatePipelines(pSwapChain->GetRenderPass());

    m_ImGUI.UpdatePipeline((pSwapChain->GetDisplayMode() == DISPLAYMODE_SDR) ? pSwapChain->GetRenderPass() : bUseMagnifier ? m_MagnifierPS.GetPassRenderPass() : m_RenderPassJustDepthAndHdr.GetRenderPass());
}


//--------------------------------------------------------------------------------------
//
// OnUpdateLocalDimmingChangedResources
//
//--------------------------------------------------------------------------------------
void SelfRenderer::OnUpdateLocalDimmingChangedResources(SwapChain* pSwapChain)
{
    m_ColorConversionPS.UpdatePipelines(pSwapChain->GetRenderPass(), pSwapChain->GetDisplayMode());
}


//--------------------------------------------------------------------------------------
//
// LoadScene
//
//--------------------------------------------------------------------------------------
int SelfRenderer::LoadScene(GLTFCommon* pGLTFCommon, int Stage)
{
    // show loading progress
    //
    ImGui::OpenPopup("Loading");
    if (ImGui::BeginPopupModal("Loading", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        float progress = (float)Stage / 12.0f;
        ImGui::ProgressBar(progress, ImVec2(0.f, 0.f), NULL);
        ImGui::EndPopup();
    }

    // use multi threading
    AsyncPool* pAsyncPool = &m_AsyncPool;

    // Loading stages
    //
    if (Stage == 0)
    {
    }
    else if (Stage == 5)
    {
        Profile p("m_pGltfLoader->Load");

        m_pGLTFTexturesAndBuffers = new GLTFTexturesAndBuffers();
        m_pGLTFTexturesAndBuffers->OnCreate(m_pDevice, pGLTFCommon, &m_UploadHeap, &m_VidMemBufferPool, &m_ConstantBufferRing);
    }
    else if (Stage == 6)
    {
        Profile p("LoadTextures");

        // here we are loading onto the GPU all the textures and the inverse matrices
        // this data will be used to create the PBR and Depth passes       
        m_pGLTFTexturesAndBuffers->LoadTextures(pAsyncPool);
    }
    else if (Stage == 7)
    {
        Profile p("m_GLTFDepth->OnCreate");

        //create the glTF's textures, VBs, IBs, shaders and descriptors for this particular pass    
        m_GLTFDepth = new GltfDepthPass();
        m_GLTFDepth->OnCreate(
            m_pDevice,
            m_Render_pass_shadow,
            &m_UploadHeap,
            &m_ResourceViewHeaps,
            &m_ConstantBufferRing,
            &m_VidMemBufferPool,
            m_pGLTFTexturesAndBuffers,
            pAsyncPool
        );

        m_VidMemBufferPool.UploadData(m_UploadHeap.GetCommandList());
        m_UploadHeap.FlushAndFinish();
    }
    else if (Stage == 8)
    {
        Profile p("m_GLTFPBR->OnCreate");

        // same thing as above but for the PBR pass
        m_GLTFPBR = new GltfPbrPass();
        m_GLTFPBR->OnCreate(
            m_pDevice,
            &m_UploadHeap,
            &m_ResourceViewHeaps,
            &m_ConstantBufferRing,
            &m_VidMemBufferPool,
            m_pGLTFTexturesAndBuffers,
            &m_SkyDome,
            false, // use SSAO mask
            m_ShadowSRVPool,
            &m_RenderPassFullGBufferWithClear,
            pAsyncPool
        );

        m_VidMemBufferPool.UploadData(m_UploadHeap.GetCommandList());
        m_UploadHeap.FlushAndFinish();
    }
    else if (Stage == 9)
    {
        Profile p("m_GLTFBBox->OnCreate");

        // just a bounding box pass that will draw boundingboxes instead of the geometry itself
        m_GLTFBBox = new GltfBBoxPass();
        m_GLTFBBox->OnCreate(
            m_pDevice,
            m_RenderPassJustDepthAndHdr.GetRenderPass(),
            &m_ResourceViewHeaps,
            &m_ConstantBufferRing,
            &m_VidMemBufferPool,
            m_pGLTFTexturesAndBuffers,
            &m_Wireframe
        );

        // we are borrowing the upload heap command list for uploading to the GPU the IBs and VBs
        m_VidMemBufferPool.UploadData(m_UploadHeap.GetCommandList());

    }
    else if (Stage == 10)
    {
        Profile p("Flush");

        m_UploadHeap.FlushAndFinish();

        //once everything is uploaded we dont need the upload heaps anymore
        m_VidMemBufferPool.FreeUploadHeap();

        // tell caller that we are done loading the map
        return 0;
    }

    Stage++;
    return Stage;
}

//--------------------------------------------------------------------------------------
//
// UnloadScene
//
//--------------------------------------------------------------------------------------
void SelfRenderer::UnloadScene()
{
    // wait for all the async loading operations to finish
    m_AsyncPool.Flush();

    m_pDevice->GPUFlush();

    if (m_GLTFPBR)
    {
        m_GLTFPBR->OnDestroy();
        delete m_GLTFPBR;
        m_GLTFPBR = NULL;
    }

    if (m_GLTFDepth)
    {
        m_GLTFDepth->OnDestroy();
        delete m_GLTFDepth;
        m_GLTFDepth = NULL;
    }

    if (m_GLTFBBox)
    {
        m_GLTFBBox->OnDestroy();
        delete m_GLTFBBox;
        m_GLTFBBox = NULL;
    }

    if (m_pGLTFTexturesAndBuffers)
    {
        m_pGLTFTexturesAndBuffers->OnDestroy();
        delete m_pGLTFTexturesAndBuffers;
        m_pGLTFTexturesAndBuffers = NULL;
    }

    assert(m_shadowMapPool.size() == m_ShadowSRVPool.size());
    while (!m_shadowMapPool.empty())
    {
        m_shadowMapPool.back().ShadowMap.OnDestroy();
        vkDestroyFramebuffer(m_pDevice->GetDevice(), m_shadowMapPool.back().ShadowFrameBuffer, nullptr);
        vkDestroyImageView(m_pDevice->GetDevice(), m_ShadowSRVPool.back(), nullptr);
        vkDestroyImageView(m_pDevice->GetDevice(), m_shadowMapPool.back().ShadowDSV, nullptr);
        m_ShadowSRVPool.pop_back();
        m_shadowMapPool.pop_back();
    }
}

void SelfRenderer::AllocateShadowMaps(GLTFCommon* pGLTFCommon)
{
    // Go through the lights and allocate shadow information
    uint32_t NumShadows = 0;
    for (int i = 0; i < pGLTFCommon->m_lightInstances.size(); ++i)
    {
        const tfLight& lightData = pGLTFCommon->m_lights[pGLTFCommon->m_lightInstances[i].m_lightId];
        if (lightData.m_shadowResolution)
        {
            SceneShadowInfo ShadowInfo;
            ShadowInfo.ShadowResolution = lightData.m_shadowResolution;
            ShadowInfo.ShadowIndex = NumShadows++;
            ShadowInfo.LightIndex = i;
            m_shadowMapPool.push_back(ShadowInfo);
        }
    }

    if (NumShadows > MaxShadowInstances)
    {
        Trace("Number of shadows has exceeded maximum supported. Please grow value in gltfCommon.h/perFrameStruct.h");
        throw;
    }

    // If we had shadow information, allocate all required maps and bindings
    if (!m_shadowMapPool.empty())
    {
        std::vector<SceneShadowInfo>::iterator CurrentShadow = m_shadowMapPool.begin();
        for (uint32_t i = 0; CurrentShadow < m_shadowMapPool.end(); ++i, ++CurrentShadow)
        {
            CurrentShadow->ShadowMap.InitDepthStencil(m_pDevice, CurrentShadow->ShadowResolution, CurrentShadow->ShadowResolution, VK_FORMAT_D32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, "ShadowMap");
            CurrentShadow->ShadowMap.CreateDSV(&CurrentShadow->ShadowDSV);

            // Create render pass shadow, will clear contents
            {
                VkAttachmentDescription depthAttachments;
                AttachClearBeforeUse(VK_FORMAT_D32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &depthAttachments);

                // Create frame buffer
                VkImageView attachmentViews[1] = { CurrentShadow->ShadowDSV };
                VkFramebufferCreateInfo fb_info = {};
                fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                fb_info.pNext = NULL;
                fb_info.renderPass = m_Render_pass_shadow;
                fb_info.attachmentCount = 1;
                fb_info.pAttachments = attachmentViews;
                fb_info.width = CurrentShadow->ShadowResolution;
                fb_info.height = CurrentShadow->ShadowResolution;
                fb_info.layers = 1;
                VkResult res = vkCreateFramebuffer(m_pDevice->GetDevice(), &fb_info, NULL, &CurrentShadow->ShadowFrameBuffer);
                assert(res == VK_SUCCESS);
            }

            VkImageView ShadowSRV;
            CurrentShadow->ShadowMap.CreateSRV(&ShadowSRV);
            m_ShadowSRVPool.push_back(ShadowSRV);
        }
    }
}


//--------------------------------------------------------------------------------------
//
// OnRender
//
//--------------------------------------------------------------------------------------
void SelfRenderer::OnRender(const UIState* pState, const Camera& Cam, SwapChain* pSwapChain)
{
    // Let our resource managers do some house keeping 
    m_ConstantBufferRing.OnBeginFrame();
    //SelfImplementation
    m_ConstBufferDynamic_CPU2GPU.OnBeginFrame();
    m_RtBufferDynamic_GPU.OnBeginFrame();

    //SelfImplementation
    /*
    m_ComputeCommandListRing.OnBeginFrame();
    VkCommandBuffer compute_cmdBuf1 = m_ComputeCommandListRing.GetNewCommandList();
    //  Begin Record Compute - CommandBuffer
    {
        VkCommandBufferBeginInfo cmd_buf_info;
        cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmd_buf_info.pNext = NULL;
        cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        cmd_buf_info.pInheritanceInfo = NULL;
        VkResult res = vkBeginCommandBuffer(compute_cmdBuf1, &cmd_buf_info);
        assert(res == VK_SUCCESS);

    }
    */

    // command buffer calls
    VkCommandBuffer cmdBuf1 = m_CommandListRing.GetNewCommandList();

    // Begin Record CommandBuffer
    {
        VkCommandBufferBeginInfo cmd_buf_info;
        cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmd_buf_info.pNext = NULL;
        cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        cmd_buf_info.pInheritanceInfo = NULL;
        VkResult res = vkBeginCommandBuffer(cmdBuf1, &cmd_buf_info);
        assert(res == VK_SUCCESS);
    }

    m_GPUTimer.OnBeginFrame(cmdBuf1, &m_TimeStamps);

    //SelfImplementation
    // submit Compute- command buffer
    /*
    if (m_FluidParticeCS_SPH.GetbFirst())
    {
        //uint32_t index = m_pDevice->GetGraphicsQueueFamilyIndex();
        m_FluidParticeCS_SPH.DrawCompute(cmdBuf1, Cam);
    }
    */
    //
    /*
    {
        m_FluidParticeCS_SPH.DrawCompute(compute_cmdBuf1);

        // End Record CommandBuffer
        VkResult res = vkEndCommandBuffer(compute_cmdBuf1);
        assert(res == VK_SUCCESS);

        VkSemaphore     waitSemaphore = m_FluidParticeCS_SPH.GetGraphicsSemaphore();
        VkSemaphore     signalSemaphore = m_FluidParticeCS_SPH.GetComputeSemaphore();
        //  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
        VkPipelineStageFlags pWaitDstStageMask[] = { VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR };

        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = NULL;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &waitSemaphore;
        submit_info.pWaitDstStageMask = pWaitDstStageMask;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &compute_cmdBuf1;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &signalSemaphore;
        res = vkQueueSubmit(m_pDevice->GetComputeQueue(), 1, &submit_info, VK_NULL_HANDLE);
        assert(res == VK_SUCCESS);

    }
    */
    //

    // pPerFrame
    // m_pGLTFTexturesAndBuffers
    // 
    // Sets the perFrame data 
    per_frame* pPerFrame = NULL;
    if (m_pGLTFTexturesAndBuffers)
    {
        //Get PerFrame-Data from Cam
        // fill as much as possible using the GLTF (camera, lights, ...)
        pPerFrame = m_pGLTFTexturesAndBuffers->m_pGLTFCommon->SetPerFrameData(Cam);

        // Set some lighting factors
        pPerFrame->iblFactor = pState->IBLFactor;
        pPerFrame->emmisiveFactor = pState->EmissiveFactor;
        pPerFrame->invScreenResolution[0] = 1.0f / ((float)m_Width);
        pPerFrame->invScreenResolution[1] = 1.0f / ((float)m_Height);

        pPerFrame->wireframeOptions.setX(pState->WireframeColor[0]);
        pPerFrame->wireframeOptions.setY(pState->WireframeColor[1]);
        pPerFrame->wireframeOptions.setZ(pState->WireframeColor[2]);
        pPerFrame->wireframeOptions.setW(pState->WireframeMode == UIState::WireframeMode::WIREFRAME_MODE_SOLID_COLOR ? 1.0f : 0.0f);
        pPerFrame->lodBias = 0.0f;
        m_pGLTFTexturesAndBuffers->SetPerFrameConstants();
        m_pGLTFTexturesAndBuffers->SetSkinningMatricesForSkeletons();
    }

    // 1
    // m_GLTFDepth{with pPerFrame}
    // 
    // Render all shadow maps
    if (m_GLTFDepth && pPerFrame != NULL)
    {
        SetPerfMarkerBegin(cmdBuf1, "ShadowPass");

        VkClearValue depth_clear_values[1];
        depth_clear_values[0].depthStencil.depth = 1.0f;
        depth_clear_values[0].depthStencil.stencil = 0;

        VkRenderPassBeginInfo rp_begin;
        rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_begin.pNext = NULL;
        rp_begin.renderPass = m_Render_pass_shadow;
        rp_begin.renderArea.offset.x = 0;
        rp_begin.renderArea.offset.y = 0;
        rp_begin.clearValueCount = 1;
        rp_begin.pClearValues = depth_clear_values;

        std::vector<SceneShadowInfo>::iterator ShadowMap = m_shadowMapPool.begin();
        while (ShadowMap < m_shadowMapPool.end())
        {
            // Clear shadow map
            rp_begin.framebuffer = ShadowMap->ShadowFrameBuffer;
            rp_begin.renderArea.extent.width = ShadowMap->ShadowResolution;
            rp_begin.renderArea.extent.height = ShadowMap->ShadowResolution;
            // 有点类似CommandBuffer Record一个RenderPass的函数
            // For  Scissor+Viewport,ShadowMap
            vkCmdBeginRenderPass(cmdBuf1, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

            // Render to shadow map
            SetViewportAndScissor(cmdBuf1, 0, 0, ShadowMap->ShadowResolution, ShadowMap->ShadowResolution);

            // Set per frame constant buffer values
            GltfDepthPass::per_frame* cbPerFrame = m_GLTFDepth->SetPerFrameConstants();
            cbPerFrame->mViewProj = pPerFrame->lights[ShadowMap->LightIndex].mLightViewProj;

            m_GLTFDepth->Draw(cmdBuf1);

            m_GPUTimer.GetTimeStamp(cmdBuf1, "Shadow Map Render");

            vkCmdEndRenderPass(cmdBuf1);
            ++ShadowMap;
        }

        SetPerfMarkerEnd(cmdBuf1);
    }

    // 2
    // m_GLTFPBR{with pPerFrame}
    // 
    // Render Scene to the GBuffer ------------------------------------------------
    SetPerfMarkerBegin(cmdBuf1, "Color pass");

    VkRect2D renderArea = { 0, 0, m_Width, m_Height };

    if (pPerFrame != NULL && m_GLTFPBR)
    {
        const bool bWireframe = pState->WireframeMode != UIState::WireframeMode::WIREFRAME_MODE_OFF;

        std::vector<GltfPbrPass::BatchList> opaque, transparent;
        // Be called by OnRender()
        m_GLTFPBR->BuildBatchLists(&opaque, &transparent, bWireframe);

        // opaque: 不透明
        // Render opaque 
        {
            m_RenderPassFullGBufferWithClear.BeginPass(cmdBuf1, renderArea);

            m_GLTFPBR->DrawBatchList(cmdBuf1, &opaque, bWireframe);
            m_GPUTimer.GetTimeStamp(cmdBuf1, "PBR Opaque");

            m_RenderPassFullGBufferWithClear.EndPass(cmdBuf1);
        }
        /*
        */

        /*
        */
        // Render skydome
        {
            m_RenderPassJustDepthAndHdr.BeginPass(cmdBuf1, renderArea);

            if (pState->SelectedSkydomeTypeIndex == 1)
            {
                math::Matrix4 clipToView = math::inverse(pPerFrame->mCameraCurrViewProj);
                m_SkyDome.Draw(cmdBuf1, clipToView);

                m_GPUTimer.GetTimeStamp(cmdBuf1, "Skydome cube");
            }
            else if (pState->SelectedSkydomeTypeIndex == 0)
            {
                SkyDomeProc::Constants skyDomeConstants;
                skyDomeConstants.invViewProj = math::inverse(pPerFrame->mCameraCurrViewProj);
                skyDomeConstants.vSunDirection = math::Vector4(1.0f, 0.05f, 0.0f, 0.0f);
                skyDomeConstants.turbidity = 10.0f;
                skyDomeConstants.rayleigh = 2.0f;
                skyDomeConstants.mieCoefficient = 0.005f;
                skyDomeConstants.mieDirectionalG = 0.8f;
                skyDomeConstants.luminance = 1.0f;
                m_SkyDomeProc.Draw(cmdBuf1, skyDomeConstants);

                m_GPUTimer.GetTimeStamp(cmdBuf1, "Skydome Proc");
            }

            m_RenderPassJustDepthAndHdr.EndPass(cmdBuf1);
        }

        //SelfImplementation
        {
            m_RenderPassJustDepthAndHdr.BeginPass(cmdBuf1, renderArea);

            m_FluidParticeCS_SPH.DrawGraphics(cmdBuf1, Cam);
            m_GPUTimer.GetTimeStamp(cmdBuf1, "Render Particle");

            m_RenderPassJustDepthAndHdr.EndPass(cmdBuf1);
        }
        /*
        */

        // draw transparent geometry
        {
            m_RenderPassFullGBuffer.BeginPass(cmdBuf1, renderArea);

            std::sort(transparent.begin(), transparent.end());
            m_GLTFPBR->DrawBatchList(cmdBuf1, &transparent, bWireframe);
            m_GPUTimer.GetTimeStamp(cmdBuf1, "PBR Transparent");

            m_RenderPassFullGBuffer.EndPass(cmdBuf1);
        }

        
        //SelfImplementation
        /*
        {
            m_RenderPassJustDepthAndHdr.BeginPass(cmdBuf1, renderArea);

            m_FluidParticeCS_SPH.DrawGraphics(cmdBuf1, Cam);
            m_GPUTimer.GetTimeStamp(cmdBuf1, "Render Particle");

            m_RenderPassJustDepthAndHdr.EndPass(cmdBuf1);
        }
        */

        // draw object's bounding boxes
        {
            m_RenderPassJustDepthAndHdr.BeginPass(cmdBuf1, renderArea);

            // m_GLTFBBox
            //
            if (m_GLTFBBox)
            {
                if (pState->bDrawBoundingBoxes)
                {
                    m_GLTFBBox->Draw(cmdBuf1, pPerFrame->mCameraCurrViewProj);

                    m_GPUTimer.GetTimeStamp(cmdBuf1, "Bounding Box");
                }
            }

            // draw light's frustums
            if (pState->bDrawLightFrustum && pPerFrame != NULL)
            {
                SetPerfMarkerBegin(cmdBuf1, "light frustums");

                math::Vector4 vCenter = math::Vector4(0.0f, 0.0f, 0.5f, 0.0f);
                math::Vector4 vRadius = math::Vector4(1.0f, 1.0f, 0.5f, 0.0f);
                math::Vector4 vColor = math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
                for (uint32_t i = 0; i < pPerFrame->lightCount; i++)
                {
                    math::Matrix4 spotlightMatrix = math::inverse(pPerFrame->lights[i].mLightViewProj);
                    math::Matrix4 worldMatrix = pPerFrame->mCameraCurrViewProj * spotlightMatrix;
                    m_WireframeBox.Draw(cmdBuf1, &m_Wireframe, worldMatrix, vCenter, vRadius, vColor);
                }

                m_GPUTimer.GetTimeStamp(cmdBuf1, "Light's frustum");

                SetPerfMarkerEnd(cmdBuf1);
            }

            m_RenderPassJustDepthAndHdr.EndPass(cmdBuf1);
        }
    }
    else
    {
        //
        m_RenderPassFullGBufferWithClear.BeginPass(cmdBuf1, renderArea);
        //SelfImplementation
        //m_FluidParticeCS_SPH.DrawGraphics(cmdBuf1, Cam);
        //m_GPUTimer.GetTimeStamp(cmdBuf1, "Render Particle");
        //
        m_RenderPassFullGBufferWithClear.EndPass(cmdBuf1);
        //
        m_RenderPassJustDepthAndHdr.BeginPass(cmdBuf1, renderArea);
        m_RenderPassJustDepthAndHdr.EndPass(cmdBuf1);
    }
    
    //  VkImageMemoryBarrier  : m_GBuffer.m_HDR.Resource();s
    VkImageMemoryBarrier barrier[1] = {};
    barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier[0].pNext = NULL;
    barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier[0].oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier[0].subresourceRange.baseMipLevel = 0;
    barrier[0].subresourceRange.levelCount = 1;
    barrier[0].subresourceRange.baseArrayLayer = 0;
    barrier[0].subresourceRange.layerCount = 1;
    barrier[0].image = m_GBuffer.m_HDR.Resource();
    vkCmdPipelineBarrier(cmdBuf1, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0, NULL, 1, barrier);

    SetPerfMarkerEnd(cmdBuf1);

    // Post proc---------------------------------------------------------------------------

    // Bloom, takes HDR as input and applies bloom to it.
    /*
    {
        SetPerfMarkerBegin(cmdBuf1, "PostProcess");

        // Downsample pass
        m_DownSample.Draw(cmdBuf1);
        m_GPUTimer.GetTimeStamp(cmdBuf1, "Downsample");

        // Bloom pass (needs the downsampled data)
        m_Bloom.Draw(cmdBuf1);
        m_GPUTimer.GetTimeStamp(cmdBuf1, "Bloom");

        SetPerfMarkerEnd(cmdBuf1);
    }
    */
    //

    // Apply TAA & Sharpen to m_HDR
    if (pState->bUseTAA)
    {
        {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.pNext = NULL;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            VkImageMemoryBarrier barriers[3];
            barriers[0] = barrier;
            barriers[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            barriers[0].oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            barriers[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            barriers[0].image = m_GBuffer.m_DepthBuffer.Resource();

            barriers[1] = barrier;
            barriers[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barriers[1].oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barriers[1].image = m_GBuffer.m_MotionVectors.Resource();

            // no layout transition but we still need to wait
            barriers[2] = barrier;
            barriers[2].oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barriers[2].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barriers[2].image = m_GBuffer.m_HDR.Resource();

            vkCmdPipelineBarrier(cmdBuf1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, NULL, 0, NULL, 3, barriers);
        }

        m_TAA.Draw(cmdBuf1);
        m_GPUTimer.GetTimeStamp(cmdBuf1, "TAA");
    }


    // Magnifier Pass: m_HDR as input, pass' own output
    if (pState->bUseMagnifier)
    {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = NULL;
        barrier.srcAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.image = m_MagnifierPS.GetPassOutputResource();

        if (m_bMagResourceReInit)
        {
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            vkCmdPipelineBarrier(cmdBuf1, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
            m_bMagResourceReInit = false;
        }
        else
        {
            barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            vkCmdPipelineBarrier(cmdBuf1, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
        }

        // Note: assumes the input texture (specified in OnCreateWindowSizeDependentResources()) is in read state
        m_MagnifierPS.Draw(cmdBuf1, pState->MagnifierParams);
        m_GPUTimer.GetTimeStamp(cmdBuf1, "Magnifier");
    }

    //m_MagnifierPS
    // Start tracking input/output resources at this point to handle HDR and SDR render paths 
    VkImage      ImgCurrentInput = pState->bUseMagnifier ? m_MagnifierPS.GetPassOutputResource() : m_GBuffer.m_HDR.Resource();
    VkImageView  SRVCurrentInput = pState->bUseMagnifier ? m_MagnifierPS.GetPassOutputSRV() : m_GBuffer.m_HDRSRV;

    // If using FreeSync HDR, we need to do these in order: Tonemapping -> GUI -> Color Conversion
    const bool bHDR = pSwapChain->GetDisplayMode() != DISPLAYMODE_SDR;
    if (bHDR)
    {
        // In place Tonemapping ------------------------------------------------------------------------
        {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.pNext = NULL;
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.image = ImgCurrentInput;
            vkCmdPipelineBarrier(cmdBuf1, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

            m_ToneMappingCS.Draw(cmdBuf1, SRVCurrentInput, pState->Exposure, pState->SelectedTonemapperIndex, m_Width, m_Height);

            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.image = ImgCurrentInput;
            vkCmdPipelineBarrier(cmdBuf1, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

        }

        // Render HUD  ------------------------------------------------------------------------
        {

            if (pState->bUseMagnifier)
            {
                m_MagnifierPS.BeginPass(cmdBuf1, renderArea);
            }
            else
            {
                m_RenderPassJustDepthAndHdr.BeginPass(cmdBuf1, renderArea);
            }

            vkCmdSetScissor(cmdBuf1, 0, 1, &m_RectScissor);
            vkCmdSetViewport(cmdBuf1, 0, 1, &m_Viewport);

            // m_ImGUI also record CommandBuffer
            // draw GUI
            m_ImGUI.Draw(cmdBuf1);

            if (pState->bUseMagnifier)
            {
                m_MagnifierPS.EndPass(cmdBuf1);
            }
            else
            {
                m_RenderPassJustDepthAndHdr.EndPass(cmdBuf1);
            }

            if (bHDR && !pState->bUseMagnifier)
            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.pNext = NULL;
                barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;
                barrier.image = ImgCurrentInput;
                vkCmdPipelineBarrier(cmdBuf1, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
            }

            m_GPUTimer.GetTimeStamp(cmdBuf1, "ImGUI Rendering");
        }
    }

    //SelfImplementation
    //if (m_FluidParticeCS_SPH.GetbFirst())
    {
        //uint32_t index = m_pDevice->GetGraphicsQueueFamilyIndex();
        m_FluidParticeCS_SPH.DrawCompute(cmdBuf1, Cam);
        m_GPUTimer.GetTimeStamp(cmdBuf1, "Compute Particle");

    }
    /*
    */
    //


    // submit command buffer
    {
        // End Record CommandBuffer
        VkResult res = vkEndCommandBuffer(cmdBuf1);
        assert(res == VK_SUCCESS);

        /*
        //SelfImplementation
        VkSemaphore waitSemaphore = m_FluidParticeCS_SPH.GetComputeSemaphore();
        VkSemaphore signalSemaphores = m_FluidParticeCS_SPH.GetGraphicsSemaphore();
        // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 
        VkPipelineStageFlags pWaitDstStageMask[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT};
        */

        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = NULL;
        submit_info.waitSemaphoreCount = 0;
        //submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = NULL;
        //submit_info.pWaitSemaphores = &waitSemaphore;
        submit_info.pWaitDstStageMask = NULL;
        // Dual 2 waitSemaphoreCount
        //submit_info.pWaitDstStageMask = pWaitDstStageMask;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmdBuf1;
        submit_info.signalSemaphoreCount = 0;
        //submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = NULL;
        //submit_info.pSignalSemaphores = &signalSemaphores;
        //
        res = vkQueueSubmit(m_pDevice->GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
        assert(res == VK_SUCCESS);
    }

    //2 SwapChain
    // Finish: Begin+End a Swapchain|LowCost RenderPass|Frame For Swapchain to Interval 2 coherent-Frame since Sequential-Calling
    // This wat can Drop-out WaitIdle-Func{vkDeviceWaitIdle,vkQueueWaitIdle,...} => But Necessarily?
    // Wait for swapchain (we are going to render to it) -----------------------------------
    int imageIndex = pSwapChain->WaitForSwapChain();
    // Wait for the CmdBufExecutedFences of previous-frame
    //Fence help 2 Syn
    

    /*
    */
    /*
    const uint32_t offset = 25*50*50+0+25;   //67500;
    auto ptr = m_FluidParticeCS_SPH.m_particlePosPointer;  //+125000-5;
    auto acc_ptr = m_FluidParticeCS_SPH.m_particleAccPointer, vlc_ptr = m_FluidParticeCS_SPH.m_particleVlcPointer;
    //math::Matrix4 view = Cam.GetView(),project= Cam.GetProjection(), transpose_view= math::transpose(Cam.GetView());
    //auto eyePos = Cam.GetPosition(), direction = Cam.GetDirection(), up = Cam.GetUp();
    //float near_plane = Cam.GetNearPlane(), far_plane = Cam.GetFarPlane();
    //auto result = ptr->vec4, view_result= ptr->vec4,project_result = ptr->vec4;
    uint32_t index = 0;
    for (uint32_t i = 0; i < 125000; ++i, ++ptr, ++acc_ptr, ++vlc_ptr)
    {
        //result = m_FluidParticeCS_SPH.GetCurrentWolrdMat4() * ptr->vec4;
        //view_result = Cam.GetView() * result;
        //project_result = Cam.GetProjection() * view_result;
        //project_result.setXYZ(project_result.getXYZ() / project_result.getW());
        //if (ptr->vec4.getX() == 0.0 && ptr->vec4.getY() == 0.0 && ptr->vec4.getZ() == 0.0 && i>0)
        //if (ptr->vec4.getW()>0.0 && i > 0)
        if (vlc_ptr->vec4.getY() < -100.0 || vlc_ptr->vec4.getY() > 100.0)
        {
            //result = view * (ptr->vec4);  //(*ptr)
            //project_result = Cam.GetProjection() * result;
            //result = project_result / project_result.getW();
            //result = Cam.GetProjection() * result;
            //project_result.getW();
            index = i;
            break;
        }
    }
    ptr -= 125000, vlc_ptr -= 125000, acc_ptr -= 125000;
    auto a = index;
    */
    //
    

    //m_MagnifierPS
    // Keep tracking input/output resource views 
    ImgCurrentInput = pState->bUseMagnifier ? m_MagnifierPS.GetPassOutputResource() : m_GBuffer.m_HDR.Resource(); // these haven't changed, re-assign as sanity check
    SRVCurrentInput = pState->bUseMagnifier ? m_MagnifierPS.GetPassOutputSRV() : m_GBuffer.m_HDRSRV;         // these haven't changed, re-assign as sanity check

    m_CommandListRing.OnBeginFrame();

    VkCommandBuffer cmdBuf2 = m_CommandListRing.GetNewCommandList();

    {
        VkCommandBufferBeginInfo cmd_buf_info;
        cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmd_buf_info.pNext = NULL;
        cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        cmd_buf_info.pInheritanceInfo = NULL;
        VkResult res = vkBeginCommandBuffer(cmdBuf2, &cmd_buf_info);
        assert(res == VK_SUCCESS);
    }

    SetPerfMarkerBegin(cmdBuf2, "Swapchain RenderPass");

    // prepare render pass
    {
        VkRenderPassBeginInfo rp_begin = {};
        rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_begin.pNext = NULL;
        rp_begin.renderPass = pSwapChain->GetRenderPass();
        rp_begin.framebuffer = pSwapChain->GetFramebuffer(imageIndex);
        rp_begin.renderArea.offset.x = 0;
        rp_begin.renderArea.offset.y = 0;
        rp_begin.renderArea.extent.width = m_Width;
        rp_begin.renderArea.extent.height = m_Height;
        rp_begin.clearValueCount = 0;
        rp_begin.pClearValues = NULL;
        // 有点类似CommandBuffer Record一个RenderPass的函数
        // For  Scissor,Viewport,HDR/SDR,HUD|m_ImGUI
        vkCmdBeginRenderPass(cmdBuf2, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
    }

    vkCmdSetScissor(cmdBuf2, 0, 1, &m_RectScissor);
    vkCmdSetViewport(cmdBuf2, 0, 1, &m_Viewport);

    if (bHDR)
    {
        m_ColorConversionPS.Draw(cmdBuf2, SRVCurrentInput);
        m_GPUTimer.GetTimeStamp(cmdBuf2, "Color Conversion");
    }

    // For SDR pipeline, we apply the tonemapping and then draw the GUI and skip the color conversion
    else
    {
        // Tonemapping ------------------------------------------------------------------------
        {
            m_ToneMappingPS.Draw(cmdBuf2, SRVCurrentInput, pState->Exposure, pState->SelectedTonemapperIndex);
            m_GPUTimer.GetTimeStamp(cmdBuf2, "Tonemapping");
        }

        // Render HUD  -------------------------------------------------------------------------
        {
            // m_ImGUI also record CommandBuffer
            // draw GUI
            m_ImGUI.Draw(cmdBuf2);
            m_GPUTimer.GetTimeStamp(cmdBuf2, "ImGUI Rendering");
        }
    }

    SetPerfMarkerEnd(cmdBuf2);

    m_GPUTimer.OnEndFrame();

    vkCmdEndRenderPass(cmdBuf2);

    // Close & Submit the command list ----------------------------------------------------
    {
        VkResult res = vkEndCommandBuffer(cmdBuf2);
        assert(res == VK_SUCCESS);

        VkSemaphore ImageAvailableSemaphore;
        VkSemaphore RenderFinishedSemaphores;
        VkFence CmdBufExecutedFences;
        pSwapChain->GetSemaphores(&ImageAvailableSemaphore, &RenderFinishedSemaphores, &CmdBufExecutedFences);

        VkPipelineStageFlags submitWaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submit_info2;
        submit_info2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info2.pNext = NULL;
        submit_info2.waitSemaphoreCount = 1;
        submit_info2.pWaitSemaphores = &ImageAvailableSemaphore;
        submit_info2.pWaitDstStageMask = &submitWaitStage;
        submit_info2.commandBufferCount = 1;
        submit_info2.pCommandBuffers = &cmdBuf2;
        submit_info2.signalSemaphoreCount = 1;
        submit_info2.pSignalSemaphores = &RenderFinishedSemaphores;

        res = vkQueueSubmit(m_pDevice->GetGraphicsQueue(), 1, &submit_info2, CmdBufExecutedFences);
        assert(res == VK_SUCCESS);
    }
}

