
#include "stdafx.h"
#include "DynamicBufferAllocater.h"
#include "Misc/Misc.h"
#include "base/ExtDebugUtils.h"

namespace CAULDRON_VK
{
    //--------------------------------------------------------------------------------------
    //
    // OnCreate
    //
    //--------------------------------------------------------------------------------------
    VkResult DynamicBufferAllocater::OnCreate(
        Device* pDevice, uint32_t numberOfBackBuffers, uint32_t memTotalSize
        //, const VkBufferCreateInfo& buffer_CreateInfo, const BufferMemoryAllocationInfo& bufferMemory_allocationInfo
        , const VkBufferCreateInfo* pBuffer_CreateInfo
        //, const VmaAllocationCreateInfo* pVmaMemory_allocationInfo, const VkMemoryAllocateInfo* pVkMemory_allocationInfo
        , PointerConstBufferMemoryAllocationInfo p_constMemoryAllocateInfo
        , bool bUseVidMem, char* name)
    {

        VkResult res;
        m_pDevice = pDevice;
        vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(m_pDevice->GetDevice(), "vkGetBufferDeviceAddressKHR"));

        m_memTotalSize = AlignUp(memTotalSize, 256u);
        m_pData = NULL;
        m_bUseVidMem = bUseVidMem;

        m_mem.OnCreate(numberOfBackBuffers, m_memTotalSize);

#ifdef USE_VMA
        /*
        VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = m_memTotalSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        */
        VkBufferCreateInfo bufferInfo = *pBuffer_CreateInfo;
        bufferInfo.size = m_memTotalSize;

        /*
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
        allocInfo.pUserData = name;
        */
        //VmaAllocationCreateInfo allocInfo = bufferMemory_allocationInfo.vmaAllocation_usage;
        //VmaAllocationCreateInfo allocInfo = *pVmaMemory_allocationInfo;
        VmaAllocationCreateInfo allocInfo = *p_constMemoryAllocateInfo.pVmaAllocationInfo;
        //allocInfo.pUserData = (void*)name;
        //
        //m_vmaMemoryAllocationUsage = allocInfo.usage;
        /*
        if (allocInfo.flags == VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT)
        {
            // Custom general-purpose Pointer that will be stored in VmaAllocation, can be read as VmaAllocationInfo::pUserData
            //  and changed using vmaSetAllocationUserData().
            // If VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT is used in .flags, it must be either Null or Pointer to a null-terminated String. 
            //  The string will be then copied to internal buffer, so it doesn't need to be valid after allocation call.
            allocInfo.pUserData = name;
        }
        */

        //vmaFindMemoryTypeIndex
        //vmaFindMemoryTypeIndexForBufferInfo
        //VmaPoolCreateInfo
        //VmaPool

        res = vmaCreateBuffer(pDevice->GetAllocator(), &bufferInfo, &allocInfo, &m_buffer, &m_bufferAlloc, nullptr);
        assert(res == VK_SUCCESS);
        SetResourceName(pDevice->GetDevice(), VK_OBJECT_TYPE_BUFFER, (uint64_t)m_buffer, "DynamicBufferRing");

        /*
        res = vmaMapMemory(pDevice->GetAllocator(), m_bufferAlloc, (void**)&m_pData);
        assert(res == VK_SUCCESS);
        */
        //m_vmaMemoryAllocationUsage = allocInfo.usage;
        //if (m_vmaMemoryAllocationUsage != VMA_MEMORY_USAGE_GPU_ONLY && m_vmaMemoryAllocationUsage != VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED)
        if (allocInfo.usage != VMA_MEMORY_USAGE_GPU_ONLY && allocInfo.usage != VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED)
        {
            res = vmaMapMemory(pDevice->GetAllocator(), m_bufferAlloc, (void**)&m_pData);
            assert(res == VK_SUCCESS);
        }
        else
        {
            VkBufferDeviceAddressInfo   buffer_deviceAddressInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
            buffer_deviceAddressInfo.buffer = m_buffer;
            //m_deviceAddressData = vkGetBufferDeviceAddress(pDevice->GetDevice(), &buffer_deviceAddressInfo);
            m_deviceAddressData = vkGetBufferDeviceAddressKHR(pDevice->GetDevice(), &buffer_deviceAddressInfo);
            assert(m_deviceAddressData);
            //
            m_bOnlyGPU = true;
        }
        //m_bufferMemoryAllocationUsage.vmaAllocation_usage = allocInfo.usage;
#else
        // create a buffer that can host uniforms, indices and vertexbuffers
        /*
        VkBufferCreateInfo buf_info = {};
        buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buf_info.pNext = NULL;
        buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        buf_info.size = m_memTotalSize;
        buf_info.queueFamilyIndexCount = 0;
        buf_info.pQueueFamilyIndices = NULL;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buf_info.flags = 0;
        */
        VkBufferCreateInfo buf_info = *pBuffer_CreateInfo;
        buf_info.size = m_memTotalSize;

        res = vkCreateBuffer(m_pDevice->GetDevice(), &buf_info, NULL, &m_buffer);
        assert(res == VK_SUCCESS);

        VkMemoryRequirements mem_reqs;
        vkGetBufferMemoryRequirements(m_pDevice->GetDevice(), m_buffer, &mem_reqs);

        /*
        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.pNext = NULL;
        alloc_info.memoryTypeIndex = 0;
        alloc_info.memoryTypeIndex = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        alloc_info.allocationSize = mem_reqs.size;
        alloc_info.memoryTypeIndex = 0;
        */
        //VkMemoryAllocateInfo alloc_info = bufferMemory_allocationInfo.vkMemoryAllocateInfo;
        //VkMemoryAllocateInfo alloc_info = *pVkMemory_allocationInfo;
        VkMemoryAllocateInfo alloc_info = *p_constMemoryAllocateInfo.pVkMemoryAllocateInfo;
        alloc_info.allocationSize = mem_reqs.size;
        //alloc_info.memoryTypeIndex = 0;
        //m_vkMemoryAllocationUsage = memoryTypeIndex;
        /*
        bool pass = memory_type_from_properties(m_pDevice->GetPhysicalDeviceMemoryProperties(), mem_reqs.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &alloc_info.memoryTypeIndex);
        */
        //bufferMemory_allocationInfo.vkMemoryAllocateInfo.memoryTypeIndex,
        //VkFlags requirement_mask= pVkMemory_allocationInfo->memoryTypeIndex;
        bool pass = memory_type_from_properties(m_pDevice->GetPhysicalDeviceMemoryProperties(), mem_reqs.memoryTypeBits,
            //pVkMemory_allocationInfo->memoryTypeIndex,
            alloc_info.memoryTypeIndex,
            &alloc_info.memoryTypeIndex);
        //
        assert(pass && "No mappable, coherent memory");

        res = vkAllocateMemory(m_pDevice->GetDevice(), &alloc_info, NULL, &m_deviceMemory);
        assert(res == VK_SUCCESS);

        /*
        res = vkMapMemory(m_pDevice->GetDevice(), m_deviceMemory, 0, mem_reqs.size, 0, (void**)&m_pData);
        assert(res == VK_SUCCESS);

        res = vkBindBufferMemory(m_pDevice->GetDevice(), m_buffer, m_deviceMemory, 0);
        assert(res == VK_SUCCESS);
        */

        res = vkBindBufferMemory(m_pDevice->GetDevice(), m_buffer, m_deviceMemory, 0);
        assert(res == VK_SUCCESS);

        //if (bufferMemory_allocationInfo.vkMemoryAllocateInfo.memoryTypeIndex & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        //if (m_vkMemoryAllocationUsage & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        //if(pVkMemory_allocationInfo->memoryTypeIndex & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        if (p_constMemoryAllocateInfo.pVkMemoryAllocateInfo->memoryTypeIndex & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            res = vkMapMemory(m_pDevice->GetDevice(), m_deviceMemory, 0, mem_reqs.size, 0, (void**)&m_pData);
            assert(res == VK_SUCCESS);
        }
        else
        {
            VkBufferDeviceAddressInfo   buffer_deviceAddressInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
            buffer_deviceAddressInfo.buffer = m_buffer;
            //m_deviceAddressData = vkGetBufferDeviceAddress(pDevice->GetDevice(), &buffer_deviceAddressInfo);
            m_deviceAddressData = vkGetBufferDeviceAddressKHR(pDevice->GetDevice(), &buffer_deviceAddressInfo);
            assert(m_deviceAddressData);
            //
            m_bOnlyGPU = true;
        }
        //m_vkMemoryAllocationUsage = pVkMemory_allocationInfo->memoryTypeIndex;
        //m_bufferMemoryAllocationUsage.vkMemoryAllocation_usage = bufferMemory_allocationInfo.vkMemoryAllocateInfo.memoryTypeIndex;
        
#endif
        //TODO: m_bUseVidMem

        return res;
    }

    //--------------------------------------------------------------------------------------
    //
    // OnDestroy
    //
    //--------------------------------------------------------------------------------------
    void DynamicBufferAllocater::OnDestroy()
    {
#ifdef USE_VMA
        //vmaUnmapMemory(m_pDevice->GetAllocator(), m_bufferAlloc);
        //if (m_bufferMemoryAllocationUsage.vmaAllocation_usage != VMA_MEMORY_USAGE_GPU_ONLY
        //    && m_bufferMemoryAllocationUsage.vmaAllocation_usage != VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED)
        //if (m_vmaMemoryAllocationUsage != VMA_MEMORY_USAGE_GPU_ONLY
        //    && m_vmaMemoryAllocationUsage != VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED)
        if(!m_bOnlyGPU)
        {
            vmaUnmapMemory(m_pDevice->GetAllocator(), m_bufferAlloc);
        }
        vmaDestroyBuffer(m_pDevice->GetAllocator(), m_buffer, m_bufferAlloc);
#else
        //vkUnmapMemory(m_pDevice->GetDevice(), m_deviceMemory);
        //if (m_bufferMemoryAllocationUsage.vkMemoryAllocateInfo.memoryTypeIndex & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        //if (m_vkMemoryAllocationUsage & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        if(!m_bOnlyGPU)
        {
            vkUnmapMemory(m_pDevice->GetDevice(), m_deviceMemory);
        }
        vkFreeMemory(m_pDevice->GetDevice(), m_deviceMemory, NULL);
        vkDestroyBuffer(m_pDevice->GetDevice(), m_buffer, NULL);
#endif
        m_mem.OnDestroy();
    }

    //Deprecate
    //--------------------------------------------------------------------------------------
    //
    // GetBufferDeviceAddress
    //
    //--------------------------------------------------------------------------------------
    //bool GetBufferDeviceAddress(VkDescriptorBufferInfo bufferInfo, VkDeviceAddress* pOut);
    
    
    //--------------------------------------------------------------------------------------
    //
    // AllocConstantBuffer
    //
    //--------------------------------------------------------------------------------------
    bool DynamicBufferAllocater::AllocBuffer(uint32_t size, VkDescriptorBufferInfo* pOut)
    {

        size = AlignUp(size, 256u);

        uint32_t memOffset;
        if (m_mem.Alloc(size, &memOffset) == false)
        {
            assert("Ran out of mem for 'dynamic' buffers, please increase the allocated size");
            return false;
        }
        
        pOut->buffer = m_bUseVidMem ? m_bufferVid : m_buffer;
        pOut->offset = memOffset;
        pOut->range = size;

        return true;
    }


    //--------------------------------------------------------------------------------------
    //
    // AllocConstantBuffer
    //
    //--------------------------------------------------------------------------------------
    bool DynamicBufferAllocater::AllocBuffer(
        uint32_t size, void** ppBuffer
        , VkDeviceAddress* pDeviceAddressBuffer
        , VkDescriptorBufferInfo* pOut)
    {
        //std::lock_guard<std::mutex> lock(m_mutex);

        size = AlignUp(size, 256u);

        uint32_t memOffset;
        if (m_mem.Alloc(size, &memOffset) == false)
        {
            assert("Ran out of mem for 'dynamic' buffers, please increase the allocated size");
            return false;
        }
        
/*
#ifdef USE_VMA
        //if (m_vmaMemoryAllocationUsage != VMA_MEMORY_USAGE_GPU_ONLY
        //    && m_vmaMemoryAllocationUsage != VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED)
        if(!m_bOnlyGPU)
        {
            *pBuffer = (void*)(m_pData + memOffset);
        }
        else
        {
            *pDeviceAddressBuffer = m_deviceAddressData + memOffset;
        }
#else
        //if (m_vkMemoryAllocationUsage & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        if(!m_bOnlyGPU)
        {
            *pBuffer = (void*)(m_pData + memOffset);
        }
        else
        {
            *pDeviceAddressBuffer = m_deviceAddressData + memOffset;
        }
#endif
*/
        //TODO
        if (!m_bOnlyGPU)
        {
            *ppBuffer = (void*)(m_pData + memOffset);
        }
        //else
        if (pDeviceAddressBuffer != NULL)
        {
            *pDeviceAddressBuffer = m_deviceAddressData + memOffset;
        }

        
        //pOut->buffer = m_buffer;
        pOut->buffer = m_bUseVidMem ? m_bufferVid : m_buffer;
        pOut->offset = memOffset;
        pOut->range = size;

        return true;
    }

    //--------------------------------------------------------------------------------------
    //
    // AllocConstantBuffer
    //
    //--------------------------------------------------------------------------------------
    // VkDescriptorBufferInfo DynamicBufferAllocater::AllocConstantBuffer()
    bool DynamicBufferAllocater::AllocBuffer(
        uint32_t size
        , void* pInitData
        , VkDeviceAddress* pDeviceAddressBuffer
        , VkDescriptorBufferInfo* pInitData_DeviceBufferInfo
        , VkDescriptorBufferInfo* pOut)
    {
        void* pBuffer;
        //VkDeviceAddress deviceAddressBuffer;
        //VkDescriptorBufferInfo out;
        //if (AllocConstantBuffer(size, &pBuffer, &out))
        if (AllocBuffer(size, &pBuffer, pDeviceAddressBuffer, pOut))
        {
            //TODO
            if (!m_bOnlyGPU)
            {
                //memcpy(pBuffer, pData, size);
                memcpy(pBuffer, pInitData, size);
            }
            //
            else
            //if (pDeviceAddressBuffer != NULL)
            {
                //TODO
                // CopyAccordingToDeviceAddress?
                // vkCmdCopyBuffer(src= *pInitData_DeviceBufferInfo, dst=pDeviceAddressBuffer)
            }
            //
            return true;
            //
/*
#ifdef USE_VMA
            //if (m_vmaMemoryAllocationUsage != VMA_MEMORY_USAGE_GPU_ONLY
            //    && m_vmaMemoryAllocationUsage != VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED)
            if(!m_bOnlyGPU)
            {
                memcpy(pBuffer, pData, size);
            }
            //
            //TODO
            else
            {
                // CopyAccordingToDeviceAddress?
                // vkCmdCopyBuffer()
            }
            
#else
            //if (m_vkMemoryAllocationUsage & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            if(!m_bOnlyGPU)
            {
                memcpy(pBuffer, pData, size);
            }
            //
            //TODO
            else
            {
                // CopyAccordingToDeviceAddress?
                // vkCmdCopyBuffer()
            }
#endif
*/
        }

        //return out;
        return false;
        //
    }

    //--------------------------------------------------------------------------------------
    //
    // AllocVertexBuffer
    //
    //--------------------------------------------------------------------------------------
    bool DynamicBufferAllocater::AllocVertexBuffer(uint32_t numbeOfVertices, uint32_t strideInBytes, void** pData, VkDeviceAddress* pDeviceAddressBuffer, VkDescriptorBufferInfo* pOut)
    {
        //return AllocConstantBuffer(numbeOfVertices * strideInBytes, pData, pOut);
        return AllocBuffer(numbeOfVertices * strideInBytes, pData, pDeviceAddressBuffer, pOut);
    }

    bool DynamicBufferAllocater::AllocIndexBuffer(uint32_t numbeOfIndices, uint32_t strideInBytes, void** pData,  VkDeviceAddress* pDeviceAddressBuffer, VkDescriptorBufferInfo* pOut)
    {
        //return AllocConstantBuffer(numbeOfIndices * strideInBytes, pData, pOut);
        return AllocBuffer(numbeOfIndices * strideInBytes, pData, pDeviceAddressBuffer, pOut);
    }

    //--------------------------------------------------------------------------------------
    //
    // OnBeginFrame
    //
    //--------------------------------------------------------------------------------------
    void DynamicBufferAllocater::OnBeginFrame()
    {
        m_mem.OnBeginFrame();
    }

    void DynamicBufferAllocater::SetDescriptorSet(int index, uint32_t size, VkDescriptorSet descriptorSet)
    {
        VkDescriptorBufferInfo out = {};
        out.buffer = m_buffer;
        out.offset = 0;
        out.range = size;

        VkWriteDescriptorSet write;
        write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = NULL;
        write.dstSet = descriptorSet;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        write.pBufferInfo = &out;
        write.dstArrayElement = 0;
        write.dstBinding = index;

        vkUpdateDescriptorSets(m_pDevice->GetDevice(), 1, &write, 0, NULL);
    }
}
