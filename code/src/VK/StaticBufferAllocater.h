
#pragma once

#include "base/Device.h"
//#include "base/ResourceViewHeaps.h"
#include "../VulkanMemoryAllocator/vk_mem_alloc.h"
#include "BufferAllocater.h"

namespace CAULDRON_VK
{
    // Simulates DX11 style static buffers. For dynamic buffers please see 'DynamicBufferRingDX12.h'
    //
    // This class allows suballocating small chuncks of memory from a huge buffer that is allocated on creation 
    // This class is specialized in vertex and index buffers. 
    //
    class StaticBufferAllocater
    {
    public:
        //VkResult OnCreate(Device* pDevice, uint32_t totalMemSize, bool bUseVidMem, const char* name);
        VkResult OnCreate(Device* pDevice, uint32_t totalMemSize
            , const VkBufferCreateInfo* pBuffer_CreateInfo, const VkBufferCreateInfo* pVidBuffer_CreateInfo
            //, const VmaAllocationCreateInfo* pVmaMemory_allocationInfo, const VkMemoryAllocateInfo* pVkMemory_allocationInfo
            , PointerConstBufferMemoryAllocationInfo p_constMemoryAllocateInfo, PointerConstBufferMemoryAllocationInfo pVid_constMemoryAllocateInfo
            , bool bUseVidMem, const char* name);
        void OnDestroy();

        //
        bool AllocBuffer(uint32_t numbeOfElements, uint32_t strideInBytes, VkDescriptorBufferInfo* pOut);
         
        // Allocates a IB/VB and returns a pointer to fill it + a descriptot
        //
        //bool AllocBuffer(uint32_t numbeOfVertices, uint32_t strideInBytes, void** pData, VkDescriptorBufferInfo* pOut);
        bool AllocBuffer(uint32_t numbeOfElements, uint32_t strideInBytes
            , void** ppBuffer, VkDeviceAddress* pDeviceAddressBuffer
            , VkDescriptorBufferInfo* pOut);
        
        //
        // Allocates a IB/VB and fill it with pInitData, returns a descriptor
        // Call bool AllocBuffer(void** pBuffer) beyond: use return-value for whetherSuccessfulAllocate
        //bool AllocBuffer(uint32_t numbeOfIndices, uint32_t strideInBytes, const void* pInitData, VkDescriptorBufferInfo* pOut);
        bool AllocBuffer(uint32_t numbeOfElements, uint32_t strideInBytes
            , const void* pInitData
            , VkDeviceAddress* pDeviceAddressBuffer
            , VkDescriptorBufferInfo* pInitData_deviceBufferInfo
            , VkDescriptorBufferInfo* pOut);

        //
        // if using vidmem this kicks the upload from the upload heap to the video mem
        void UploadData(VkCommandBuffer cmd_buf);

        // if using vidmem frees the upload heap
        void FreeUploadHeap();

    private:
        PFN_vkGetBufferDeviceAddressKHR					vkGetBufferDeviceAddressKHR;

        Device* m_pDevice;
        std::mutex       m_mutex = {};
        //
        uint32_t         m_totalMemSize = 0;
        uint32_t         m_memOffset = 0;
        //
        // <=> const void*: hostAddress
        char*            m_pData = nullptr;
        // <=> VkDeviceAddress: deviceAddress: 0==null_pointer
        uint64_t         m_deviceAddressData = 0;
        //
        bool             m_bUseVidMem = false;  //true
        VkBuffer         m_buffer = VK_NULL_HANDLE;
        VkBuffer         m_bufferVid = VK_NULL_HANDLE;
        //
        bool             m_bOnlyGPU = false;
        bool             m_bOnlyGPUVid = false;
        //
#ifdef USE_VMA
        VmaAllocation           m_bufferAlloc = VK_NULL_HANDLE;
        VmaAllocation           m_bufferAllocVid = VK_NULL_HANDLE;
        //
        //https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/group__group__alloc.html#gaa5846affa1e9da3800e3e78fae2305cc
        // <=> VmaMemoryUsage{enum: No VkFlags}
        //Deprecate
        //VmaMemoryUsage          m_vmaMemoryAllocationUsage;
#else
        VkDeviceMemory            m_deviceMemory = VK_NULL_HANDLE;;
        VkDeviceMemory            m_deviceMemoryVid = VK_NULL_HANDLE;;
        //
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkMemoryPropertyFlagBits.html
        // <=> VkMemoryPropertyFlagBits
        //typedef VkFlags|uint32_t VkMemoryPropertyFlags
        //Deprecate
        //VkMemoryPropertyFlags   m_vkMemoryAllocationUsage;
#endif
    };
}

