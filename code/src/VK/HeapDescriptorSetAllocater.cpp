
#include "stdafx.h"
#include "Misc/misc.h"
#include "HeapDescriptorSetAllocater.h"

namespace CAULDRON_VK
{

    //void ResourceViewHeaps::OnCreate(Device* pDevice, uint32_t cbvDescriptorCount, uint32_t srvDescriptorCount, uint32_t uavDescriptorCount, uint32_t samplerDescriptorCount)
    void HeapDescriptorSetAllocater::OnCreate(Device* pDevice, uint32_t numMaxDescriptorSet, std::vector<VkDescriptorPoolSize>& array_descriptorTypePoolSize)
    {
        m_pDevice = pDevice;
        m_allocatedDescriptorSetCount = 0;
        
        /*
        const VkDescriptorPoolSize type_count[] =
        {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, cbvDescriptorCount },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, cbvDescriptorCount },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, srvDescriptorCount },
            { VK_DESCRIPTOR_TYPE_SAMPLER, samplerDescriptorCount },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, uavDescriptorCount }
        };
        */

        VkDescriptorPoolCreateInfo descriptor_pool = {};
        descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool.pNext = NULL;
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorPoolCreateFlagBits.html
        descriptor_pool.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        descriptor_pool.maxSets = numMaxDescriptorSet;
        //descriptor_pool.poolSizeCount = _countof(type_count);
        descriptor_pool.poolSizeCount = (uint32_t)array_descriptorTypePoolSize.size();
        //descriptor_pool.pPoolSizes = type_count;
        descriptor_pool.pPoolSizes = array_descriptorTypePoolSize.data();

        VkResult res = vkCreateDescriptorPool(pDevice->GetDevice(), &descriptor_pool, NULL, &m_descriptorPool);
        assert(res == VK_SUCCESS);
    }

    void HeapDescriptorSetAllocater::OnDestroy()
    {
        vkDestroyDescriptorPool(m_pDevice->GetDevice(), m_descriptorPool, NULL);
    }

    bool HeapDescriptorSetAllocater::CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding>* pDescriptorLayoutBinding, VkDescriptorSetLayout* pDescSetLayout)
    {
        // Next take layout bindings and use them to create a descriptor set layout

        VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
        descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_layout.pNext = NULL;
        descriptor_layout.bindingCount = (uint32_t)pDescriptorLayoutBinding->size();
        descriptor_layout.pBindings = pDescriptorLayoutBinding->data();

        VkResult res = vkCreateDescriptorSetLayout(m_pDevice->GetDevice(), &descriptor_layout, NULL, pDescSetLayout);
        assert(res == VK_SUCCESS);
        return (res == VK_SUCCESS);
    }
    bool HeapDescriptorSetAllocater::CreateDescriptorSetLayoutAndAllocDescriptorSet(std::vector<VkDescriptorSetLayoutBinding>* pDescriptorLayoutBinding, VkDescriptorSetLayout* pDescSetLayout, VkDescriptorSet* pDescriptorSet)
    {
        // Next take layout bindings and use them to create a descriptor set layout

        VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
        descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_layout.pNext = NULL;
        descriptor_layout.bindingCount = (uint32_t)pDescriptorLayoutBinding->size();
        descriptor_layout.pBindings = pDescriptorLayoutBinding->data();

        VkResult res = vkCreateDescriptorSetLayout(m_pDevice->GetDevice(), &descriptor_layout, NULL, pDescSetLayout);
        assert(res == VK_SUCCESS);

        return AllocDescriptor(*pDescSetLayout, pDescriptorSet);
    }

    bool HeapDescriptorSetAllocater::AllocDescriptor(VkDescriptorSetLayout descLayout, VkDescriptorSet* pDescriptorSet)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        VkDescriptorSetAllocateInfo alloc_info;
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.pNext = NULL;
        alloc_info.descriptorPool = m_descriptorPool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &descLayout;

        VkResult res = vkAllocateDescriptorSets(m_pDevice->GetDevice(), &alloc_info, pDescriptorSet);
        assert(res == VK_SUCCESS);

        m_allocatedDescriptorSetCount++;

        return res == VK_SUCCESS;
    }

    void HeapDescriptorSetAllocater::FreeDescriptor(VkDescriptorSet descriptorSet)
    {
        m_allocatedDescriptorSetCount--;
        vkFreeDescriptorSets(m_pDevice->GetDevice(), m_descriptorPool, 1, &descriptorSet);
    }

    bool HeapDescriptorSetAllocater::AllocDescriptor(int size, const VkSampler* pSamplers, VkDescriptorSetLayout* pDescSetLayout, VkDescriptorSet* pDescriptorSet)
    {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings(size);
        for (int i = 0; i < size; i++)
        {
            layoutBindings[i].binding = i;
            layoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBindings[i].descriptorCount = 1;
            layoutBindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            layoutBindings[i].pImmutableSamplers = (pSamplers != NULL) ? &pSamplers[i] : NULL;
        }

        return CreateDescriptorSetLayoutAndAllocDescriptorSet(&layoutBindings, pDescSetLayout, pDescriptorSet);
    }

    bool HeapDescriptorSetAllocater::AllocDescriptor(std::vector<uint32_t>& descriptorCounts, const VkSampler* pSamplers, VkDescriptorSetLayout* pDescSetLayout, VkDescriptorSet* pDescriptorSet)
    {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings(descriptorCounts.size());
        for (int i = 0; i < descriptorCounts.size(); i++)
        {
            layoutBindings[i].binding = i;
            layoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBindings[i].descriptorCount = descriptorCounts[i];
            layoutBindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            layoutBindings[i].pImmutableSamplers = (pSamplers != NULL) ? &pSamplers[i] : NULL;
        }

        return CreateDescriptorSetLayoutAndAllocDescriptorSet(&layoutBindings, pDescSetLayout, pDescriptorSet);
    }

}
