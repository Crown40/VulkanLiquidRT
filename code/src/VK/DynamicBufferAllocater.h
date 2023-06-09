
#pragma once

#include "base/Device.h"
#include "Misc/Ring.h"
#include "../VulkanMemoryAllocator/vk_mem_alloc.h"
#include "BufferAllocater.h"

namespace CAULDRON_VK
{
    // This class mimics the behaviour or the DX11 dynamic buffers. I can hold uniforms, index and vertex buffers.
    // It does so by suballocating memory from a huge buffer. The buffer is used in a ring fashion.  
    // Allocated memory is taken from the tail, freed memory makes the head advance;
    // See 'ring.h' to get more details on the ring buffer.
    //
    // The class knows when to free memory by just knowing:
    //    1) the amount of memory used per frame
    //    2) the number of backbuffers 
    //    3) When a new frame just started ( indicated by OnBeginFrame() )
    //         - This will free the data of the oldest frame so it can be reused for the new frame
    //
    // Note than in this ring an allocated chuck of memory has to be contiguous in memory, that is it cannot spawn accross the tail and the head.
    // This class takes care of that.

    // DynamicBufferRing
    class DynamicBufferAllocater
    {
    public:
        //VkResult OnCreate(Device* pDevice, uint32_t numberOfBackBuffers, uint32_t memTotalSize, char* name = NULL);
        VkResult OnCreate(Device* pDevice, uint32_t numberOfBackBuffers, uint32_t memTotalSize
            , const VkBufferCreateInfo* pBuffer_CreateInfo
            //, const VmaAllocationCreateInfo* pVmaMemory_allocationInfo, const VkMemoryAllocateInfo* pVkMemory_allocationInfo
            , PointerConstBufferMemoryAllocationInfo p_constMemoryAllocateInfo
            , bool bUseVidMem = false, char* name = NULL);
        void OnDestroy();
        //
        bool AllocBuffer(uint32_t size, VkDescriptorBufferInfo* pOut);
        //
        //bool AllocConstantBuffer(uint32_t size, void **pData, VkDescriptorBufferInfo *pOut);
        bool AllocBuffer(uint32_t size, void** ppBuffer, VkDeviceAddress* pDeviceAddressBuffer
            , VkDescriptorBufferInfo* pOut);
        //VkDescriptorBufferInfo AllocConstantBuffer(uint32_t size, void* pData);
        // Call bool AllocConstantBuffer() beyond: assert(whetherSuccessfulAllocate)
        //VkDescriptorBufferInfo AllocConstantBuffer(uint32_t size, void* pInitData, VkDeviceAddress* pDeviceAddressBuffer, VkDescriptorBufferInfo* pDeviceBufferInfoInitData);
        bool AllocBuffer(uint32_t size, void* pInitData
            , VkDeviceAddress* pDeviceAddressBuffer
            , VkDescriptorBufferInfo* pInitData_DeviceBufferInfo
            , VkDescriptorBufferInfo* pOut);
        //
        //bool AllocVertexBuffer(uint32_t numbeOfVertices, uint32_t strideInBytes, void** pData, VkDescriptorBufferInfo* pOut);
        bool AllocVertexBuffer(uint32_t numbeOfVertices, uint32_t strideInBytes, void** pData, VkDeviceAddress* pDeviceAddressBuffer, VkDescriptorBufferInfo* pOut);
        //bool AllocIndexBuffer(uint32_t numbeOfIndices, uint32_t strideInBytes, void** pData, VkDescriptorBufferInfo* pOut);
        bool AllocIndexBuffer(uint32_t numbeOfIndices, uint32_t strideInBytes, void** pData, VkDeviceAddress* pDeviceAddressBuffer, VkDescriptorBufferInfo* pOut);
        void OnBeginFrame();
        void SetDescriptorSet(int i, uint32_t size, VkDescriptorSet descriptorSet);

    private:
        PFN_vkGetBufferDeviceAddressKHR					vkGetBufferDeviceAddressKHR;

        Device* m_pDevice;
        std::mutex      m_mutex = {};
        //
        uint32_t        m_memTotalSize;
        RingWithTabs    m_mem;
        //
        // <=> const void*: hostAddress
        char*           m_pData = nullptr;
        // <=> VkDeviceAddress: deviceAddress: 0==null_pointer
        uint64_t        m_deviceAddressData = 0;
        //
        bool            m_bUseVidMem = false;
        VkBuffer        m_buffer = VK_NULL_HANDLE;
        VkBuffer        m_bufferVid = VK_NULL_HANDLE;
        //
        bool            m_bOnlyGPU = false;
        bool            m_bOnlyGPUVid = false;
        //Deprecate
        //BufferMemoryAllocationUsage     m_bufferMemoryAllocationUsage;

#ifdef USE_VMA
        VmaAllocation           m_bufferAlloc = VK_NULL_HANDLE;
        VmaAllocation           m_bufferAllocVid = VK_NULL_HANDLE;
        //
        //https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/group__group__alloc.html#gaa5846affa1e9da3800e3e78fae2305cc
        // <=> VmaMemoryUsage{enum: No VkFlags}
        //Deprecate
        //VmaMemoryUsage          m_vmaMemoryAllocationUsage;
#else
        VkDeviceMemory          m_deviceMemory = VK_NULL_HANDLE;
        VkDeviceMemory          m_deviceMemoryVid = VK_NULL_HANDLE;;
        //
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkMemoryPropertyFlagBits.html
        // <=> VkMemoryPropertyFlagBits
        //typedef VkFlags|uint32_t VkMemoryPropertyFlags
        //Deprecate
        //VkMemoryPropertyFlags   m_vkMemoryAllocationUsage;
#endif
    };
}