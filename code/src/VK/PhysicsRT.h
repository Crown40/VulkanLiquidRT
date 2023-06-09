
#pragma once

#include "stdafx.h"
#include "base/Device.h"
#include "HeapDescriptorSetAllocater.h"
#include "DynamicBufferAllocater.h"
#include "StaticBufferAllocater.h"

namespace CAULDRON_VK
{
	//
	//
	// Available features and properties
	// Extensions and its Features and Properties
	// Have been Implemented by ExtRTCheckExtensions() be Called in Device::OnCreate() through Device::SetEssentialDeviceExtensions()
	//VkPhysicalDeviceRayTracingPipelinePropertiesKHR		m_physicalDeviceRTpipelineProperties = {};
	//VkPhysicalDeviceAccelerationStructureFeaturesKHR	m_physicalDeviceAccelerationStructureFeatures = {};
	/*
	// Enabled features and properties
	//VkPhysicalDeviceRayTracingPipelineFeaturesKHR
	//VkPhysicalDeviceRayTracingPipelinePropertiesKHR
	//VkPhysicalDeviceAccelerationStructureFeaturesKHR
	//VkPhysicalDeviceAccelerationStructurePropertiesKHR
	//VkPhysicalDeviceRayQueryFeaturesKHR
	//VkPhysicalDeviceRayTracingMotionBlurFeaturesNV
	//VkPhysicalDeviceRayTracingPropertiesNV
	//VkPhysicalDeviceBufferDeviceAddressFeatures
	*/
	//

	//

	//Deprecate: typedef struct,class
	struct ParticleConfiguration
	{
	public:
		//
		VkDescriptorBufferInfo				m_particle_descriptorBufferInfo;
		//VkDeviceOrHostAddressKHR,VkDeviceOrHostAddressConstKHR
		VkDeviceOrHostAddressConstKHR		m_particle_bufferAddress;
		//
		VkDescriptorSetLayout				m_particle_descriptorSetLayout;
		VkDescriptorSet						m_particle_descriptorSet;
		//Alter
		uint32_t							m_primitiveCount;
		//
		VkDescriptorBufferInfo GetDescriptorBufferInfo() { return m_particle_descriptorBufferInfo; }
		void SetDescriptorBufferInfo(VkDescriptorBufferInfo		descriptorBufferInfo) { m_particle_descriptorBufferInfo = descriptorBufferInfo; }
		//VkDeviceOrHostAddressKHR,VkDeviceOrHostAddressConstKHR
		VkDeviceOrHostAddressConstKHR GetBufferAddress() { return m_particle_bufferAddress; }
		void SetBufferHostAddress(const void* hostAddress) { m_particle_bufferAddress.hostAddress = hostAddress; }
		void SetBufferDeviceAddress(VkDeviceAddress	deviceAddress) { m_particle_bufferAddress.deviceAddress = deviceAddress; }
		//
		VkDescriptorSetLayout GetDescriptorSetLayout() { return m_particle_descriptorSetLayout; }
		void SetDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) { m_particle_descriptorSetLayout = descriptorSetLayout; }
		VkDescriptorSet GetDescriptorSet() { return m_particle_descriptorSet; }
		void SetDescriptorSet(VkDescriptorSet descriptorSet) { m_particle_descriptorSet = descriptorSet; }
		//
		virtual uint32_t GetPrimitiveCount() { return m_primitiveCount; }
		virtual void SetPrimitiveCount(uint32_t primitiveCount) { m_primitiveCount = primitiveCount; }
		//
		//void OnDestroy(VkDevice device, VkDescriptorBufferInfo* pOut_desciptorBufferInfo, VkDescriptorSet* pOut_desciptorSet)
		void OnDestroy(VkDevice device, VkDescriptorSet* pOut_desciptorSet)
		{

			//
			//*pOut_desciptorBufferInfo = m_particle_descriptorBufferInfo;
			m_particle_descriptorBufferInfo = { VK_NULL_HANDLE,0,0 };
			m_particle_bufferAddress.deviceAddress = NULL;
			//
			//If descriptorSetLayout is not VK_NULL_HANDLE, descriptorSetLayout must be a valid VkDescriptorSetLayout handle
			vkDestroyDescriptorSetLayout(device, m_particle_descriptorSetLayout, NULL);
			m_particle_descriptorSetLayout = VK_NULL_HANDLE;
			//vkDestroyDesciptorSet: vkFreeDescriptorSets() be called by Heap|Count-DescriptorSetAllocater::FreeDescriptor()
			//vkFreeDescriptorSets(): Each element of pDescriptorSets that is a valid handle must have been created, allocated, or retrieved from descriptorPool
			//	So use pOut to catch{actually dangerous here}
			*pOut_desciptorSet = m_particle_descriptorSet;
			m_particle_descriptorSet = VK_NULL_HANDLE;
			//
			m_primitiveCount = 0;
			//
		}
		//
	};

	struct ParticleAabbPositionAlign
	{
		float    minX;float    minY;float    minZ;
		float    maxX;float    maxY;float    maxZ;
		float	 minAlign;float	 maxAlign;
	};

	struct ParticleAabbBuffer :public ParticleConfiguration
	{
	public:
		//Deprecate
		//uint32_t						m_NumParticle;
		//
		uint32_t GetParticleNum() { return m_primitiveCount; }
		//void SetParticleNum(uint32_t NumParticle) { m_primitiveCount = NumParticle; }
		void OnInit(uint32_t numParticle){ m_primitiveCount = numParticle; }
		//
		uint32_t GetPrimitiveCount() override{ return m_primitiveCount; }
		void SetPrimitiveCount(uint32_t NumParticle) override { m_primitiveCount = NumParticle; }
		//
		/*
		//Deprecate VkBuffer_T*{Handle}
		//VkBuffer						m_particlePosition_buffer;
		// 
		//VkDescriptorBufferInfo{VkBuffer buffer,VkDeviceSize offset,VkDeviceSize range}
		VkDescriptorBufferInfo			m_particleData_descriptorBufferInfo;
		// union{deviceAddress[uint64_t] / hostAddress[const void*]}
		VkDeviceOrHostAddressConstKHR	m_particleData_bufferAddress;
		VkDescriptorBufferInfo GetDescriptorBufferInfo() { return m_particleData_descriptorBufferInfo; }
		void SetDescriptorBufferInfo(VkDescriptorBufferInfo		buffer_descriptorBufferInfo) { m_particleData_descriptorBufferInfo = buffer_descriptorBufferInfo; }
		//
		VkDeviceOrHostAddressConstKHR GetBufferAddress(){ return m_particleData_bufferAddress; }
		void SetBufferHostAddress(const void*	hostAddress) { m_particleData_bufferAddress.hostAddress = hostAddress; }
		void SetBufferDeviceAddress(VkDeviceAddress	deviceAddress) { m_particleData_bufferAddress.deviceAddress = deviceAddress; }
		//
		*/
		//
	};

	struct ParticleAccelerationStructure :public ParticleConfiguration
	{
	public:
		//
		VkAccelerationStructureKHR		m_particle_accelerationStructureKHR = VK_NULL_HANDLE;
		VkDeviceAddress					m_particle_accelerationStructureDeviceAddress = NULL;
		//Deprecate: struct
		//VkAllocationCallbacks			m_particleData_allocationCallbacks;
		// VkAccelerationStructureKHR or VkDeviceAddress
		VkAccelerationStructureKHR GetAccelerationStructureKHR() { return m_particle_accelerationStructureKHR; }
		void SetAccelerationStructureKHR(VkAccelerationStructureKHR		accelerationStructureKHR) { m_particle_accelerationStructureKHR = accelerationStructureKHR; }
		VkDeviceAddress GetAccelerationStructureDeviceAddress() { return m_particle_accelerationStructureDeviceAddress; }
		void SetAccelerationStructureDeviceAddress(VkDeviceAddress		accelerationStructureDeviceAddress) { m_particle_accelerationStructureDeviceAddress = accelerationStructureDeviceAddress; }
		//
		//
		//void OnDestroy(VkDevice device, VkDescriptorBufferInfo* pOut_desciptorBufferInfo, VkDescriptorSet* pOut_desciptorSet)
		void OnDestroy(VkDevice device, VkDescriptorSet* pOut_desciptorSet)
		{
			//
			//ParticleConfiguration::OnDestroy(device, pOut_desciptorBufferInfo, pOut_desciptorSet);
			ParticleConfiguration::OnDestroy(device, pOut_desciptorSet);
			//If accelerationStructure is not VK_NULL_HANDLE, accelerationStructure must be a valid VkAccelerationStructureKHR handle
			PFN_vkDestroyAccelerationStructureKHR			vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
			vkDestroyAccelerationStructureKHR(device, m_particle_accelerationStructureKHR, NULL);
			//
			m_particle_accelerationStructureKHR = VK_NULL_HANDLE;
			m_particle_accelerationStructureDeviceAddress = NULL;
			//
			
		}
		//
		/*
		VkDescriptorBufferInfo			m_particleData_descriptorBufferInfo;
		VkDeviceOrHostAddressConstKHR	m_particleData_bufferAddress;
		//
		VkDescriptorBufferInfo GetDescriptorBufferInfo() { return m_particleData_descriptorBufferInfo; }
		void SetDescriptorBufferInfo(VkDescriptorBufferInfo		descriptorBufferInfo) { m_particleData_descriptorBufferInfo = descriptorBufferInfo; }
		//
		VkDeviceOrHostAddressConstKHR GetBufferAddress() { return m_particleData_bufferAddress; }
		//Deprecate
		//VkDeviceAddress GetDeviceAddress() { return m_particleData_bufferAddress.deviceAddress; }
		void SetBufferHostAddress(const void* hostAddress) { m_particleData_bufferAddress.hostAddress = hostAddress; }
		void SetBufferDeviceAddress(VkDeviceAddress	deviceAddress) { m_particleData_bufferAddress.deviceAddress = deviceAddress; }
		//
		*/
		//
	};

	struct ParticleInstanceBuffer :public ParticleConfiguration
	{
	public:
		//
		VkAccelerationStructureInstanceKHR	m_particle_asInstanceKHR;
		//Deprecate
		//uint32_t							m_particle_NumInstance = 1;
		uint32_t							m_primitiveCount = 1;
		//
		uint32_t							m_instanceCustomIndex;
		uint32_t							m_instanceShaderBindingTableRecordOffset;
		//
		VkAccelerationStructureInstanceKHR GetAccelerationStructureInstanceKHR() { return m_particle_asInstanceKHR; }
		void SetAccelerationStructureInstanceKHR(VkAccelerationStructureInstanceKHR		asInstanceKHR) { m_particle_asInstanceKHR = asInstanceKHR; }
		//
		uint32_t GetPrimitiveCount() override { return m_primitiveCount; }
		void SetPrimitiveCount(uint32_t numInstance) override { m_primitiveCount = numInstance; }
		//
		uint32_t GetInstanceCustomIndex() { return m_instanceCustomIndex; }
		void SetInstanceCustomIndex(uint32_t instanceCustomIndex) { m_instanceCustomIndex = instanceCustomIndex; }
		uint32_t GetInstanceShaderBindingTableRecordOffset() { return m_instanceShaderBindingTableRecordOffset; }
		void SetInstanceShaderBindingTableRecordOffset(uint32_t instanceShaderBindingTableRecordOffset) { m_instanceShaderBindingTableRecordOffset = instanceShaderBindingTableRecordOffset; }
		//
		//void OnDestroy(VkDevice device, VkDescriptorBufferInfo* pOut_desciptorBufferInfo, VkDescriptorSet* pOut_desciptorSet)
		void OnDestroy(VkDevice device, VkDescriptorSet* pOut_desciptorSet)
		{

			//
			//ParticleConfiguration::OnDestroy(device, pOut_desciptorBufferInfo, pOut_desciptorSet);
			ParticleConfiguration::OnDestroy(device, pOut_desciptorSet);
			//
			m_particle_asInstanceKHR = VkAccelerationStructureInstanceKHR{};
			//
			//m_primitiveCount = 0;
			m_instanceCustomIndex = 0;
			m_instanceShaderBindingTableRecordOffset = 0;
			//

		}
		//
	};



	class PhysicsRT
	{
	public:
		//void OnCreate(Device* pDevice, VkPipeline pipeline, ResourceViewHeaps* pResourceViewHeaps, StaticBufferPool* pStaticBufferPool, DynamicBufferRing* pDynamicBufferRing);
		void OnCreate(Device* pDevice, uint32_t numParticle
			, HeapDescriptorSetAllocater* pDescriptorSetHeap
			, DynamicBufferAllocater* m_pConstBufferDynamic_CPU2GPU
			, DynamicBufferAllocater* pRtBufferDynamic_GPU, StaticBufferAllocater* pRtBufferStatic_GPU);
		void OnDestroy();
		//


		VkDescriptorSetLayout GetAabbDescriptorSetLayout() { return m_particleAabbBuffer.GetDescriptorSetLayout(); }
		VkDescriptorSet GetAabbDescriptorSet() { return m_particleAabbBuffer.GetDescriptorSet(); }
		VkDescriptorBufferInfo GetAabbDescriptorBufferInfo() { return m_particleAabbBuffer.GetDescriptorBufferInfo(); }
		VkDescriptorSetLayout GetTlasDescriptorSetLayout() { return m_particleTopLevel_AsKHR.GetDescriptorSetLayout(); }
		VkDescriptorSet GetTlasDescriptorSet() { return m_particleTopLevel_AsKHR.GetDescriptorSet(); }
		VkDescriptorBufferInfo GetTlasDescriptorBufferInfo() { return m_particleTopLevel_AsKHR.GetDescriptorBufferInfo(); }

		//AlloceBuffer: OnCreateAabbBuffer(),OnCreateInstanceBuffer(),CreateAccelerationStructure(){As+Scratch}
		//
		void OnCreateAabbBuffer(uint32_t numParticle, ParticleAabbBuffer& particleAabbBuffer);
		void OnCreateInstanceBuffer(uint32_t numInstance, ParticleInstanceBuffer& ParticleInstance
			, uint32_t instanceCustomIndex, uint32_t instanceShaderBindingTableRecordOffset);
		//

		//
		//void CreateFullLevelAccelerationStructure(VkCommandBuffer cmdBuffer, bool bUpdateAS = false);
		void BuildFullLevelAccelerationStructure(VkCommandBuffer cmdBuffer);
		void BuildBottomLevelAccelerationStructure(VkCommandBuffer cmdBuffer, ParticleAccelerationStructure& particle_blas
				, ParticleAabbBuffer& particle_AabbBuffer, bool bUpdateAs, bool bDynamicAsSize);
		void BuildTopLevelAccelerationStructure(VkCommandBuffer cmdBuffer, ParticleAccelerationStructure& particle_tlas, ParticleAccelerationStructure& particle_blas
				, ParticleInstanceBuffer& particle_instanceBuffer, bool bUpdateAs, bool bDynamicAsSize);
		//void CreateTopLevelAccelerationStructure(VkCommandBuffer cmdBuffer, ParticleAccelerationStructure& particle_tlas, ParticleAccelerationStructure& particle_blas
		//		, ParticleInstance& particle_instanceBuffer, uint32_t instanceCustomIndex,uint32_t instanceShaderBindingTableRecordOffset, bool bUpdateAS);
		void BuildAccelerationStructure(VkCommandBuffer cmdBuffer
			, std::vector<VkAccelerationStructureGeometryKHR>& array_AsGeomKHR, std::vector<uint32_t>& array_AsGeomMaxPrimitiveCounts
			, ParticleAccelerationStructure& particleAs		//, ParticleAccelerationStructure* pSrc_particleAs
			, ParticleConfiguration* pParticleConfiguration
			, bool bUpdateAs,bool bBLAS, bool bDynamicAsSize);
		void CreateAccelerationStructure(ParticleAccelerationStructure& particleAs
			, VkAccelerationStructureBuildSizesInfoKHR& as_buildSizeInfoKHR
			, bool bBLAS, bool bDynamicAsSize);
		//void CreateScratchBuffer();
		//
	private:
		//
		Device*							m_pDevice;
		//ResourceViewHeaps*				m_pResourceViewHeaps;
		//StaticBufferPool*				m_pStaticBufferPool;
		//DynamicBufferRing*			m_pDynamicBufferRing;
		HeapDescriptorSetAllocater*		m_pDescriptorSetHeap;
		DynamicBufferAllocater*			m_pConstBufferDynamic_CPU2GPU;
		DynamicBufferAllocater*         m_pRtBufferDynamic_GPU;
		StaticBufferAllocater*			m_pRtBufferStatic_GPU;

		// VkPipeline_T*{Handle}
		//VkPipeline						m_computePipeline = VK_NULL_HANDLE;
		//
		
		// Function pointers for ray tracing related stuff
		PFN_vkGetBufferDeviceAddressKHR					vkGetBufferDeviceAddressKHR;
		PFN_vkCreateAccelerationStructureKHR			vkCreateAccelerationStructureKHR;
		PFN_vkDestroyAccelerationStructureKHR			vkDestroyAccelerationStructureKHR;
		PFN_vkGetAccelerationStructureBuildSizesKHR		vkGetAccelerationStructureBuildSizesKHR;
		PFN_vkGetAccelerationStructureDeviceAddressKHR	vkGetAccelerationStructureDeviceAddressKHR;
		PFN_vkBuildAccelerationStructuresKHR			vkBuildAccelerationStructuresKHR;
		PFN_vkCmdBuildAccelerationStructuresKHR			vkCmdBuildAccelerationStructuresKHR;
		PFN_vkCmdTraceRaysKHR							vkCmdTraceRaysKHR;
		PFN_vkGetRayTracingShaderGroupHandlesKHR		vkGetRayTracingShaderGroupHandlesKHR;
		PFN_vkCreateRayTracingPipelinesKHR				vkCreateRayTracingPipelinesKHR;
		//
		
		//
		//ParticlePosition|Aabb-Buffer, Instance-Buffer
		//
		uint32_t						m_numParticle;
		uint32_t						m_deltaSizeAs = 256 * 4;	//1024
		ParticleAabbBuffer				m_particleAabbBuffer;
		ParticleInstanceBuffer			m_particleInstanceBuffer;
		//Tlas: TopLevelAcceleration1Structure;	Blas: BottomLevelAcceleration1Structure
		ParticleAccelerationStructure	m_particleBottomLevel_AsKHR;
		ParticleAccelerationStructure	m_particleTopLevel_AsKHR;
		bool							m_bUpdateBlas = false;
		bool							m_bUpdateTlas = false;
		bool							m_bDynamicBlasSize = false;
		bool							m_bDynamicTlasSize = false;
		uint32_t						m_NumCreatedInstanceCustomIndex = 0;
		uint32_t						m_NumCreatedInstanceShaderBindingTableRecordOffset = 0;
		//
	};
}
