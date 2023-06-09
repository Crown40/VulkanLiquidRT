
#include "PhysicsRT.h"


namespace CAULDRON_VK
{
	//void PhysicsRT::OnCreate(Device* pDevice, VkPipeline computePipeline, ResourceViewHeaps* pResourceViewHeaps, StaticBufferPool* pStaticBufferPool, DynamicBufferRing* pDynamicBufferRing)
	void PhysicsRT::OnCreate(Device* pDevice, uint32_t numParticle
		, HeapDescriptorSetAllocater* pDescriptorSetHeap
		, DynamicBufferAllocater* pConstBufferDynamic_CPU2GPU
		, DynamicBufferAllocater* pRtBufferDynamic_GPU, StaticBufferAllocater* pRtBufferStatic_GPU)
	{
		m_pDevice = pDevice;
		//assert(computePipeline);
		//m_computePipeline = computePipeline;
		m_pDescriptorSetHeap = pDescriptorSetHeap;
		//m_pStaticBufferPool = pStaticBufferPool;
		//m_pDynamicBufferRing = pDynamicBufferRing;
		//
		m_pConstBufferDynamic_CPU2GPU = pConstBufferDynamic_CPU2GPU;
		m_pRtBufferDynamic_GPU = pRtBufferDynamic_GPU;
		m_pRtBufferStatic_GPU = pRtBufferStatic_GPU;
		//



		VkResult res;
		//


		// Extensions and its Features and Properties
		// Have been Implemented by ExtRTCheckExtensions() be Called in Device::OnCreate() through Device::SetEssentialDeviceExtensions()
		/*
		// Get Properties and Features
		m_physicalDeviceRTpipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		// VkPhysicalDeviceProperties For Vulkan-basic
		// VkPhysicalDeviceProperties2 For Vulkan-extension
		VkPhysicalDeviceProperties2 physicalDevice_properties2 = {};
		physicalDevice_properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		physicalDevice_properties2.pNext = &m_physicalDeviceRTpipelineProperties;
		vkGetPhysicalDeviceProperties2(m_pDevice->GetPhysicalDevice(), &physicalDevice_properties2);
		// 
		m_physicalDeviceAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		// VkPhysicalDeviceFeatures For Vulkan-basic
		// VkPhysicalDeviceFeatures2 For Vulkan-extension
		VkPhysicalDeviceFeatures2 physicalDevice_features2 = {};
		physicalDevice_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		physicalDevice_features2.pNext = &m_physicalDeviceAccelerationStructureFeatures;
		// 
		*/
		//
		
		//
		// Get the function pointers required for ray tracing
		//Altered
		vkGetBufferDeviceAddressKHR					= reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(m_pDevice->GetDevice(), "vkGetBufferDeviceAddressKHR"));
		vkCmdBuildAccelerationStructuresKHR			= reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(m_pDevice->GetDevice(), "vkCmdBuildAccelerationStructuresKHR"));
		vkBuildAccelerationStructuresKHR			= reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(m_pDevice->GetDevice(), "vkBuildAccelerationStructuresKHR"));
		vkCreateAccelerationStructureKHR			= reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(m_pDevice->GetDevice(), "vkCreateAccelerationStructureKHR"));
		vkDestroyAccelerationStructureKHR			= reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(m_pDevice->GetDevice(), "vkDestroyAccelerationStructureKHR"));
		vkGetAccelerationStructureBuildSizesKHR		= reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(m_pDevice->GetDevice(), "vkGetAccelerationStructureBuildSizesKHR"));
		vkGetAccelerationStructureDeviceAddressKHR	= reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(m_pDevice->GetDevice(), "vkGetAccelerationStructureDeviceAddressKHR"));
		vkCmdTraceRaysKHR							= reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(m_pDevice->GetDevice(), "vkCmdTraceRaysKHR"));
		vkGetRayTracingShaderGroupHandlesKHR		= reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(m_pDevice->GetDevice(), "vkGetRayTracingShaderGroupHandlesKHR"));
		vkCreateRayTracingPipelinesKHR				= reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(m_pDevice->GetDevice(), "vkCreateRayTracingPipelinesKHR"));
		//
		/*
		PFN_vkGetBufferDeviceAddressKHR					vkGetBufferDeviceAddressKHR;
		vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(m_pDevice->GetDevice(), "vkGetBufferDeviceAddressKHR"));
		VkBufferDeviceAddressInfo buffer_deviceAddressInfo{};
		//buffer_deviceAddressInfo.
		//vkGetBufferDeviceAddressKHR()
		buffer_deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		buffer_deviceAddressInfo.pNext = NULL;
		buffer_deviceAddressInfo.buffer = m_bufferVid;
		//
		uint64_t device_address = vkGetBufferDeviceAddressKHR(pDevice->GetDevice(), &buffer_deviceAddressInfo);
		device_address;
		*/
		//

		//
		m_numParticle = numParticle;
		//m_particleAabbBuffer
		OnCreateAabbBuffer(m_numParticle, m_particleAabbBuffer);
		//

		//
		//m_particleInstanceBuffer
		OnCreateInstanceBuffer(1, m_particleInstanceBuffer, 0, 0);
		//

		//m_particleBottomLevel_AsKHR,m_particleTopLevel_AsKHR
		m_bDynamicBlasSize = false;
		m_bDynamicTlasSize = false;
		m_bUpdateBlas = false;
		m_bUpdateTlas = false;
		//


		//
		//Tlas
		//Allocate-DescriptorSet in **RT**
		//	Create DescriptorSetLayout
		VkDescriptorSetLayoutBinding	descriptorSetLayoutBinding;
		{
			//
			descriptorSetLayoutBinding.binding = 0;
			//https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorType.html
			//When a descriptor set is updated via elements of VkWriteDescriptorSet
			//	, members of pImageInfo, pBufferInfo and pTexelBufferView are only accessed by the implementation when they correspond to descriptor type being defined 
			//	- otherwise they are ignored. The members accessed are as follows for each descriptor type:
			//For VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, **VK_DESCRIPTOR_TYPE_STORAGE_BUFFER**, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, or VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
			//	, all members of each element of VkWriteDescriptorSet::pBufferInfo are accessed
			descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
			descriptorSetLayoutBinding.descriptorCount = 1;
			// | VK_SHADER_STAGE_CALLABLE_BIT_KHR
			descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR;
			//
			VkDescriptorSetLayout			descriptorSetLayout;
			m_pDescriptorSetHeap->CreateDescriptorSetLayout(&std::vector<VkDescriptorSetLayoutBinding>{ descriptorSetLayoutBinding }
			, & descriptorSetLayout);
			//particle_instanceBuffer.SetDescriptorSetLayout(descriptorSetLayout);
			m_particleTopLevel_AsKHR.SetDescriptorSetLayout(descriptorSetLayout);
			//	Create DescriptorSet
			VkDescriptorSet					descriptorSet;
			m_pDescriptorSetHeap->AllocDescriptor(descriptorSetLayout, &descriptorSet);
			//particle_instanceBuffer.SetDescriptorSet(descriptorSet);
			m_particleTopLevel_AsKHR.SetDescriptorSet(descriptorSet);
			//
			// would be vkCmdBindDescriptorSets(VkDescriptorSet*) after OnCreateAabbBuffer()
			//vkCmdBindDescriptorSets(VkDescriptorSet*)
			//vkCmdBindPipeline(VkPipeline computePipeline)
			//
		}
		//

		
		//SetResourceName

		//
	}

	void PhysicsRT::OnDestroy()
	{
		
		VkDescriptorSet		descriptorSet = VK_NULL_HANDLE;
		//
		m_particleAabbBuffer.OnDestroy(m_pDevice->GetDevice(), &descriptorSet);
		if (descriptorSet != VK_NULL_HANDLE)
		{
			m_pDescriptorSetHeap->FreeDescriptor(descriptorSet);
			descriptorSet = VK_NULL_HANDLE;
		}
		//
		m_particleInstanceBuffer.OnDestroy(m_pDevice->GetDevice(), &descriptorSet);
		if (descriptorSet != VK_NULL_HANDLE)
		{
			m_pDescriptorSetHeap->FreeDescriptor(descriptorSet);
			descriptorSet = VK_NULL_HANDLE;
		}
		//
		m_particleBottomLevel_AsKHR.OnDestroy(m_pDevice->GetDevice(), &descriptorSet);
		if (descriptorSet != VK_NULL_HANDLE)
		{
			m_pDescriptorSetHeap->FreeDescriptor(descriptorSet);
			descriptorSet = VK_NULL_HANDLE;
		}
		//
		m_particleTopLevel_AsKHR.OnDestroy(m_pDevice->GetDevice(), &descriptorSet);
		if (descriptorSet != VK_NULL_HANDLE)
		{
			m_pDescriptorSetHeap->FreeDescriptor(descriptorSet);
			descriptorSet = VK_NULL_HANDLE;
		}
		//
	}


	void PhysicsRT::OnCreateAabbBuffer(uint32_t numParticle, ParticleAabbBuffer& particleAabbBuffer)
	{

		VkResult res;
		//
		
		//Allocate-StaticBuffer
		VkDeviceAddress				deviceAddress;
		VkDescriptorBufferInfo		descriptorBufferInfo;
		{
			//
			// 24 => 32
			//m_pRtBufferStatic_GPU->AllocBuffer(numParticle, sizeof(VkAabbPositionsKHR)
			m_pRtBufferStatic_GPU->AllocBuffer(numParticle, sizeof(ParticleAabbPositionAlign)
				, NULL, &deviceAddress, &descriptorBufferInfo);
			assert(deviceAddress && descriptorBufferInfo.buffer);
			//
			//particleAabbBuffer.SetParticleNum(numParticle);
			//Below include particleAabbBuffer.SetPrimitiveCount(numParticle);
			particleAabbBuffer.OnInit(numParticle);
			particleAabbBuffer.SetDescriptorBufferInfo(descriptorBufferInfo);
			//particleAabbBuffer.SetBufferHostAddress()
			particleAabbBuffer.SetBufferDeviceAddress(deviceAddress);
			//Below
			//particleAabbBuffer.SetDescriptorSet()
			//particleAabbBuffer.SetDescriptorSetLayout()
			//
		}


		//
		//Allocate-DescriptorSet in **Compute-Pipeline**
		//	Create DescriptorSetLayout
		VkDescriptorSetLayoutBinding	descriptorSetLayoutBinding;
		{
			//
			descriptorSetLayoutBinding.binding = 0;
			//https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorType.html
			//When a descriptor set is updated via elements of VkWriteDescriptorSet
			//	, members of pImageInfo, pBufferInfo and pTexelBufferView are only accessed by the implementation when they correspond to descriptor type being defined 
			//	- otherwise they are ignored. The members accessed are as follows for each descriptor type:
			//For VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, **VK_DESCRIPTOR_TYPE_STORAGE_BUFFER**, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, or VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
			//	, all members of each element of VkWriteDescriptorSet::pBufferInfo are accessed
			descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorSetLayoutBinding.descriptorCount = 1;
			//descriptorSetLayoutBinding.pImmutableSamplers=
			descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			//
			VkDescriptorSetLayout			descriptorSetLayout;
			m_pDescriptorSetHeap->CreateDescriptorSetLayout(&std::vector<VkDescriptorSetLayoutBinding>{ descriptorSetLayoutBinding }
				, &descriptorSetLayout);
			particleAabbBuffer.SetDescriptorSetLayout(descriptorSetLayout);
			//	Create DescriptorSet
			VkDescriptorSet					descriptorSet;
			m_pDescriptorSetHeap->AllocDescriptor(descriptorSetLayout, &descriptorSet);
			particleAabbBuffer.SetDescriptorSet(descriptorSet);
			//
			// would be vkCmdBindDescriptorSets(VkDescriptorSet*) after OnCreateAabbBuffer()
			//vkCmdBindDescriptorSets(VkDescriptorSet*)
			//vkCmdBindPipeline(VkPipeline computePipeline)
			//
		}
		//

		//
		//UpdateFormat-DescriptorSet: **Bind Buffer 2 DescriptorSet**
		//
		VkDescriptorBufferInfo write_descriptorBufferInfo = particleAabbBuffer.GetDescriptorBufferInfo();
		//For Descriptors in DescriptorSet
		VkWriteDescriptorSet writeDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		{
			//writeDescriptorSet.pNext = NULL;
			writeDescriptorSet.dstSet = particleAabbBuffer.GetDescriptorSet();
			writeDescriptorSet.dstBinding = 0;
			//Starting element in that array: 
			// If the descriptor binding identified by dstSet and dstBinding has a descriptor type of VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK 
			//	then dstArrayElement specifies the Starting-byte-Offset within the binding.
			//	1: if 0-start-offset for Special-process|usage
			writeDescriptorSet.dstArrayElement = 0;
			//
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			writeDescriptorSet.pBufferInfo = &write_descriptorBufferInfo;

			vkUpdateDescriptorSets(m_pDevice->GetDevice(), 1, &writeDescriptorSet, 0, NULL);
			//
		}
		//
		//TODO
		//SetResourceName(m_pDevice->GetDevice(),VK_OBJECT_TYPE_BUFFER,)
	}

	void PhysicsRT::OnCreateInstanceBuffer(uint32_t numInstance, ParticleInstanceBuffer& particleInstanceBuffer
		, uint32_t instanceCustomIndex, uint32_t instanceShaderBindingTableRecordOffset
	)
	{

		VkResult res;
		//

		//Allocate-StaticBuffer
		VkDeviceAddress				deviceAddress;
		VkDescriptorBufferInfo		descriptorBufferInfo;
		m_pRtBufferStatic_GPU->AllocBuffer(numInstance, sizeof(VkAccelerationStructureInstanceKHR)
			, NULL, &deviceAddress, &descriptorBufferInfo);
		assert(deviceAddress && descriptorBufferInfo.buffer);
		//
		particleInstanceBuffer.SetPrimitiveCount(numInstance);
		particleInstanceBuffer.SetInstanceCustomIndex(instanceCustomIndex);
		particleInstanceBuffer.SetInstanceShaderBindingTableRecordOffset(instanceShaderBindingTableRecordOffset);
		//
		particleInstanceBuffer.SetDescriptorBufferInfo(descriptorBufferInfo);
		//particleAabbBuffer.SetBufferHostAddress()
		particleInstanceBuffer.SetBufferDeviceAddress(deviceAddress);
		//
		
		//No need to Allocate/Upadte|Write DescriptorSet for InstanceBuffer
		//TODO
		//SetResourceName(m_pDevice->GetDevice(),VK_OBJECT_TYPE_BUFFER,)
	}



	void PhysicsRT::BuildFullLevelAccelerationStructure(VkCommandBuffer cmdBuffer)
	{
		//
		/*
		uint32_t instanceCustomIndex, instanceShaderBindingTableRecordOffset = 0;
		if (!bUpdateAs)
		{
			m_particleInstanceBuffer.SetInstanceCustomIndex(m_NumCreatedInstanceCustomIndex++);
			m_particleInstanceBuffer.SetInstanceShaderBindingTableRecordOffset(m_NumCreatedInstanceShaderBindingTableRecordOffset++);
		}
		*/
		//
		BuildBottomLevelAccelerationStructure(cmdBuffer, m_particleBottomLevel_AsKHR
			, m_particleAabbBuffer, m_bUpdateBlas, m_bDynamicBlasSize);
		m_bUpdateBlas = true;
		//
		BuildTopLevelAccelerationStructure(cmdBuffer, m_particleTopLevel_AsKHR, m_particleBottomLevel_AsKHR
			, m_particleInstanceBuffer, m_bUpdateTlas, m_bDynamicTlasSize);
		m_bUpdateTlas = true;
		//


		//SetResourceName(m_pDevice->GetDevice(),VK_OBJECT_TYPE_BUFFER,)
	}

	//Create BLAS
	void PhysicsRT::BuildBottomLevelAccelerationStructure(
		VkCommandBuffer cmdBuffer
		, ParticleAccelerationStructure& particle_blas
		, ParticleAabbBuffer& particle_AabbBuffer
		, bool bUpdateAs
		, bool bDynamicAsSize
	)
	{
		//
		//VkDevice device = m_pDevice->GetDevice();
		VkResult res;
		//

		// Check[Get] GeomBuffer -Device|Host Address
		// union: deviceAddress|DeviceAddress{VkDeviceAddress|uint64_t} / hostAddress|HostAddressConst{const void*}
		if (particle_AabbBuffer.GetBufferAddress().deviceAddress == NULL 
			|| particle_AabbBuffer.GetDescriptorBufferInfo().buffer == VK_NULL_HANDLE)
		{
			// 
			VkDescriptorBufferInfo	descriptorBufferInfo;
			if (particle_AabbBuffer.GetDescriptorBufferInfo().buffer == VK_NULL_HANDLE)
			{
				//Be included: AllocateBuffer()?
				//Be included: particle_AabbBuffer.SetBufferDeviceAddress()
				//Finished
				OnCreateAabbBuffer(m_numParticle, particle_AabbBuffer);
				//
				//particle_AabbBuffer.SetDescriptorBufferInfo(descriptorBufferInfo);
			}
			else
			{
				//assert(particle_AabbBuffer.GetDescriptorBufferInfo().buffer)
				//vkGetBufferDeviceAddress,vkGetBufferDeviceAddressKHR	,vkGetBufferDeviceAddressEXT
				//VkBufferDeviceAddressInfo,VkBufferDeviceAddressInfoEXT,VkBufferDeviceAddressInfoKHR
				descriptorBufferInfo = particle_AabbBuffer.GetDescriptorBufferInfo();
				VkBufferDeviceAddressInfo buffer_deviceAddressInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
				buffer_deviceAddressInfo.buffer = descriptorBufferInfo.buffer;
				// A value of Zero is reserved as a “Null” pointer and must not be returned as a valid buffer device address.
				//	vkGetBufferDeviceAddressKHR(),vkGetBufferDeviceAddressEXT()
				VkDeviceAddress		deviceAddress = vkGetBufferDeviceAddressKHR(m_pDevice->GetDevice(), &buffer_deviceAddressInfo);
				assert(deviceAddress);
				particle_AabbBuffer.SetBufferDeviceAddress(deviceAddress + descriptorBufferInfo.offset);
			}
			// 
			//
		}
		//



		//Geom:AABB
		//<Duplicate>
		// Set geom info:	bottomLevel-AccelerationStructure-Grometry{KHR}
		//	Set ASGeomInfo: VkAccelerationStructureGeometryKHR
		VkAccelerationStructureGeometryKHR	as_geomKHR{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		{
			//
			//as_geomKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			//as_geomKHR.pNext = NULL;
			// Instead of providing actual geometry (e.g. triangles), we only provide the axis aligned bounding boxes (AABBs) of the spheres
			// The data for the actual spheres is passed elsewhere as a shader storage buffer object
			//
			// VK_GEOMETRY_OPAQUE_BIT_KHR,	VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR	,	VK_GEOMETRY_OPAQUE_BIT_NV = VK_GEOMETRY_OPAQUE_BIT_KHR, VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_NV = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR,
			// VK_GEOMETRY_OPAQUE_BIT_KHR指示此几何体不会调用任何命中着色器，即使存在于命中组中也是如此。
			// VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR指示实现必须仅为此几何体中的每个图元调用一次任意命中着色器。如果此位不存在，实现可能会为此几何 多次调用任意命中着色器
			as_geomKHR.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
			//as_geomKHR.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
			//
			// VK_GEOMETRY_TYPE_INSTANCES_KHR,VK_GEOMETRY_TYPE_AABBS_KHR,VK_GEOMETRY_TYPE_TRIANGLES_KHR	,VK_GEOMETRY_TYPE_AABBS_NV = VK_GEOMETRY_TYPE_AABBS_KHR,VK_GEOMETRY_TYPE_TRIANGLES_NV = VK_GEOMETRY_TYPE_TRIANGLES_KHR
			as_geomKHR.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
			as_geomKHR.geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
			//as_geomKHR.geometry.aabbs.pNext = NULL;
			// GeomBufferAddress{VkDeviceOrHostAddressConstKHR}
			as_geomKHR.geometry.aabbs.data = particle_AabbBuffer.GetBufferAddress();
			//as_geomKHR.geometry.aabbs.data.deviceAddress = particle_AabbBuffer.GetBufferAddress().deviceAddress;
			// stride是 中每个条目之间的步幅（以字节为单位）data。步幅必须是 的倍数8
			// sizeof(VkAabbPositionsKHR)|24 => 32
			as_geomKHR.geometry.aabbs.stride = sizeof(ParticleAabbPositionAlign);
			//
		}
		//

		/*
		CreateAccelerationStructure(cmdBuffer, { as_geomKHR }, { 1 }
				, particle_tlas, &particle_instanceBuffer, false, false);
		*/
		bool bBLAS = true;
		BuildAccelerationStructure(cmdBuffer
			, std::vector<VkAccelerationStructureGeometryKHR>{ as_geomKHR }, std::vector<uint32_t>{ particle_AabbBuffer.GetPrimitiveCount() }
			, particle_blas, &particle_AabbBuffer, bUpdateAs, bBLAS, bDynamicAsSize);
		//



		//

	}

	void PhysicsRT::BuildTopLevelAccelerationStructure(
		VkCommandBuffer cmdBuffer
		, ParticleAccelerationStructure& particle_tlas
		, ParticleAccelerationStructure& particle_blas
		, ParticleInstanceBuffer& particle_instanceBuffer
		//, uint32_t instanceCustomIndex
		//, uint32_t instanceShaderBindingTableRecordOffset
		, bool bUpdateAs
		, bool bDynamicAsSize
	)
	{
		//
		//VkDevice device = m_pDevice->GetDevice();
		VkResult res;
		//

		// Check	BLAS -Device|Host Address
		assert(particle_blas.GetAccelerationStructureDeviceAddress() && particle_blas.GetAccelerationStructureKHR());
		
		//Instance
		// Check[Set] Instacne info:		topLevel-AccelerationStructureInstance{KHR}
		VkAccelerationStructureInstanceKHR		as_instanceKHR = particle_instanceBuffer.GetAccelerationStructureInstanceKHR();
		if (as_instanceKHR.accelerationStructureReference == NULL 
			|| as_instanceKHR.accelerationStructureReference != particle_blas.GetBufferAddress().deviceAddress);
		{
			//
			VkTransformMatrixKHR transformMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f
			};
			//
			as_instanceKHR.transform = transformMatrix;
			//instanceCustomIndex:uint32_t: 24-bit user-specified index value Accessible to ray shaders in the InstanceCustomIndexKHR built-in.
			//Deprecate: as_instanceKHR.instanceCustomIndex = m_NumCreatedInstanceCustomIndex++;
			//as_instanceKHR.instanceCustomIndex = instanceCustomIndex;
			as_instanceKHR.instanceCustomIndex = particle_instanceBuffer.GetInstanceCustomIndex();
			//mask:uint32_t:	8-bit visibility mask for the geometry. The instance may only be hit **if Cull Mask & instance.mask != 0**
			as_instanceKHR.mask = 0xFF;
			//
			//instanceShaderBindingTableRecordOffset
			//Deprecate: if and only if	Create instanceShaderBindingTableRecordOffset
			//as_instanceKHR.instanceShaderBindingTableRecordOffset = m_NumCreatedInstanceShaderBindingTableRecordOffset++;
			//Deprecate:
			//as_instanceKHR.instanceShaderBindingTableRecordOffset = 0;
			//as_instanceKHR.instanceShaderBindingTableRecordOffset = instanceShaderBindingTableRecordOffset;
			as_instanceKHR.instanceShaderBindingTableRecordOffset = particle_instanceBuffer.GetInstanceShaderBindingTableRecordOffset();
			//
			//flags:VkGeometryInstanceFlagBitsKHR: https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkGeometryInstanceFlagBitsKHR.html
			//  VK_GEOMETRY_INSTANCE_FORCE_OPACITY_MICROMAP_2_STATE_EXT,VK_GEOMETRY_INSTANCE_DISABLE_OPACITY_MICROMAPS_EXT
			//	,VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR{DisableFaceCull},VK_GEOMETRY_INSTANCE_TRIANGLE_FLIP_FACING_BIT_KHR{Indicates that the facing-determination for geometry in this-instance is inverted.}
			//	,VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR{ForceOpaquee: act as though **VK_GEOMETRY_OPAQUE_BIT_KHR were specified** on all Geometries Referenced by this instance}
			//	,VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR{ForeNoOpaquee: act as though **VK_GEOMETRY_OPAQUE_BIT_KHR were not specified** on all Geometries Referenced by this instance}
			as_instanceKHR.flags = VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR;
			//as_instanceKHR.flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR;
			//
			//accelerationStructureReference:uint64_t: either
			//	a Device-address containing the value obtained from vkGetAccelerationStructureDeviceAddressKHR or vkGetAccelerationStructureHandleNV 
			// (used by **Device operations** which reference acceleration structures)
			//	a VkAccelerationStructureKHR object (used by **Host operations** which reference acceleration structures).
			//
			//as_instanceKHR.accelerationStructureReference = particle_blas.GetBufferAddress().deviceAddress;
			as_instanceKHR.accelerationStructureReference = particle_blas.GetAccelerationStructureDeviceAddress();
			//
			particle_instanceBuffer.SetAccelerationStructureInstanceKHR(as_instanceKHR);
			//Finished: Use ConstBufferDynamic to CopyBuffer
			void*					temp_pBuffer;
			VkDescriptorBufferInfo	temp_bufferInfo;
			m_pConstBufferDynamic_CPU2GPU->AllocBuffer(sizeof(VkAccelerationStructureInstanceKHR)
				, &temp_pBuffer, NULL, &temp_bufferInfo);
			memcpy(temp_pBuffer, &as_instanceKHR, sizeof(VkAccelerationStructureInstanceKHR));
			//	Copy
			VkDescriptorBufferInfo	target_bufferInfo = particle_instanceBuffer.GetDescriptorBufferInfo();
			VkBufferCopy	temp_bufferCopy{ temp_bufferInfo.offset,target_bufferInfo.offset,sizeof(VkAccelerationStructureInstanceKHR) };
			vkCmdCopyBuffer(cmdBuffer, temp_bufferInfo.buffer, target_bufferInfo.buffer
				, 1, &temp_bufferCopy);
			//?TODO?: ?Pipeline-BufferBarrier?

			//VkBufferMemoryBarrier2
			VkBufferMemoryBarrier	bufferMemory_barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
			{
				VkDescriptorBufferInfo descriptorBufferInfo = target_bufferInfo;

				// | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR 
				bufferMemory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				// | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR
				bufferMemory_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
				bufferMemory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferMemory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferMemory_barrier.buffer = descriptorBufferInfo.buffer;
				bufferMemory_barrier.offset = descriptorBufferInfo.offset;
				bufferMemory_barrier.size = descriptorBufferInfo.range;

				//vkCmdPipelineBarrier2()
				vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
					, 0, 0, NULL, 1, &bufferMemory_barrier, 0, NULL);
				//
			}

			//
		}
		/*
		//Deprecate
		else
		{
			as_instanceKHR.accelerationStructureReference = particle_blas.GetBufferAddress().deviceAddress;
		}
		*/
		//

		//InstanceBuffer
		// Check[Get] InstanceBuffer -Device|Host Address
		// Allocate InstanceBuffer:		topLevel-InstanceBuffer
		{
			//
			if (particle_instanceBuffer.GetBufferAddress().deviceAddress == NULL 
				|| particle_instanceBuffer.GetDescriptorBufferInfo().buffer == VK_NULL_HANDLE)
			{
				
				VkDescriptorBufferInfo	descriptorBufferInfo;
				if (particle_instanceBuffer.GetDescriptorBufferInfo().buffer == VK_NULL_HANDLE)
				{
					//Should be Static+UpLoad
					// Offet-Allocate Buffer{VkDescriptorBufferInfo} for Instance{VkAccelerationStructureInstanceKHR}
					//Be included below
					// Allocate(sizeof(VkAccelerationStructureInstanceKHR), descriptorBufferInfo)
					// particle_instanceBuffer.SetDescriptorBufferInfo(descriptorBufferInfo);
					// particle_instanceBuffer.SetBufferDeviceAddress()
					//m_NumCreatedInstanceCustomIndex,m_NumCreatedInstanceShaderBindingTableRecordOffset
					//
					OnCreateInstanceBuffer(1, particle_instanceBuffer, 0, 0);
					//
				}
				else
				{
					descriptorBufferInfo = particle_instanceBuffer.GetDescriptorBufferInfo();
					VkBufferDeviceAddressInfo	buffer_deviceAddressInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
					buffer_deviceAddressInfo.buffer = descriptorBufferInfo.buffer;
					VkDeviceAddress		deviceAddress = vkGetBufferDeviceAddressKHR(m_pDevice->GetDevice(), &buffer_deviceAddressInfo);
					assert(deviceAddress);
					particle_instanceBuffer.SetBufferDeviceAddress(deviceAddress + descriptorBufferInfo.offset);
				}
				//
				
				//
			}
			/*
			//Deprecate
			else
			{
				//
				//TODO
				//particle_instanceBuffer
				//UploadData()
				//FlushAndFinish()
				//
			}
			*/
			//Deprecate
			//particle_instanceBuffer.SetAccelerationStructureInstanceKHR(as_instanceKHR);
			//
		}
		//
		
		//Geom: Instance
		//Refer 2 Duplicate
		//topLevel-AccelerationStructure-ScratchBuffer-BuildRangeInfo{KHR}
		// Set geom info:		topLevel-AccelerationStructure-GeometryInfo{KHR}
		VkAccelerationStructureGeometryKHR		as_geomKHR{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		{
			//
			//as_geomKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			//as_geomKHR.pNext = NULL;
			// VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR, VK_GEOMETRY_OPAQUE_BIT_KHR
			as_geomKHR.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
			//as_geomKHR.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
			// VK_GEOMETRY_TYPE_INSTANCES_KHR
			as_geomKHR.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
			//
			as_geomKHR.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
			//as_geomKHR.geometry.instances.pNext = NULL;
			as_geomKHR.geometry.instances.arrayOfPointers = VK_FALSE;
			//
			as_geomKHR.geometry.instances.data = particle_instanceBuffer.GetBufferAddress();
			//
		}
		//

		//
		bool bBLAS = false;
		BuildAccelerationStructure(cmdBuffer
			, std::vector<VkAccelerationStructureGeometryKHR>{ as_geomKHR }, std::vector<uint32_t>{ 1 }
			, particle_tlas, &particle_instanceBuffer, bUpdateAs, bBLAS, bDynamicAsSize);


		//
		//Update Tlas-DescriptorSet
		//if (!bBLAS)
		{
			//
			//Tlas
			//Allocate-DescriptorSet in **RT**
			//	Create DescriptorSetLayout
			VkDescriptorSetLayoutBinding	descriptorSetLayoutBinding;
			//if (particleAs.GetDescriptorSetLayout() == VK_NULL_HANDLE || particleAs.GetDescriptorSet() == VK_NULL_HANDLE)
			if (particle_tlas.GetDescriptorSetLayout() == VK_NULL_HANDLE || particle_tlas.GetDescriptorSet() == VK_NULL_HANDLE)
			{
				//
				descriptorSetLayoutBinding.binding = 0;
				//https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorType.html
				//When a descriptor set is updated via elements of VkWriteDescriptorSet
				//	, members of pImageInfo, pBufferInfo and pTexelBufferView are only accessed by the implementation when they correspond to descriptor type being defined 
				//	- otherwise they are ignored. The members accessed are as follows for each descriptor type:
				//For VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, **VK_DESCRIPTOR_TYPE_STORAGE_BUFFER**, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, or VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
				//	, all members of each element of VkWriteDescriptorSet::pBufferInfo are accessed
				descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
				descriptorSetLayoutBinding.descriptorCount = 1;
				// | VK_SHADER_STAGE_CALLABLE_BIT_KHR
				descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR;
				//
				VkDescriptorSetLayout			descriptorSetLayout;
				m_pDescriptorSetHeap->CreateDescriptorSetLayout(&std::vector<VkDescriptorSetLayoutBinding>{ descriptorSetLayoutBinding }
				, & descriptorSetLayout);
				//particle_instanceBuffer.SetDescriptorSetLayout(descriptorSetLayout);
				//particleAs.SetDescriptorSetLayout(descriptorSetLayout);
				particle_tlas.SetDescriptorSetLayout(descriptorSetLayout);
				//	Create DescriptorSet
				VkDescriptorSet					descriptorSet;
				m_pDescriptorSetHeap->AllocDescriptor(descriptorSetLayout, &descriptorSet);
				//particle_instanceBuffer.SetDescriptorSet(descriptorSet);
				//particleAs.SetDescriptorSet(descriptorSet);
				particle_tlas.SetDescriptorSet(descriptorSet);
				//
				// would be vkCmdBindDescriptorSets(VkDescriptorSet*) after OnCreateAabbBuffer()
				//vkCmdBindDescriptorSets(VkDescriptorSet*)
				//vkCmdBindPipeline(VkPipeline computePipeline)
				//
			}
			//

			//
			//UpdateFormat-DescriptorSet: **Bind TlasBuffer 2 DescriptorSet**
			//VkWriteDescriptorSetAccelerationStructureKHR 
			VkWriteDescriptorSetAccelerationStructureKHR	writeDescriptorSet_AsKHR{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR };
			{
				//VkAccelerationStructureKHR		accelerationStructureKHR = particleAs.GetAccelerationStructureKHR();
				VkAccelerationStructureKHR		accelerationStructureKHR = particle_tlas.GetAccelerationStructureKHR();
				writeDescriptorSet_AsKHR.accelerationStructureCount = 1;
				writeDescriptorSet_AsKHR.pAccelerationStructures = &accelerationStructureKHR;
			}
			//
			//For Descriptors in DescriptorSet
			//VkWriteDescriptorSet
			VkWriteDescriptorSet writeDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			{
				//writeDescriptorSet.pNext = NULL;
				//writeDescriptorSet.dstSet = particleAs.GetDescriptorSet();
				writeDescriptorSet.dstSet = particle_tlas.GetDescriptorSet();
				writeDescriptorSet.dstBinding = 0;
				//Starting element in that array: 
				// If the descriptor binding identified by dstSet and dstBinding has a descriptor type of VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK 
				//	then dstArrayElement specifies the Starting-byte-Offset within the binding.
				//	1: if 0-start-offset for Special-process|usage
				writeDescriptorSet.dstArrayElement = 0;
				//
				writeDescriptorSet.descriptorCount = 1;
				// | VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
				//For AS
				writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
				//
				//writeDescriptorSet.pBufferInfo = &write_descriptorBufferInfo;
				//
				//For Tlas
				writeDescriptorSet.pNext = &writeDescriptorSet_AsKHR;

				vkUpdateDescriptorSets(m_pDevice->GetDevice(), 1, &writeDescriptorSet, 0, NULL);
				//
			}

			//
		}
		//

	}

	
	void PhysicsRT::BuildAccelerationStructure(
		VkCommandBuffer cmdBuffer
		, std::vector<VkAccelerationStructureGeometryKHR>& Geometries
		, std::vector<uint32_t>& maxPrimitiveCounts	// array_AsGeomKHR.size()==maxPrimitiveCounts.size()
		, ParticleAccelerationStructure& particleAs
		//, ParticleAccelerationStructure* pSrc_particleAs
		, ParticleConfiguration* pParticleConfiguration
		, bool bUpdateAs, bool bBLAS, bool bDynamicAsSize
	)
	{
		//
		//bUpdateAs: true
		//VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR specifies that the *Destination-acceleration-structure* 
		// will be **Built** Using data-in a -*Source-acceleration-structure*, Updated by the specified-Geometries
		//
		//VkDevice device = m_pDevice->GetDevice();
		VkResult res;
		//
		
		//Begin Create+Build a AS+ScratchBuffer
		//Get Build-Size
		// 
		//Duplicate
		// Set build-geom info:	bottom|topLevel-AccelerationStructure-BuildGrometryInfo{KHR}
		//	Set ASBuildGeomInfo: VkAccelerationStructureBuildGeometryInfoKHR
		VkAccelerationStructureBuildGeometryInfoKHR		as_buildGeomInfoKHR{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		//VkAccelerationStructureKHR		currentAccelerationStructureKHR = particleAs.GetAccelerationStructureKHR();
		{
			//
			//as_buildGeomInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			//as_buildGeomInfoKHR.pNext = NULL;
			// VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR 
			//	,VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR是一种加速结构，其类型在构建时确定，用于特殊情况。在这些情况下，加速结构类型在创建时是未知的，但必须在构建时指定为顶部或底部
			if (bBLAS)
			{
				as_buildGeomInfoKHR.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			}
			else
			{
				as_buildGeomInfoKHR.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
			}
			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBuildAccelerationStructureFlagBitsKHR.html
			//VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR{AllowUpdate},VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR{Allow-SoureOfCopy}
			// ,VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR{PerferTrace},VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR{PerferBuild},VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR{PerLowMemory,RahterThan-Trace|Build}
			//AccelerationStructure:	| VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR
			as_buildGeomInfoKHR.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			//
			//if (bUpdateAs) { as_buildGeomInfoKHR.flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR; }
			as_buildGeomInfoKHR.flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
			//
			//VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR{DstAsBuildedUsingSpecifiedGeoms}, VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR{DstAsBuildedUsingSpecifiedGeomsOfSrcAs}
			// defaule=0:VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR
			if (bUpdateAs)
			{
				//as_buildGeomInfoKHR.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
				// For Update: VkAccelerationStructureKHR|Handle{pointer to an existing acceleration structure}
				//as_buildGeomInfoKHR.srcAccelerationStructure = (pSrc_particleAs != NULL) ? pSrc_particleAs->GetAccelerationStructureKHR() : currentAccelerationStructureKHR;
				//as_buildGeomInfoKHR.srcAccelerationStructure = (as_buildGeomInfoKHR.srcAccelerationStructure != VK_NULL_HANDLE) ? as_buildGeomInfoKHR.srcAccelerationStructure : currentAccelerationStructureKHR;
				as_buildGeomInfoKHR.srcAccelerationStructure = particleAs.GetAccelerationStructureKHR();
				//
				//as_buildGeomInfoKHR.srcAccelerationStructure = currentAccelerationStructureKHR;
				//as_buildGeomInfoKHR.dstAccelerationStructure = currentAccelerationStructureKHR;
			}
			/*
			else
			{
				//as_buildGeomInfoKHR.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
			}
			*/
			// For Update|Build: VkAccelerationStructureKHR|Handle{pointer to the target acceleration structure}
			//as_buildGeomInfoKHR.dstAccelerationStructure = currentAccelerationStructureKHR;
			// 
			//Finished
			//=> For Instance: geometryCount -> gl_GeometryIndexEXT
			// NumberOf pGeometries: uint32_t{Number of Geometries that will be built into dstAccelerationStructur}
			//as_buildGeomInfoKHR.geometryCount = 1;
			//as_buildGeomInfoKHR.geometryCount = static_cast<uint32_t>(Geometries.size());
			as_buildGeomInfoKHR.geometryCount = (uint32_t)(Geometries.size());
			// const VkAccelerationStructureGeometryKHR*:	pointer to an array of VkAccelerationStructureGeometryKHR structures.
			as_buildGeomInfoKHR.pGeometries = Geometries.data();
			//
			// const VkAccelerationStructureGeometryKHR* const*:	pointer to an array of pointers to VkAccelerationStructureGeometryKHR structures
			//as_buildGeomInfoKHR.ppGeometries
			// VkDeviceOrHostAddressKHR:	device or host address to memory that will be used as **Scratch Memory for the build**.
			//as_buildGeomInfoKHR.scratchData
			//
		}
		//

		// Get build-size info:	bottom|topLevle-AccelerationStructure-BuildSizeInfo{KHR}
		VkAccelerationStructureBuildSizesInfoKHR	as_buildSizeInfoKHR{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
		{
			//
			//as_buildSizeInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
			//as_buildSizeInfoKHR.pNext = NULL;
			// VkDeviceSize{Size in Bytes required in a VkAccelerationStructureKHR for a build or update operation.}
			//as_buildSizeInfoKHR.accelerationStructureSize
			// VkDeviceSize{Size in Bytes required in a scratch buffer for an Update operation}
			//as_buildSizeInfoKHR.updateScratchSize
			// VkDeviceSize{Size in Bytes required in a scratch buffer for a **Build operation**.}
			//as_buildSizeInfoKHR.buildScratchSize
			//

			//TOAlter
			//Deprecate: uint32_t particleNum = particle_AabbBuffer.GetParticleNum();
			//std::vector<uint32_t> maxPrimitiveCounts{ pParticleConfiguration->GetPrimitiveCount() };
			//=> For Instance: maxPrimitiveCount -> gl_PrimitiveID
			//
			//buildType: VkAccelerationStructureBuildTypeKHR: VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR
			// VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR{requests the memory requirement for operations performed by **either** the host, or the device.}
			//pBuildInfo: const VkAccelerationStructureBuildGeometryInfoKHR*
			//	pointer to a VkAccelerationStructureBuildGeometryInfoKHR structure describing parameters of a build operation.
			//pMaxPrimitiveCounts: const uint32_t*: uint32_t values defining the Number of primitives built into each geometry of array of pBuildInfo->geometryCount
			//	Pointer to an array of **pBuildInfo->geometryCount uint32_t values** Defining the Number of primitives built into each geometry
			//	**pMaxPrimitiveCounts[pBuildInfo->geometryCount]**: MaxPrimitiveCounts of each geometry = pMaxPrimitiveCounts[each]**
			//pSizeInfo: VkAccelerationStructureBuildSizesInfoKHR*: 1个
			//	which **Returns** the Size required for an **acceleration structure** and the Sizes required for the **scratch buffers**, given the build parameters
			//VkAccelerationStructureBuildTypeKHR:	VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR,	VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR 
			VkAccelerationStructureBuildTypeKHR         buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
			vkGetAccelerationStructureBuildSizesKHR(m_pDevice->GetDevice(), buildType
				, &as_buildGeomInfoKHR, maxPrimitiveCounts.data(), &as_buildSizeInfoKHR);
			//
			//Delta: The .srcAccelerationStructure, .dstAccelerationStructure, and .mode members of pBuildInfo are **Ignored**. 
			// Any VkDeviceOrHostAddressKHR members of pBuildInfo are ignored by this command, except that the hostAddress member of VkAccelerationStructureGeometryTrianglesDataKHR::transformData will be examined to check if it is NULL.

			//
		}
		//

		//
		// Allocate+Create BL|TL-AS
		// Create BL|TL-AS:		bottom|topLevel-AccelerationStructure{KHR}
		VkAccelerationStructureKHR		accelerationStructureKHR = particleAs.GetAccelerationStructureKHR();
		long long current_deltaSizeAs = (long long)particleAs.GetDescriptorBufferInfo().range - (long long)as_buildSizeInfoKHR.accelerationStructureSize;
		if (bDynamicAsSize || accelerationStructureKHR == VK_NULL_HANDLE || particleAs.GetDescriptorBufferInfo().buffer == VK_NULL_HANDLE
			//|| particleAs.GetDescriptorBufferInfo().range != as_buildSizeInfoKHR.accelerationStructureSize)
			|| current_deltaSizeAs < 0 || std::abs(current_deltaSizeAs) > m_deltaSizeAs)
		{
			
			//
			//TOAlter
			//if (!bUpdateAs && accelerationStructureKHR != VK_NULL_HANDLE)
			if (accelerationStructureKHR != VK_NULL_HANDLE)
			{
				//Delta
				bUpdateAs = false;
				// **Since Offet-Allocation Buffer, So only Free|Destroy by OnDestroy()**
				//vkFreeMemory(device, memory, nullptr);
				//vkDestroyBuffer(device, buffer, nullptr);
				// 
				//	const VkAllocationCallbacks* pAllocator=nullptr
				//vkDestroyAccelerationStructureKHR(m_pDevice->GetDevice(), accelerationStructureKHR, nullptr);
				//VkDescriptorBufferInfo	descriptorBufferInfo;
				VkDescriptorSet			descriptorSet;
				//particleAs.OnDestroy(m_pDevice->GetDevice(), &descriptorBufferInfo, &descriptorSet);
				particleAs.OnDestroy(m_pDevice->GetDevice(), &descriptorSet);
				//descriptorSet
				m_pDescriptorSetHeap->FreeDescriptor(descriptorSet);
				//NONEED: Recycle descriptorBufferInfo
				/*
				if (bDynamicAsSize)
				{
					//No Need
					//m_pRtBufferDynamic_GPU
				}
				else
				{
					//TODO
					//m_pRtBufferStatic_GPU
				}
				*/
				//
				accelerationStructureKHR = VK_NULL_HANDLE;
				//particleAs.SetAccelerationStructureKHR(VK_NULL_HANDLE);
				
				//
			}
			//
			CreateAccelerationStructure(particleAs, as_buildSizeInfoKHR, bBLAS, bDynamicAsSize);
			accelerationStructureKHR = particleAs.GetAccelerationStructureKHR();
			//

			//
			/*
			// Offet-Allocate Buffer{VkDescriptorBufferInfo} for BL|TL-AS{VkAccelerationStructureKHR}
			VkDescriptorBufferInfo	descriptorBufferInfo;
			//TODO
			// Allocate(as_buildSizeInfoKHR.accelerationStructureSize, descriptorBufferInfo)
			if (bDynamicAsSize)
			{
				// Dynamic-Allocate
			}
			else
			{
				// Static-Allocate
			}
			//
			particleAs.SetDescriptorBufferInfo(descriptorBufferInfo);
			//Delta: The acceleration structure device address **may be different** from the buffer device address corresponding to the acceleration structure’s start offset in its storage buffer for acceleration structure types 
			// other than VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR
			//=> So use vkGetAccelerationStructureDeviceAddressKHR() to set particleAs.SetDeviceAddress() after Create-AccelerationStructure
			//Uselse-TODO: particleAs.SetDeviceAddress()
			//
			// Create BL|TL-AS{VkAccelerationStructureKHR}
			VkAccelerationStructureCreateInfoKHR	as_createInfoKHR{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
			//as_createInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
			//as_createInfoKHR.pNext = NULL;
			//VkAccelerationStructureCreateFlagBitsKHR
			// VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR{指定加速结构的地址可以保存并在后续运行中重复使用}
			//	,VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT{指定加速结构在捕获和重放时可以与描述符缓冲区一起使用（例如，用于跟踪捕获和重放），请参阅VkOpaqueCaptureDescriptorDataCreateInfoEXT了解更多详细信息。}
			//	,VK_ACCELERATION_STRUCTURE_CREATE_MOTION_BIT_NV
			//as_createInfoKHR.createFlags=
			//
			//VkBuffer {Buffer on which the acceleration structure will be stored.}
			as_createInfoKHR.buffer = descriptorBufferInfo.buffer;
			//VkDeviceSize {Offset in bytes from the base address of the buffer at which the acceleration structure will be stored, and **must be a multiple of 256**}
			//	分配的Particle数据的大小应该是256的倍数，或者另外开一个bufferAlloc_ter
			as_createInfoKHR.offset = descriptorBufferInfo.offset;
			//VkDeviceSize {Size required for the acceleration structure.}
			as_createInfoKHR.size = descriptorBufferInfo.range;
			//
			//VkAccelerationStructureTypeKHR{Same as	VkAccelerationStructureBuildGeometryInfoKHR.type}
			// VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR ,	VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR是一种加速结构，其类型在构建时确定，用于特殊情况。在这些情况下，加速结构类型在创建时是未知的，但必须在构建时指定为顶部或底部
			if (bBLAS)
			{
				as_createInfoKHR.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			}
			else
			{
				as_createInfoKHR.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
			}
			//
			//VkDeviceAddress{Device-address requested for the acceleration structure if the accelerationStructureCaptureReplay feature is being used.}
			//as_createInfoKHR.deviceAddress=
			//
			//vkCreateAccelerationStructureKHR
			//pCreateInfo: const VkAccelerationStructureCreateInfoKHR*: 
			//	Pointer to **a** VkAccelerationStructureCreateInfoKHR **structure** containing parameters affecting creation of the acceleration structure.
			//pAllocator=nullptr: const VkAllocationCallbacks*: controls host memory allocation as described in the Memory Allocation chapter
			//pAccelerationStructure: VkAccelerationStructureKHR*:
			//	Pointer to **a** VkAccelerationStructureKHR **handle** in which the **resulting acceleration structure object is returned**.
			res = vkCreateAccelerationStructureKHR(m_pDevice->GetDevice(), &as_createInfoKHR, NULL, &accelerationStructureKHR);
			assert(res == VK_SUCCESS);
			// Create+Set	VkAccelerationStructureKHR
			particleAs.SetAccelerationStructureKHR(accelerationStructureKHR);
			*/
			//
		}
		//

		//
		// Get AS-deviceAddress:		bottom|topLevel-AccelerationStructure-DeviceAddressInfo{KHR}
		VkAccelerationStructureDeviceAddressInfoKHR		accelerationStructure_deviceAddressInfoKHR{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
		if(particleAs.GetAccelerationStructureDeviceAddress() == NULL)
		{
			//
			//accelerationStructure_deviceAddressInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
			//accelerationStructure_deviceAddressInfoKHR.pNext = NULL;
			//
			accelerationStructure_deviceAddressInfoKHR.accelerationStructure = accelerationStructureKHR;
			//
			//vkGetAccelerationStructureDeviceAddressKHR
			//pInfo: const VkAccelerationStructureDeviceAddressInfoKHR*:
			//	pointer to **a** VkAccelerationStructureDeviceAddressInfoKHR **structure** specifying the acceleration structure to **retrieve{取回} an address** for
			VkDeviceAddress		as_deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(m_pDevice->GetDevice(), &accelerationStructure_deviceAddressInfoKHR);
			assert(as_deviceAddress);
			particleAs.SetAccelerationStructureDeviceAddress(as_deviceAddress);
			//If the acceleration structure was created with a non-zero value of 
			//	VkAccelerationStructureCreateInfoKHR::deviceAddress{if the accelerationStructureCaptureReplay feature is being used}, the return value will be the same address.
			//The returned address must be aligned to 256 bytes.{I implement it with Allocate alignUp-bufferSize[256u] buffer-memory}
			//
		}
		//

		//Use ScratchBuffer to Build|Update
		//ScratchBuffer
		// Allocate ScratchBuffer:	bottomLevel-ScratchBuffer
		//	For VkAccelerationStructureBuildGeometryInfoKHR
		// {Create a small scratch buffer used during Build of the bottom level acceleration structure}
		VkDescriptorBufferInfo	scratchBuffer_descriptorBufferInfo;
		VkDeviceAddress			scratchBuffer_deviceAddress;
		{
			VkDeviceSize	deviceSize;
			if (bUpdateAs)
			{
				deviceSize = as_buildSizeInfoKHR.updateScratchSize;
			}
			else
			{
				deviceSize = as_buildSizeInfoKHR.buildScratchSize;
			}
			//
			//TODO
			//Dynamic Allocation
			// Allocate(deviceSize, scratchBuffer_descriptorBufferInfo)
			// GetBufferDeviceAddress()
			//AllocBuffer(uint32_t size, void** ppBuffer, VkDeviceAddress* pDeviceAddressBuffer, VkDescriptorBufferInfo* pOut);
			m_pRtBufferDynamic_GPU->AllocBuffer(deviceSize, NULL, &scratchBuffer_deviceAddress, &scratchBuffer_descriptorBufferInfo);
			// 
			//
		}
		//

		// Set build-geom info:		bottom|topLevel-AccelerationStructure-ScratchBuffer-BuildGeometryInfo{KHR}
		// Refer to <Duplicate>
		//Deprecate: VkAccelerationStructureBuildGeometryInfoKHR		as_buildGeomInfoKHR{};
		VkAccelerationStructureBuildGeometryInfoKHR		scratchBuffer_AsBuildGeomInfoKHR{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		{
			//
			//scratchBuffer_AsBuildGeomInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			//scratchBuffer_AsBuildGeomInfoKHR.pNext = NULL;
			// VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR ,	VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR是一种加速结构，其类型在构建时确定，用于特殊情况。在这些情况下，加速结构类型在创建时是未知的，但必须在构建时指定为顶部或底部
			if (bBLAS)
			{
				scratchBuffer_AsBuildGeomInfoKHR.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			}
			else
			{
				scratchBuffer_AsBuildGeomInfoKHR.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
			}
			//
			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBuildAccelerationStructureFlagBitsKHR.html
			//VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR{AllowUpdate},VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR{Allow-SoureOfCopy}
			// ,VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR{PerferTrace},VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR{PerferBuild},VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR{PerLowMemory,RahterThan-Trace|Build}
			//ScratchBuffer
			//scratchBuffer_AsBuildGeomInfoKHR.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
			scratchBuffer_AsBuildGeomInfoKHR.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			//
			scratchBuffer_AsBuildGeomInfoKHR.flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
			//
			//VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR{DstAsBuildedUsingSpecifiedGeoms}, VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR{DstAsBuildedUsingSpecifiedGeomsOfSrcAs}
			// defaule=0:VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR
			//**Update Usually use Same AccelerationStructureKHR-Handle**
			//	bUpdateAs may be Altered2 false if accelerationStructureKHR!=VK_NULL_HANDLE && buildSizeInfo.accelerationStructureSize != particleAs_buffer.range
			if (bUpdateAs)
			{
				//
				scratchBuffer_AsBuildGeomInfoKHR.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
				// For Update: VkAccelerationStructureKHR|Handle{pointer to an existing acceleration structure}
				//TOAlter
				scratchBuffer_AsBuildGeomInfoKHR.srcAccelerationStructure = accelerationStructureKHR;
				//scratchBuffer_AsBuildGeomInfoKHR.srcAccelerationStructure = as_buildGeomInfoKHR.srcAccelerationStructure;
			}
			else
			{
				scratchBuffer_AsBuildGeomInfoKHR.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
			}
			//ScratchBuffer
			//TOAlter
			// For Build|Update: VkAccelerationStructureKHR|Handle{pointer to the target acceleration structure}
			scratchBuffer_AsBuildGeomInfoKHR.dstAccelerationStructure = accelerationStructureKHR;
			//
			// NumberOf pGeometries: uint32_t{Number of Geometries that will be built into dstAccelerationStructur}
			//scratchBuffer_AsBuildGeomInfoKHR.geometryCount = 1;
			//scratchBuffer_AsBuildGeomInfoKHR.geometryCount = static_cast<uint32_t>(Geometries.size());
			scratchBuffer_AsBuildGeomInfoKHR.geometryCount = (uint32_t)Geometries.size();
			// const VkAccelerationStructureGeometryKHR*:	pointer to an array of VkAccelerationStructureGeometryKHR structures.
			//scratchBuffer_AsBuildGeomInfoKHR.pGeometries = &as_geomKHR;
			scratchBuffer_AsBuildGeomInfoKHR.pGeometries = Geometries.data();
			// 
			// const VkAccelerationStructureGeometryKHR* const*:	pointer to an array of pointers to VkAccelerationStructureGeometryKHR structures
			//scratchBuffer_AsBuildGeomInfoKHR.ppGeometries
			//
			//ScratchBuffer
			//
			if (scratchBuffer_deviceAddress == NULL)
			{
				VkBufferDeviceAddressInfo buffer_deviceAddressInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
				//
				//buffer_deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
				//buffer_deviceAddressInfo.pNext = NULL;
				buffer_deviceAddressInfo.buffer = scratchBuffer_descriptorBufferInfo.buffer;
				scratchBuffer_deviceAddress = vkGetBufferDeviceAddressKHR(m_pDevice->GetDevice(), &buffer_deviceAddressInfo);
				assert(scratchBuffer_deviceAddress);
				scratchBuffer_deviceAddress += scratchBuffer_descriptorBufferInfo.offset;
				//scratchBuffer_AsBuildGeomInfoKHR.scratchData.deviceAddress = scratchBuffer_deviceAddress;
				//
			}
			// VkDeviceOrHostAddressKHR:	device or host address to memory that will be used as **Scratch Memory for the build**.
			scratchBuffer_AsBuildGeomInfoKHR.scratchData.deviceAddress = scratchBuffer_deviceAddress;
			//
		}
		//

		// Set build-range info:	bottomLevel-AccelerationStructure-ScratchBuffer-BuildRangeInfo{KHR}
		VkAccelerationStructureBuildRangeInfoKHR		as_buildRangeInfoKHR{};
		{
			//uint32_t:		All Below
			//Deprecate: as_buildRangeInfoKHR.primitiveCount = particle_AabbBuffer.GetParticleNum();
			as_buildRangeInfoKHR.primitiveCount = pParticleConfiguration->GetPrimitiveCount();
			//TOComplete
			as_buildRangeInfoKHR.primitiveOffset = 0;
			as_buildRangeInfoKHR.firstVertex = 0;
			as_buildRangeInfoKHR.transformOffset = 0;
			//
		}
		//
		//End Create+Build a AS+ScratchBuffer

		// Build AS through cmd:	bottomLevel-AccelerationStructure{KHR}
		//	Build the acceleration structure on the device via a one-time command buffer submission
		//	Some implementations may support acceleration structure building on the Host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands)
		//	, but we Prefer Device builds
		//
		{
			//
			//TOComplete
			std::vector<VkAccelerationStructureBuildGeometryInfoKHR>	array_scratchBufferAsBuildGeomInfoKHR{ scratchBuffer_AsBuildGeomInfoKHR };
			std::vector<VkAccelerationStructureBuildRangeInfoKHR*>		array_pAsBuildRangeInfoKHR{ &as_buildRangeInfoKHR };
			//Number of acceleration structures to build
			//uint32_t infoCount = 1;
			//uint32_t infoCount = static_cast<uint32_t>(array_scratchBufferAsBuildGeomInfoKHR.size());
			uint32_t infoCount = (uint32_t)array_scratchBufferAsBuildGeomInfoKHR.size();
			//
			//vkBuildAccelerationStructuresKHR{Build an acceleration structure on the Host}
			//vkCmdBuildAccelerationStructuresIndirectKHR{Build an acceleration structure with some Parameters Provided on the Device}
			//vkCmdBuildAccelerationStructuresKHR{Build an acceleration structure}
			//
			//infoCount: uint32_t: Number of acceleration structures to build. It Specifies the number of the pInfos structures + ppBuildRangeInfos pointers that Must be provided
			//pInfos: const VkAccelerationStructureBuildGeometryInfoKHR*: pointer to an array of infoCount个 VkAccelerationStructureBuildGeometryInfoKHR structures that Defining the geometry be Used to build each acceleration structure.
			//ppBuildRangeInfos: const VkAccelerationStructureBuildRangeInfoKHR* const*: pointer to an array of infoCount个 Pointers to arrays of VkAccelerationStructureBuildRangeInfoKHR structures
			//	Each ppBuildRangeInfos[i] is a Pointer to an array of **pInfos[i].geometryCount** VkAccelerationStructureBuildRangeInfoKHR structures Defining dynamic offsets to the addresses where geometry data is stored, as defined by pInfos[i]
			//vkCmdBuildAccelerationStructuresKHR(cmdBuffer, infoCount, &as_buildGeomInfoKHR, array_pAsBuildRangeInfoKHR.data());
			vkCmdBuildAccelerationStructuresKHR(cmdBuffer, infoCount, array_scratchBufferAsBuildGeomInfoKHR.data(), array_pAsBuildRangeInfoKHR.data());
			//
		}
		//
		
		//
	}


	void PhysicsRT::CreateAccelerationStructure(
		ParticleAccelerationStructure& particleAs
		, VkAccelerationStructureBuildSizesInfoKHR& as_buildSizeInfoKHR
		, bool bBLAS, bool bDynamicAsSize
	)
	{
		
		VkResult res;


		//
		// Allocate+Create BL|TL-AS
		// Create BL|TL-AS:		bottom|topLevel-AccelerationStructure{KHR}
		VkAccelerationStructureKHR		accelerationStructureKHR = particleAs.GetAccelerationStructureKHR();
		//if (bDynamicAsSize || accelerationStructureKHR == VK_NULL_HANDLE || particleAs.GetDescriptorBufferInfo().buffer == VK_NULL_HANDLE
		//	|| particleAs.GetDescriptorBufferInfo().range != as_buildSizeInfoKHR.accelerationStructureSize)
		{

			//
			// Offet-Allocate Buffer{VkDescriptorBufferInfo} for BL|TL-AS{VkAccelerationStructureKHR}
			VkDescriptorBufferInfo	descriptorBufferInfo;
			VkDeviceAddress			deviceAddress;
			//TOAlter
			// Allocate(as_buildSizeInfoKHR.accelerationStructureSize, descriptorBufferInfo)
			if (bDynamicAsSize)
			{
				// Dynamic-Allocate
				//AllocBuffer(uint32_t size, void** ppBuffer, VkDeviceAddress* pDeviceAddressBuffer, VkDescriptorBufferInfo* pOut);
				m_pRtBufferDynamic_GPU->AllocBuffer(as_buildSizeInfoKHR.accelerationStructureSize, NULL, &deviceAddress, &descriptorBufferInfo);
			}
			else
			{
				// Static-Allocate
				//AllocBuffer(uint32_t numbeOfElements, uint32_t strideInBytes
				//	, void** ppBuffer, VkDeviceAddress* pDeviceAddressBuffer
				//	, VkDescriptorBufferInfo* pOut);
				m_pRtBufferStatic_GPU->AllocBuffer(1, as_buildSizeInfoKHR.accelerationStructureSize, NULL, &deviceAddress, &descriptorBufferInfo);
			}
			//
			particleAs.SetDescriptorBufferInfo(descriptorBufferInfo);
			//Delta: The acceleration structure device address **may be different** from the buffer device address corresponding to the acceleration structure’s start offset in its storage buffer for acceleration structure types 
			// other than VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR
			//=> So use vkGetAccelerationStructureDeviceAddressKHR() to set particleAs.SetDeviceAddress() after Create-AccelerationStructure
			//Uselse-TODO: particleAs.SetBufferDeviceAddress()
			//be replaced by GetAccelerationStructureDeviceAddress() later
			particleAs.SetBufferDeviceAddress(deviceAddress);
			//
			// Create BL|TL-AS{VkAccelerationStructureKHR}
			VkAccelerationStructureCreateInfoKHR	as_createInfoKHR{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
			//as_createInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
			//as_createInfoKHR.pNext = NULL;
			//VkAccelerationStructureCreateFlagBitsKHR
			// VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR{指定加速结构的地址可以保存并在后续运行中重复使用}
			//	,VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT{指定加速结构在捕获和重放时可以与描述符缓冲区一起使用（例如，用于跟踪捕获和重放），请参阅VkOpaqueCaptureDescriptorDataCreateInfoEXT了解更多详细信息。}
			//	,VK_ACCELERATION_STRUCTURE_CREATE_MOTION_BIT_NV
			//as_createInfoKHR.createFlags=
			//
			//VkBuffer {Buffer on which the acceleration structure will be stored.}
			as_createInfoKHR.buffer = descriptorBufferInfo.buffer;
			//VkDeviceSize {Offset in bytes from the base address of the buffer at which the acceleration structure will be stored, and **must be a multiple of 256**}
			//	分配的Particle数据的大小应该是256的倍数，或者另外开一个bufferAlloc_ter
			as_createInfoKHR.offset = descriptorBufferInfo.offset;
			//VkDeviceSize {Size required for the acceleration structure.}
			as_createInfoKHR.size = descriptorBufferInfo.range;
			//
			//VkAccelerationStructureTypeKHR{Same as	VkAccelerationStructureBuildGeometryInfoKHR.type}
			// VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR ,	VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR是一种加速结构，其类型在构建时确定，用于特殊情况。在这些情况下，加速结构类型在创建时是未知的，但必须在构建时指定为顶部或底部
			if (bBLAS)
			{
				as_createInfoKHR.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			}
			else
			{
				as_createInfoKHR.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
			}
			//
			//VkDeviceAddress{Device-address requested for the acceleration structure if the accelerationStructureCaptureReplay feature is being used.}
			//as_createInfoKHR.deviceAddress=
			// Create	VkAccelerationStructureKHR
			//vkCreateAccelerationStructureKHR
			//pCreateInfo: const VkAccelerationStructureCreateInfoKHR*: 
			//	Pointer to **a** VkAccelerationStructureCreateInfoKHR **structure** containing parameters affecting creation of the acceleration structure.
			//pAllocator=nullptr: const VkAllocationCallbacks*: controls host memory allocation as described in the Memory Allocation chapter
			//pAccelerationStructure: VkAccelerationStructureKHR*:
			//	Pointer to **a** VkAccelerationStructureKHR **handle** in which the **resulting acceleration structure object is returned**.
			res = vkCreateAccelerationStructureKHR(m_pDevice->GetDevice(), &as_createInfoKHR, NULL, &accelerationStructureKHR);
			assert(res == VK_SUCCESS);
			// CSet	VkAccelerationStructureKHR
			particleAs.SetAccelerationStructureKHR(accelerationStructureKHR);
			//

		}
		//


		//
		// Get AS-deviceAddress:		bottom|topLevel-AccelerationStructure-DeviceAddressInfo{KHR}
		VkAccelerationStructureDeviceAddressInfoKHR		accelerationStructure_deviceAddressInfoKHR{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
		// AccelerationStructureDeviceAddress may Different from BufferAddress
		//So vkGetAccelerationStructureDeviceAddress()
		//if (particleAs.GetAccelerationStructureDeviceAddress() == NULL)
		{
			//
			//accelerationStructure_deviceAddressInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
			//accelerationStructure_deviceAddressInfoKHR.pNext = NULL;
			//
			accelerationStructure_deviceAddressInfoKHR.accelerationStructure = accelerationStructureKHR;
			//
			//vkGetAccelerationStructureDeviceAddressKHR
			//pInfo: const VkAccelerationStructureDeviceAddressInfoKHR*:
			//	pointer to **a** VkAccelerationStructureDeviceAddressInfoKHR **structure** specifying the acceleration structure to **retrieve{取回} an address** for
			VkDeviceAddress		as_deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(m_pDevice->GetDevice(), &accelerationStructure_deviceAddressInfoKHR);
			assert(as_deviceAddress);
			particleAs.SetAccelerationStructureDeviceAddress(as_deviceAddress);
			//If the acceleration structure was created with a non-zero value of 
			//	VkAccelerationStructureCreateInfoKHR::deviceAddress{if the accelerationStructureCaptureReplay feature is being used}, the return value will be the same address.
			//The returned address must be aligned to 256 bytes.{I implement it with Allocate alignUp-bufferSize[256u] buffer-memory}
			//
		}
		//

		

	}


}
