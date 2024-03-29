
#pragma once


#include "base/Device.h"

namespace CAULDRON_VK
{
    // This class will create a Descriptor Pool and allows allocating, freeing and initializing Descriptor Set Layouts(DSL) from this pool. 

    class HeapDescriptorSetAllocater
    {
    public:
        //void OnCreate(Device* pDevice, uint32_t cbvDescriptorCount, uint32_t srvDescriptorCount, uint32_t uavDescriptorCount, uint32_t samplerDescriptorCount);
        void OnCreate(Device* pDevice, uint32_t numMaxDescriptorSet, std::vector<VkDescriptorPoolSize>& array_descriptorTypePoolSize);
        void OnDestroy();
        bool AllocDescriptor(VkDescriptorSetLayout descriptorLayout, VkDescriptorSet* pDescriptor);
        bool AllocDescriptor(int size, const VkSampler* pSamplers, VkDescriptorSetLayout* descriptorLayout, VkDescriptorSet* pDescriptor);
        bool AllocDescriptor(std::vector<uint32_t>& descriptorCounts, const VkSampler* pSamplers, VkDescriptorSetLayout* descriptorLayout, VkDescriptorSet* pDescriptor);
        bool CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding>* pDescriptorLayoutBinding, VkDescriptorSetLayout* pDescSetLayout);
        bool CreateDescriptorSetLayoutAndAllocDescriptorSet(std::vector<VkDescriptorSetLayoutBinding>* pDescriptorLayoutBinding, VkDescriptorSetLayout* descriptorLayout, VkDescriptorSet* pDescriptor);
        void FreeDescriptor(VkDescriptorSet descriptorSet);
    private:
        Device* m_pDevice;
        VkDescriptorPool m_descriptorPool;
        std::mutex       m_mutex = {};
        int              m_allocatedDescriptorSetCount = 0;
    };
}

