
#include "stdafx.h"
#include "Misc/Misc.h"
#include "StaticBufferAllocater.h"
#include "base/ExtDebugUtils.h"

namespace CAULDRON_VK
{
    VkResult StaticBufferAllocater::OnCreate(
        Device* pDevice, uint32_t totalMemSize
        //, const VkBufferCreateInfo* pBuffer_CreateInfo
        , const VkBufferCreateInfo* pBuffer_CreateInfo, const VkBufferCreateInfo* pVidBuffer_CreateInfo
        //, const VmaAllocationCreateInfo* pVmaMemory_allocationInfo, const VkMemoryAllocateInfo* pVkMemory_allocationInfo
        , PointerConstBufferMemoryAllocationInfo p_constMemoryAllocateInfo, PointerConstBufferMemoryAllocationInfo pVid_constMemoryAllocateInfo
        , bool bUseVidMem, const char* name)
    {

        VkResult res;
        m_pDevice = pDevice;
        vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(m_pDevice->GetDevice(), "vkGetBufferDeviceAddressKHR"));

        m_totalMemSize = totalMemSize;
        m_memOffset = 0;
        m_pData = NULL;
        m_bUseVidMem = bUseVidMem;

#ifdef USE_VMA
        /*
        VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = m_totalMemSize;
        bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (bUseVidMem)
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        */
        VkBufferCreateInfo bufferInfo = *pBuffer_CreateInfo;
        bufferInfo.size = m_totalMemSize;
        if (m_bUseVidMem)
        {
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }

        /*
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
        allocInfo.pUserData = (void*)name;
        */
        //VmaAllocationCreateInfo allocInfo = *pVmaMemory_allocationInfo;
        VmaAllocationCreateInfo allocInfo = *p_constMemoryAllocateInfo.pVmaAllocationInfo;
        allocInfo.pUserData = (void*)name;
        //
        //m_vmaMemoryAllocationUsage = allocInfo.usage;

        res = vmaCreateBuffer(pDevice->GetAllocator(), &bufferInfo, &allocInfo, &m_buffer, &m_bufferAlloc, nullptr);
        assert(res == VK_SUCCESS);
        SetResourceName(pDevice->GetDevice(), VK_OBJECT_TYPE_BUFFER, (uint64_t)m_buffer, "StaticBufferPool (sys mem)");

        /*
        res = vmaMapMemory(pDevice->GetAllocator(), m_bufferAlloc, (void**)&m_pData);
        assert(res == VK_SUCCESS);
        */
        //m_vmaMemoryAllocationUsage = allocInfo.usage;
        //if (m_vmaMemoryAllocationUsage != VMA_MEMORY_USAGE_GPU_ONLY 
        //    && m_vmaMemoryAllocationUsage != VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED)
        if (allocInfo.usage != VMA_MEMORY_USAGE_GPU_ONLY
            && allocInfo.usage != VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED)
        {
            res = vmaMapMemory(pDevice->GetAllocator(), m_bufferAlloc, (void**)&m_pData);
            assert(res == VK_SUCCESS);
        }
        else
        {
            //
            m_bOnlyGPU = true;
        }
        //Buffer-DeviceAddress
        {
            VkBufferDeviceAddressInfo   buffer_deviceAddressInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
            buffer_deviceAddressInfo.buffer = m_buffer;
            //m_deviceAddressData = vkGetBufferDeviceAddress(pDevice->GetDevice(), &buffer_deviceAddressInfo);
            m_deviceAddressData = vkGetBufferDeviceAddressKHR(pDevice->GetDevice(), &buffer_deviceAddressInfo);
            assert(m_deviceAddressData);
        }
        //m_vmaMemoryAllocationUsage = allocInfo.usage;
        //
#else
        // create the buffer, allocate it in SYSTEM memory, bind it and map it
        /*
        VkBufferCreateInfo buf_info = {};
        buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buf_info.pNext = NULL;
        buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (bUseVidMem)
            buf_info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buf_info.size = m_totalMemSize;
        buf_info.queueFamilyIndexCount = 0;
        buf_info.pQueueFamilyIndices = NULL;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buf_info.flags = 0;
        */
        VkBufferCreateInfo buf_info = *pBuffer_CreateInfo;
        buf_info.size = m_totalMemSize;
        if (m_bUseVidMem)
        {
            buf_info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }

        res = vkCreateBuffer(m_pDevice->GetDevice(), &buf_info, NULL, &m_buffer);
        assert(res == VK_SUCCESS);

        // allocate the buffer in system memory

        VkMemoryRequirements mem_reqs;
        vkGetBufferMemoryRequirements(m_pDevice->GetDevice(), m_buffer, &mem_reqs);

        /*
        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.pNext = NULL;
        alloc_info.memoryTypeIndex = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        alloc_info.allocationSize = mem_reqs.size;
        */
        //VkMemoryAllocateInfo alloc_info = *pVkMemory_allocationInfo;
        VkMemoryAllocateInfo alloc_info = *p_constMemoryAllocateInfo.pVkMemoryAllocateInfo;
        alloc_info.allocationSize = mem_reqs.size;
        //alloc_info.memoryTypeIndex = 0;
        //m_vkMemoryAllocationUsage = alloc_info.memoryTypeIndex;
        /*
        bool pass = memory_type_from_properties(m_pDevice->GetPhysicalDeviceMemoryProperties(), mem_reqs.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &alloc_info.memoryTypeIndex);
        */
        bool pass = memory_type_from_properties(m_pDevice->GetPhysicalDeviceMemoryProperties(), mem_reqs.memoryTypeBits,
            //pVkMemory_allocationInfo->memoryTypeIndex,
            //m_vkMemoryAllocationUsage,
            alloc_info.memoryTypeIndex,
            &alloc_info.memoryTypeIndex);
        assert(pass && "No mappable, coherent memory");

        res = vkAllocateMemory(m_pDevice->GetDevice(), &alloc_info, NULL, &m_deviceMemory);
        assert(res == VK_SUCCESS);

        /*
        // bind buffer

        res = vkBindBufferMemory(m_pDevice->GetDevice(), m_buffer, m_deviceMemory, 0);
        assert(res == VK_SUCCESS);

        // Map it and leave it mapped. This is fine for Win10 and Win7.

        res = vkMapMemory(m_pDevice->GetDevice(), m_deviceMemory, 0, mem_reqs.size, 0, (void**)&m_pData);
        assert(res == VK_SUCCESS);
        */

        res = vkBindBufferMemory(m_pDevice->GetDevice(), m_buffer, m_deviceMemory, 0);
        assert(res == VK_SUCCESS);

        //if (m_vkMemoryAllocationUsage & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        if (p_constMemoryAllocateInfo.pVkMemoryAllocateInfo->memoryTypeIndex & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            res = vkMapMemory(m_pDevice->GetDevice(), m_deviceMemory, 0, mem_reqs.size, 0, (void**)&m_pData);
            assert(res == VK_SUCCESS);
        }
        else
        {
            //
            m_bOnlyGPU = true;
        }
        // Buffer-DeviceAddress
        {
            VkBufferDeviceAddressInfo   buffer_deviceAddressInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
            buffer_deviceAddressInfo.buffer = m_buffer;
            //m_deviceAddressData = vkGetBufferDeviceAddress(pDevice->GetDevice(), &buffer_deviceAddressInfo);
            m_deviceAddressData = vkGetBufferDeviceAddressKHR(pDevice->GetDevice(), &buffer_deviceAddressInfo);
            assert(m_deviceAddressData);
        }
        
        //m_vkMemoryAllocationUsage = pVkMemory_allocationInfo->memoryTypeIndex;
        //
#endif

        if (m_bUseVidMem)
        {
#ifdef USE_VMA
            /*
            VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            bufferInfo.size = m_totalMemSize;
            bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            */
            VkBufferCreateInfo bufferInfo = *pVidBuffer_CreateInfo;
            bufferInfo.size = m_totalMemSize;

            /*
            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            allocInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
            allocInfo.pUserData = (void*)name;
            */
            VmaAllocationCreateInfo allocInfo = *pVid_constMemoryAllocateInfo.pVmaAllocationInfo;
            //allocInfo.pUserData = (void*)name;

            res = vmaCreateBuffer(pDevice->GetAllocator(), &bufferInfo, &allocInfo, &m_bufferVid, &m_bufferAllocVid, nullptr);
            assert(res == VK_SUCCESS);
            SetResourceName(pDevice->GetDevice(), VK_OBJECT_TYPE_BUFFER, (uint64_t)m_buffer, "StaticBufferPool (vid mem)");

            /*
            //TODO
            if (pVid_constMemoryAllocateInfo.pVmaAllocationInfo->usage != VMA_MEMORY_USAGE_GPU_ONLY 
                && pVid_constMemoryAllocateInfo.pVmaAllocationInfo->usage != VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED)
            {
                res = vmaMapMemory(pDevice->GetAllocator(), m_bufferAllocVid, (void**)&m_pDataVid);
                assert(res == VK_SUCCESS);
            }
            else
            {
                VkBufferDeviceAddressInfo   buffer_deviceAddressInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
                buffer_deviceAddressInfo.buffer = m_bufferVid;
                //vkGetBufferDeviceAddressKHR
                m_deviceAddressDataVid = vkGetBufferDeviceAddress(pDevice->GetDevice(), &buffer_deviceAddressInfo);
                assert(m_deviceAddressDataVid);
                //
                m_bOnlyGPU = true;
            }
            */
            //
#else

            // create the buffer, allocate it in VIDEO memory and bind it 
            /*
            VkBufferCreateInfo buf_info = {};
            buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buf_info.pNext = NULL;
            buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            buf_info.size = m_totalMemSize;
            buf_info.queueFamilyIndexCount = 0;
            buf_info.pQueueFamilyIndices = NULL;
            buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buf_info.flags = 0;
            */
            VkBufferCreateInfo buf_info = *pVidBuffer_CreateInfo;
            buf_info.size = m_totalMemSize;
            //
            buf_info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

            res = vkCreateBuffer(m_pDevice->GetDevice(), &buf_info, NULL, &m_bufferVid);
            assert(res == VK_SUCCESS);

            // allocate the buffer in VIDEO memory

            VkMemoryRequirements mem_reqs;
            vkGetBufferMemoryRequirements(m_pDevice->GetDevice(), m_bufferVid, &mem_reqs);

            /*
            VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.pNext = NULL;
            alloc_info.memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            alloc_info.allocationSize = mem_reqs.size;
            */
            VkMemoryAllocateInfo alloc_info = *pVid_constMemoryAllocateInfo.pVkMemoryAllocateInfo;
            alloc_info.allocationSize = mem_reqs.size;
            //alloc_info.memoryTypeIndex = 0;

            /*
            bool pass = memory_type_from_properties(m_pDevice->GetPhysicalDeviceMemoryProperties(), mem_reqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                &alloc_info.memoryTypeIndex);
            */
            bool pass = memory_type_from_properties(m_pDevice->GetPhysicalDeviceMemoryProperties(), mem_reqs.memoryTypeBits,
                alloc_info.memoryTypeIndex,
                &alloc_info.memoryTypeIndex);
            //
            assert(pass && "No mappable, coherent memory");

            res = vkAllocateMemory(m_pDevice->GetDevice(), &alloc_info, NULL, &m_deviceMemoryVid);
            assert(res == VK_SUCCESS);

            // bind buffer

            res = vkBindBufferMemory(m_pDevice->GetDevice(), m_bufferVid, m_deviceMemoryVid, 0);
            assert(res == VK_SUCCESS);

            /*
            //TODO: May not Map
            if (pVid_constMemoryAllocateInfo.pVkMemoryAllocateInfo->memoryTypeIndex & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                res = vkMapMemory(m_pDevice->GetDevice(), m_deviceMemoryVid, 0, mem_reqs.size, 0, (void**)&m_pDataVid);
                assert(res == VK_SUCCESS);
            }
            else
            {
                VkBufferDeviceAddressInfo   buffer_deviceAddressInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
                buffer_deviceAddressInfo.buffer = m_bufferVid;
                //vkGetBufferDeviceAddressKHR
                m_deviceAddressDataVid = vkGetBufferDeviceAddress(pDevice->GetDevice(), &buffer_deviceAddressInfo);
                assert(m_deviceAddressDataVid);
                //
                m_bOnlyGPU = true;
            }
            */

            //

#endif
        }



        return res;

    }

    void StaticBufferAllocater::OnDestroy()
    {
        if (m_bUseVidMem && m_bufferVid)
        {
#ifdef USE_VMA
            //TODO: vmaUnmapMemory: may not Map
            vmaDestroyBuffer(m_pDevice->GetAllocator(), m_bufferVid, m_bufferAllocVid);
#else
            //TODO: vkUnmapMemory: may not Map
            vkFreeMemory(m_pDevice->GetDevice(), m_deviceMemoryVid, NULL);
            vkDestroyBuffer(m_pDevice->GetDevice(), m_bufferVid, NULL);
#endif
            m_bufferVid = VK_NULL_HANDLE;
        }

        if (m_buffer != VK_NULL_HANDLE)
        {
#ifdef USE_VMA
            if (!m_bOnlyGPU)
            {
                vmaUnmapMemory(m_pDevice->GetAllocator(), m_bufferAlloc);
            }
            vmaDestroyBuffer(m_pDevice->GetAllocator(), m_buffer, m_bufferAlloc);
#else
            if (!m_bOnlyGPU)
            {
                vkUnmapMemory(m_pDevice->GetDevice(), m_deviceMemory);
            }
            vkFreeMemory(m_pDevice->GetDevice(), m_deviceMemory, NULL);
            vkDestroyBuffer(m_pDevice->GetDevice(), m_buffer, NULL);
#endif
            m_buffer = VK_NULL_HANDLE;
        }
    }

    bool StaticBufferAllocater::AllocBuffer(uint32_t numbeOfElements, uint32_t strideInBytes, VkDescriptorBufferInfo* pOut)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        //
        uint32_t size = AlignUp(numbeOfElements * strideInBytes, 256u);
        assert(m_memOffset + size < m_totalMemSize);
        //
        pOut->buffer = m_bUseVidMem ? m_bufferVid : m_buffer;
        pOut->offset = m_memOffset;
        pOut->range = size;

        m_memOffset += size;

        return true;
    }

    bool StaticBufferAllocater::AllocBuffer(
        uint32_t numbeOfElements, uint32_t strideInBytes
        , void** ppBuffer, VkDeviceAddress* pDeviceAddressBuffer
        , VkDescriptorBufferInfo* pOut)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        uint32_t size = AlignUp(numbeOfElements * strideInBytes, 256u);
        assert(m_memOffset + size < m_totalMemSize);

        //*pData = (void*)(m_pData + m_memOffset);
        if (!m_bOnlyGPU)
        {
            *ppBuffer = (void*)(m_pData + m_memOffset);
        }
        //else
        if (pDeviceAddressBuffer != NULL)
        {
            *pDeviceAddressBuffer = m_deviceAddressData + m_memOffset;
        }
        

        pOut->buffer = m_bUseVidMem ? m_bufferVid : m_buffer;
        pOut->offset = m_memOffset;
        pOut->range = size;

        m_memOffset += size;

        return true;
    }

    bool StaticBufferAllocater::AllocBuffer(
        uint32_t numbeOfElements, uint32_t strideInBytes
        , const void* pInitData
        , VkDeviceAddress* pDeviceAddressBuffer
        , VkDescriptorBufferInfo* pInitData_deviceBufferInfo
        , VkDescriptorBufferInfo* pOut)
    {
        void* pData;
        //if (AllocBuffer(numbeOfVertices, strideInBytes, &pData, pOut))
        if (AllocBuffer(numbeOfElements, strideInBytes, &pData, pDeviceAddressBuffer, pOut))
        {
            if (!m_bOnlyGPU)
            {
                memcpy(pData, pInitData, numbeOfElements * strideInBytes);
            }
            //
            else
            //if (pDeviceAddressBuffer != NULL)
            {
                //TODO: refer2 StaticBufferAllocater::UploadData(), May not Implement
                // CopyAccordingToDeviceAddress?
                // vkCmdCopyBuffer(src= *pInitData_DeviceBufferInfoInitData, dst=pDeviceAddressBuffer)
            }
            return true;
        }
        return false;
    }

    void StaticBufferAllocater::UploadData(VkCommandBuffer cmd_buf)
    {
        VkBufferCopy region;
        region.srcOffset = 0;
        region.dstOffset = 0;
        region.size = m_totalMemSize;

        vkCmdCopyBuffer(cmd_buf, m_buffer, m_bufferVid, 1, &region);
    }

    void StaticBufferAllocater::FreeUploadHeap()
    {
        if (m_bUseVidMem)
        {
            assert(m_buffer != VK_NULL_HANDLE);
#ifdef USE_VMA
            if (!m_bOnlyGPU)
            {
                vmaUnmapMemory(m_pDevice->GetAllocator(), m_bufferAlloc);
            }
            vmaDestroyBuffer(m_pDevice->GetAllocator(), m_buffer, m_bufferAlloc);
#else
            //release upload heap
            if (!m_bOnlyGPU)
            {
                vkUnmapMemory(m_pDevice->GetDevice(), m_deviceMemory);
            }
            vkFreeMemory(m_pDevice->GetDevice(), m_deviceMemory, NULL);
            m_deviceMemory = VK_NULL_HANDLE;
            vkDestroyBuffer(m_pDevice->GetDevice(), m_buffer, NULL);

#endif
            m_buffer = VK_NULL_HANDLE;
        }
    }
}
