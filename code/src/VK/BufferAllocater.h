#pragma once

#include "../VulkanMemoryAllocator/vk_mem_alloc.h"

namespace CAULDRON_VK
{
    //
    union BufferMemoryAllocationUsage
    {
        //https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/group__group__alloc.html#gaa5846affa1e9da3800e3e78fae2305cc
        // <=> VmaMemoryUsage
        VkFlags                 vmaAllocation_usage;
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkMemoryPropertyFlagBits.html
        // <=> VkMemoryPropertyFlagBits
        //typedef VkFlags|uint32_t VkMemoryPropertyFlags
        VkMemoryPropertyFlags   vkMemoryAllocation_usage;
    };

    union BufferMemoryAllocationInfo
    {
        //https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocation_create_info.html
        VmaAllocationCreateInfo     vmaAllocationInfo;
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkMemoryAllocateInfo.html
        VkMemoryAllocateInfo        vkMemoryAllocateInfo;
    };

    union PointerBufferMemoryAllocationInfo
    {
        //https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocation_create_info.html
        VmaAllocationCreateInfo*  pVmaAllocationInfo;
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkMemoryAllocateInfo.html
        VkMemoryAllocateInfo*     pVkMemoryAllocateInfo;
    };

    union PointerConstBufferMemoryAllocationInfo
    {
        //https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocation_create_info.html
        const VmaAllocationCreateInfo*  pVmaAllocationInfo;
        //https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkMemoryAllocateInfo.html
        const VkMemoryAllocateInfo*     pVkMemoryAllocateInfo;
    };
    //
    //
}
