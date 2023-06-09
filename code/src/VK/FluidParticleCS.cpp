
#include "stdafx.h"
#include "Base/DynamicBufferRing.h"
#include "Base/StaticBufferPool.h"
#include "Base/ExtDebugUtils.h"
#include "Base/UploadHeap.h"
#include "Base/Texture.h"
#include "Base/Helper.h"
//#include "TAA.h"
//#include "PhysicsRT.h"
#include "FluidParticleCS.h"


namespace CAULDRON_VK
{
	void FluidParticleCS::OnCreate(Device* pDevice, uint32_t numParticle, std::array<uint32_t,3> numDimensions
		, VkRenderPass renderPass
		//, ResourceViewHeaps* pResourceViewHeaps
		, HeapDescriptorSetAllocater* pDescriptorSetHeap
		//, StaticBufferPool* pStaticBufferPool, DynamicBufferRing* pDynamicBufferRing
		, DynamicBufferAllocater* pConstBufferDynamic_CPU2GPU, DynamicBufferAllocater* pRtBufferDynamic_GPU
		, StaticBufferAllocater* pStorageBufferStatic_GPU, StaticBufferAllocater* pRtBufferStatic_GPU
	)
	{
		VkResult res;
		//
		m_pDevice = pDevice;
		//m_pResourceViewHeaps = pResourceViewHeaps;
		//m_pStaticBufferPool = pStaticBufferPool;
		//m_pDynamicBufferRing = pDynamicBufferRing;
		//
		m_pDescriptorSetHeap = pDescriptorSetHeap;
		m_pConstBufferDynamic_CPU2GPU = pConstBufferDynamic_CPU2GPU;
		m_pRtBufferDynamic_GPU = pRtBufferDynamic_GPU;
		m_pStorageBufferStatic_GPU = pStorageBufferStatic_GPU;
		m_pRtBufferStatic_GPU = pRtBufferStatic_GPU;
		//
		
		// 
		//m_pResourceViewHeaps->CreateDescriptorSetLayout()
		//m_pResourceViewHeaps->AllocDescriptor()
		//m_pResourceViewHeaps->CreateDescriptorSetLayout()
		//m_pStaticBufferPool->AllocBuffer()
		//m_particleCS.OnCreate()
		//
		
		//
		m_numPartilce = numParticle;
		m_numDimensions = numDimensions;
		
		// Create VS+FS
		// Create VS+FS	 -uniform
		//TODO:
		{
			VkDescriptorSetLayoutBinding	descriptorSetLayoutBinding;
			descriptorSetLayoutBinding.binding = 0;
			descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorSetLayoutBinding.descriptorCount = 1;
			descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			descriptorSetLayoutBinding.pImmutableSamplers = NULL;

			std::vector<VkDescriptorSetLayoutBinding>	array_descriptorSetLayoutBinding{ descriptorSetLayoutBinding };
			//
			OnCreateUniformDescriptorSet(sizeof(RenderParticlePSUniform)
				, &m_uniformDescriptorSetLayout_particlePS, &m_uniformDescriptorSet_particlePS
				, array_descriptorSetLayoutBinding);
			//VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC need DynamicBufferAllocater::SetDescriptro() after DynamicBufferAllocater::AllocateBuffer()
		}
		// Create VS+FS	 -pipeline
		//
		{
			ShaderCompileParameter	vS_ParticlePsCompileParameter{ "ParticlePS_VS.glsl","main","" };
			ShaderCompileParameter	fS_ParticlePsCompileParameter{ "ParticleRender_FS.glsl","main","" };
			//
			//https://zhuanlan.zhihu.com/p/450157594
			VkVertexInputBindingDescription		vertexInput_bindingDescription;
			// binding|connect-Point
			vertexInput_bindingDescription.binding = 0;
			//	16
			vertexInput_bindingDescription.stride = sizeof(ParticlePropertyComponentAlign);
			//	VK_VERTEX_INPUT_RATE_VERTEX,	VK_VERTEX_INPUT_RATE_INSTANCE 
			vertexInput_bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			//
			VkVertexInputAttributeDescription	vertexInput_attributeDescription;
			vertexInput_attributeDescription.location = 0;
			vertexInput_attributeDescription.binding = 0;
			//	offsetof(ParticleProperty, position)
			//byte-Offset of this-Attribute relative-to the Start of an Element in the vertex input binding
			vertexInput_attributeDescription.offset = 0;
			//	Be consisted to m_GBuffer.m_flag[GBUFFER_FORWARD]==VK_FORMAT_R16G16B16A16_SFLOAT
			//VK_FORMAT_R16G16B16A16_SFLOAT;
			vertexInput_attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			//
			ParticleVertexInputStateParameter particleVertexInputStateParameter =
			{
					std::vector<VkVertexInputBindingDescription>(1,vertexInput_bindingDescription)
				,	std::vector<VkVertexInputAttributeDescription>(1,vertexInput_attributeDescription)
			};
			//
			std::vector<VkDescriptorSetLayout>	array_descriptorSetLayout = 
			{
				//Deprecate: Be bound by vkCmdBindVertexBuffer()
				//	m_particleData.GetPositionDescriptorSetLayout()
				//TODO: Uniform_DescriptorSetLayout for VS+FS
				//
			};
			if (m_uniformDescriptorSetLayout_particlePS != VK_NULL_HANDLE)
			{
				array_descriptorSetLayout.push_back(m_uniformDescriptorSetLayout_particlePS);
			}
			//

			m_renderParticlePS.OnCreate(m_pDevice, renderPass, { 50,50,50 }
				, vS_ParticlePsCompileParameter, fS_ParticlePsCompileParameter
				, &particleVertexInputStateParameter
				, pStorageBufferStatic_GPU, pConstBufferDynamic_CPU2GPU
				, array_descriptorSetLayout
				, NULL, VK_SAMPLE_COUNT_1_BIT);
			//

		}
		
		// Create RT's AccelerationStructure for Physics|Particle-Collision in ParticelCS
		//No RTpipeline
		m_physicsRT.OnCreate(pDevice, m_numPartilce, m_pDescriptorSetHeap, m_pConstBufferDynamic_CPU2GPU, m_pRtBufferDynamic_GPU, m_pRtBufferStatic_GPU);
		
		//
		OnCreateParticleBuffer(m_particleData, m_numPartilce, sizeof(ParticleProperty) / sizeof(ParticlePropertyComponentAlign));

		//Load Shader{ComputeShader} => Create VkPipelineLayout,VkPipeline
		//
		// Create m_firstParticleCS+m_mainParticleParticleCS	 -uniform
		//TODO:
		{
			VkDescriptorSetLayoutBinding	descriptorSetLayoutBinding;
			descriptorSetLayoutBinding.binding = 0;
			descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorSetLayoutBinding.descriptorCount = 1;
			descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			descriptorSetLayoutBinding.pImmutableSamplers = NULL;

			std::vector<VkDescriptorSetLayoutBinding>	array_descriptorSetLayoutBinding{ descriptorSetLayoutBinding };
			//
			OnCreateUniformDescriptorSet(sizeof(RenderParticlePSUniform)
				, &m_uniformDescriptorSetLayout_particleCS, &m_uniformDescriptorSet_particleCS
				, array_descriptorSetLayoutBinding);
			//VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC need DynamicBufferAllocater::SetDescriptro() after DynamicBufferAllocater::AllocateBuffer()
		}

		//
		//m_mainParticleCS
		{
			std::vector<VkDescriptorSetLayout>	array_descriptorSetLayout =
			{
				 m_particleData.GetDescriptorSetLayout()
				,m_physicsRT.GetTlasDescriptorSetLayout()
				,m_physicsRT.GetAabbDescriptorSetLayout()
			};
			if (m_uniformDescriptorSetLayout_particleCS != VK_NULL_HANDLE)
			{
				//TODO: Uniform_DescriptorSetLayout: Dynamic
				array_descriptorSetLayout.push_back(m_uniformDescriptorSetLayout_particleCS);
			}
			//
			DefineList defineList;
			defineList["DELTA_T"] = std::to_string(0.00075);	// 0.0005
			defineList["DELTA_X"] = std::to_string(0.01);
			defineList["HALF_BOUNDBOX"] = std::to_string(0.005);	//0.005
			//
			defineList["H"] = std::to_string(0.02);	// 0.01-0.0000001
			defineList["RAY_H"] = std::to_string(0.02);	// 0.01-0.0000001
			defineList["SQRT_3"] = std::to_string(std::sqrt(3.0));
			defineList["REST_DENSITY"] = std::to_string(1000.0);
			defineList["INV_REST_DENSITY"] = std::to_string(0.001);
			defineList["MASS"] = std::to_string(0.01 * 0.01 * 0.01 * 1000.0);
			defineList["GRAVITY_ACCELERATION_Y"] = std::to_string(-10.0);	//-9.8
			defineList["PRESSURE_COEFFICIENT"] = std::to_string(1.0);		//1.0,50.0
			defineList["VISCOSITY_COEFFICIENT"] = std::to_string(0.005);	//1.0

			m_mainParticleCS.OnCreate(m_pDevice, "WSPH_CS.glsl", "main", "", array_descriptorSetLayout
				, m_numDimensions[0], m_numDimensions[1], m_numDimensions[2], &defineList);
		}
		// 
		//m_firstParticleCS
		{
			m_bFirst = true;

			std::vector<VkDescriptorSetLayout>	array_descriptorSetLayout =
			{
				 m_particleData.GetPositionDescriptorSetLayout()
				,m_physicsRT.GetAabbDescriptorSetLayout()
			};
			if (m_uniformDescriptorSetLayout_particleCS != VK_NULL_HANDLE)
			{
				//TODO:	Uniform_DescriptorSetLayout: Dynamic
				array_descriptorSetLayout.push_back(m_uniformDescriptorSetLayout_particleCS);
			}
			//
			DefineList defineList;
			defineList["STARTX"] = std::to_string(0.255);		//0.25
			defineList["STARTY"] = std::to_string(0.5);		//0.01
			defineList["STARTZ"] = std::to_string(0.255);		//0.25
			//defineList["DELTA_T"] = std::to_string(0.0005);
			defineList["DELTA_X"] = std::to_string(0.01);	//0.01
			defineList["HALF_BOUNDBOX"] = std::to_string(0.005);	//0.005
			//
			//defineList["H"] = std::to_string(0.01-0.0000001);
			defineList["REST_DENSITY"] = std::to_string(1000.0);	//0.01

			m_firstParticleCS.OnCreate(m_pDevice, "ParticleInit_CS.glsl", "main", "", array_descriptorSetLayout
				, m_numDimensions[0], m_numDimensions[1], m_numDimensions[2], &defineList);
		}


		//VkSemaphore
		{
			//Graphics
			VkSemaphoreCreateInfo	semaphoreCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			res = vkCreateSemaphore(m_pDevice->GetDevice(), &semaphoreCreateInfo, NULL, &m_graphicsSemaphore);
			assert(res == VK_SUCCESS);
			//Compute
			res = vkCreateSemaphore(m_pDevice->GetDevice(), &semaphoreCreateInfo, NULL, &m_computeSemaphore);
			assert(res == VK_SUCCESS);
			//
			/*
			VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &m_graphicsSemaphore;
			
			res = vkQueueSubmit(m_pDevice->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			assert(res == VK_SUCCESS);
			res = vkQueueWaitIdle(m_pDevice->GetGraphicsQueue());
			assert(res == VK_SUCCESS);
			*/
			//
		}

		//m_currentWolrdMat4,m_previousWolrdMat4
		//
		{
			/*
			m_currentWolrdMat4 = math::Matrix4( math::Vector4(+1.0, 0.0, 0.0, 0.0)
											,	math::Vector4(0.0, 0.0, -1.0, 0.0)
											,	math::Vector4(0.0, +1.0, 0.0, 0.0)
											,	math::Vector4(0.0, 0.0, 0.0, +1.0));
			*/
			m_currentWolrdMat4 = math::Matrix4(	math::Vector4(+1.0, 0.0, 0.0, 0.0)
											,	math::Vector4(0.0, +1.0, 0.0, 0.0)
											,	math::Vector4(0.0, 0.0, +1.0, 0.0)
											,	math::Vector4(0.0, +5.0, +2.0, +1.0));
			m_previousWolrdMat4 = m_currentWolrdMat4;
		}

		//
		//TODO
		//SetResourceName(m_pDevice->GetDevice(),VK_OBJECT_TYPE_BUFFER,)
		//
	}
	

	void FluidParticleCS::OnCreateUniformDescriptorSet(uint32_t sizeByte,VkDescriptorSetLayout* pDescriptorSetLayout, VkDescriptorSet* pDescriptorSet
		, std::vector<VkDescriptorSetLayoutBinding>& array_descriptorSetLayoutBinding)
	{
		m_pDescriptorSetHeap->CreateDescriptorSetLayoutAndAllocDescriptorSet(&array_descriptorSetLayoutBinding
			, pDescriptorSetLayout, pDescriptorSet);
		//
		// If std140|std430, Please alter size to Satisfy Align
		//m_pConstBufferDynamic_CPU2GPU->SetDescriptorSet(0, sizeof(RenderParticlePSUniform), *pDescriptorSet);
		m_pConstBufferDynamic_CPU2GPU->SetDescriptorSet(0, sizeByte, *pDescriptorSet);
		//

		//SetResourceName(m_pDevice->GetDevice(),VK_OBJECT_TYPE_BUFFER,)
	}

	void FluidParticleCS::OnCreateParticleBuffer(ParticleData& particleData, uint32_t numParticle, uint32_t numProperty)
	{
		VkResult res;
		//
		particleData.OnInit(numParticle, numProperty);
		VkDescriptorBufferInfo	descriptorBufferInfo;
		{
			//	Position
			//Finished: AllocBuffer()
			//
			// All Property
			for (uint32_t i = 0; i < numProperty; ++i)
			{
				//Finished: AllocBuffer()
				m_pStorageBufferStatic_GPU->AllocBuffer(numParticle, sizeof(ParticlePropertyComponentAlign), &descriptorBufferInfo);
				//m_pStorageBufferStatic_GPU->AllocBuffer(numParticle, sizeof(ParticlePropertyComponentAlign)
				//	, (void**)&m_particleVlcPointer, NULL
				//	, &descriptorBufferInfo);
				particleData.SetPropertyDescriptorBufferInfo(i, descriptorBufferInfo);
			}
			//
		}
		//
		


		//For m_mainParticleCS
		//	Create DescriptorSetLayout+DescriptorSet
		{

			//
			VkDescriptorSetLayout			dscriptorLayout;
			VkDescriptorSet					descriptorSet;
			{
				VkDescriptorSetLayoutBinding	descriptorSetLayoutBinding{};
				//Create DescriptorSetLayout
				//descriptorSetLayoutBinding.binding = 0;	//Delta
				descriptorSetLayoutBinding.descriptorCount = 1;
				descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
				//descriptorSetLayoutBinding.pImmutableSamplers = NULL;
				//
				std::vector<VkDescriptorSetLayoutBinding> array_descriptorSetLayoutBinding(numProperty, descriptorSetLayoutBinding);
				for (uint32_t i = 0; i < numProperty; ++i)
				{
					array_descriptorSetLayoutBinding[i].binding = i;
				}
				//
				//Create DescriptorSetLayout+DescriptorSet
				m_pDescriptorSetHeap->CreateDescriptorSetLayoutAndAllocDescriptorSet(&array_descriptorSetLayoutBinding
					, &dscriptorLayout, &descriptorSet);
				//
				m_particleData.SetDescriptorSetLayout(dscriptorLayout);
				m_particleData.SetDescriptorSet(descriptorSet);
				//

			}
			//
			//

			//Update|Write DescriptorSet
			//	Bind Buffer 2 DescriptorSet
			//For m_mainParticleCS
			VkWriteDescriptorSet writeDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			{
				writeDescriptorSet.dstSet = m_particleData.GetDescriptorSet();
				//writeDescriptorSet.dstBinding = 0;	//Delta
				//Starting element in that array: 
				// If the descriptor binding identified by dstSet and dstBinding has a descriptor type of VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK 
				//	then dstArrayElement specifies the Starting-byte-Offset within the binding.
				//	1: if 0-start-offset for Special-process|usage
				writeDescriptorSet.dstArrayElement = 0;
				//
				writeDescriptorSet.descriptorCount = 1;
				writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				//
				//writeDescriptorSet.pTexelBufferView = NULL;
				//writeDescriptorSet.pBufferInfo=	//Delta
				//
				std::vector<VkWriteDescriptorSet> array_writeDescriptorSet(numProperty, writeDescriptorSet);
				std::vector<VkDescriptorBufferInfo> array_propertyDescriptorBufferInfo = m_particleData.GetPropertyArrayDescriptorBufferInfo();
				auto pArray_propertyDescriptorBufferInfo = array_propertyDescriptorBufferInfo.data();
				for (uint32_t i = 0; i < numProperty; ++i)
				{
					array_writeDescriptorSet[i].dstBinding = i;
					//array_writeDescriptorSet[i].pBufferInfo = &(array_propertyDescriptorBufferInfo[i]);
					array_writeDescriptorSet[i].pBufferInfo = pArray_propertyDescriptorBufferInfo + i;
					//
				}
				//
				vkUpdateDescriptorSets(m_pDevice->GetDevice(), numProperty, array_writeDescriptorSet.data(), 0, NULL);
				//
			}
			//

		}
		//

		//For m_initParticleCS
		//	Will be bound as VertexBuffer for Graphics_Pipeline
		//
		{

			//
			VkDescriptorSetLayout			dscriptorLayout;
			VkDescriptorSet					descriptorSet;
			{
				VkDescriptorSetLayoutBinding	descriptorSetLayoutBinding{};
				//Create DescriptorSetLayout
				descriptorSetLayoutBinding.binding = 0;	//Delta
				descriptorSetLayoutBinding.descriptorCount = 1;
				descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				//descriptorSetLayoutBinding.pImmutableSamplers=
				//Specifying which pipeline shader stages can access a resource for this binding.
				//	VK_SHADER_STAGE_ALL is a shorthand specifying that all defined shader stages, including any additional stages defined by extensions, can access the resource.
				//	If a shader stage is not included in stageFlags, then a resource must not be accessed from that stage via this binding within any pipeline using the set layout. 
				//	Other than *input attachments which are limited to the fragment shader*, there are no limitations on what combinations of stages can use a descriptor binding
				//	, and in **particular a binding can be used by both graphics stages and the compute stage**
				descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;	// | VK_SHADER_STAGE_VERTEX_BIT
				//
				std::vector<VkDescriptorSetLayoutBinding> array_descriptorSetLayoutBinding(1, descriptorSetLayoutBinding);
				//
				//Create DescriptorSetLayout+DescriptorSet
				m_pDescriptorSetHeap->CreateDescriptorSetLayoutAndAllocDescriptorSet(&array_descriptorSetLayoutBinding
					, &dscriptorLayout, &descriptorSet);
				//
				m_particleData.SetPositionDescriptorSetLayout(dscriptorLayout);
				m_particleData.SePositiontDescriptorSet(descriptorSet);
				//

			}
			//

			//Update|Write DescriptorSet
			//	Bind Buffer 2 DescriptorSet
			//For m_mainParticleCS
			VkWriteDescriptorSet writeDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			{
				VkDescriptorBufferInfo  descriptorBufferInfo = m_particleData.GetPositionDescriptorBufferInfo();

				writeDescriptorSet.dstSet = m_particleData.GetPositionDescriptorSet();
				writeDescriptorSet.dstBinding = 0;	//Delta
				//Starting element in that array: 
				// If the descriptor binding identified by dstSet and dstBinding has a descriptor type of VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK 
				//	then dstArrayElement specifies the Starting-byte-Offset within the binding.
				//	1: if 0-start-offset for Special-process|usage
				writeDescriptorSet.dstArrayElement = 0;
				//
				writeDescriptorSet.descriptorCount = 1;
				writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				//
				writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;	//Delta
				//
				std::vector<VkWriteDescriptorSet> array_writeDescriptorSet(1, writeDescriptorSet);
				//
				vkUpdateDescriptorSets(m_pDevice->GetDevice(), 1, array_writeDescriptorSet.data(), 0, NULL);
				//
			}
			//

		}
		//

		//SetResourceName(m_pDevice->GetDevice(), VK_OBJECT_TYPE_BUFFER, )
	}

	void FluidParticleCS::OnDestroy()
	{
		
		//
		m_numPartilce = 0;
		m_numDimensions = { 0,0,0 };
		//
		std::vector<VkDescriptorSet> array_desciptorSet;
		//m_particleData
		m_particleData.OnDestroy(m_pDevice->GetDevice(), &array_desciptorSet);
		//
		m_renderParticlePS.OnDestroy();
		m_mainParticleCS.OnDestroy();
		m_firstParticleCS.OnDestroy();
		m_physicsRT.OnDestroy();
		//
		vkDestroyDescriptorSetLayout(m_pDevice->GetDevice(), m_uniformDescriptorSetLayout_particlePS, NULL);
		if (m_uniformDescriptorSet_particlePS != VK_NULL_HANDLE)
		{
			m_pDescriptorSetHeap->FreeDescriptor(m_uniformDescriptorSet_particlePS);
			m_uniformDescriptorSet_particlePS = VK_NULL_HANDLE;
		}

		vkDestroyDescriptorSetLayout(m_pDevice->GetDevice(), m_uniformDescriptorSetLayout_particleCS, NULL);
		if (m_uniformDescriptorSet_particleCS != VK_NULL_HANDLE)
		{
			m_pDescriptorSetHeap->FreeDescriptor(m_uniformDescriptorSet_particleCS);
			m_uniformDescriptorSet_particleCS = VK_NULL_HANDLE;
		}

		//
		vkDestroySemaphore(m_pDevice->GetDevice(), m_graphicsSemaphore, NULL);
		m_graphicsSemaphore = VK_NULL_HANDLE;
		vkDestroySemaphore(m_pDevice->GetDevice(), m_computeSemaphore, NULL);
		m_computeSemaphore = VK_NULL_HANDLE;
		//
	}

	void FluidParticleCS::DrawGraphics(VkCommandBuffer cmd_buf, const Camera& Cam)
	{

		VkResult res;
		//
		
		//Graphics 
		//
		{
			SetPerfMarkerBegin(cmd_buf, "RenderParticle");
			//
			RenderParticlePSUniform*	pRenderParticlePSUniform;
			VkDescriptorBufferInfo		renderParticlePSUniform_descriptorBufferInfo;
			m_pConstBufferDynamic_CPU2GPU->AllocBuffer(sizeof(RenderParticlePSUniform), (void**)&pRenderParticlePSUniform, NULL
				, &renderParticlePSUniform_descriptorBufferInfo);
			pRenderParticlePSUniform->model = m_currentWolrdMat4;
			pRenderParticlePSUniform->viewProjection = Cam.GetProjection()* Cam.GetView();
			// be Called at OnCreate()
			//m_pConstBufferDynamic_CPU2GPU->SetDescriptorSet(0, renderParticlePSUniform_descriptorBufferInfo.range, m_uniformDescriptorSet_particlePS);
			//
			VkDescriptorBufferInfo	particlePosition_descriptorBufferInfo = m_particleData.GetPositionDescriptorBufferInfo();
			//
			m_renderParticlePS.Draw(cmd_buf, &renderParticlePSUniform_descriptorBufferInfo, m_uniformDescriptorSet_particlePS
				, &particlePosition_descriptorBufferInfo, m_numPartilce);
			//
			
			//VkBufferMemoryBarrier2
			VkBufferMemoryBarrier	bufferMemory_barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
			{
				//VkDescriptorBufferInfo particlePositionDescriptorBufferInfo = m_particleData.GetPositionDescriptorBufferInfo();
				VkDescriptorBufferInfo descriptorBufferInfo = m_particleData.GetPositionDescriptorBufferInfo();

				bufferMemory_barrier.srcAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
				// | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_SHADER_WRITE_BIT
				bufferMemory_barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
				bufferMemory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferMemory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferMemory_barrier.buffer = descriptorBufferInfo.buffer;
				bufferMemory_barrier.offset = descriptorBufferInfo.offset;
				bufferMemory_barrier.size = descriptorBufferInfo.range;

				//vkCmdPipelineBarrier2()
				vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
					, 0
					, 0, NULL, 1, &bufferMemory_barrier, 0, NULL);
				//
			}
			//

			//
			SetPerfMarkerEnd(cmd_buf);
		}


	}

	void FluidParticleCS::DrawCompute(VkCommandBuffer cmd_buf, const Camera& Cam)
	{
		VkResult res;
		//

		// First
		if (m_bFirst)
		{
			SetPerfMarkerBegin(cmd_buf, "InitParticle");

			std::vector<VkDescriptorSet>	array_storageDescriptorSet =
			{
				 m_particleData.GetPositionDescriptorSet()
				,m_physicsRT.GetAabbDescriptorSet()
			};
			//
			VkDescriptorBufferInfo		renderParticlePSUniform_descriptorBufferInfo;
			if (m_uniformDescriptorSet_particleCS != VK_NULL_HANDLE)
			{
				//array_storageDescriptorSet.push_back(m_uniformDescriptorSet_particleCS);
				//
				RenderParticlePSUniform*	pRenderParticlePSUniform;
				m_pConstBufferDynamic_CPU2GPU->AllocBuffer(sizeof(RenderParticlePSUniform), (void**)&pRenderParticlePSUniform, NULL
					, &renderParticlePSUniform_descriptorBufferInfo);
				pRenderParticlePSUniform->model = m_currentWolrdMat4;
				pRenderParticlePSUniform->viewProjection = Cam.GetProjection()* Cam.GetView();
			}
			//
			//m_firstParticleCS.Draw(cmd_buf, NULL, NULL, m_numPartilce, 1, 1);
			m_firstParticleCS.Draw(cmd_buf, array_storageDescriptorSet
				, &renderParticlePSUniform_descriptorBufferInfo, m_uniformDescriptorSet_particleCS
				, m_numPartilce, 1, 1);
			//	, NULL, VK_NULL_HANDLE, m_numPartilce, 1, 1);
			//

			//VkBufferMemoryBarrier2
			VkBufferMemoryBarrier	bufferMemory_barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
			{
				//VkDescriptorBufferInfo descriptorBufferInfo = m_particleData.GetPositionDescriptorBufferInfo();
				VkDescriptorBufferInfo descriptorBufferInfo = m_physicsRT.GetAabbDescriptorBufferInfo();

				bufferMemory_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				// | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR
				//bufferMemory_barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
				bufferMemory_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
				bufferMemory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferMemory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferMemory_barrier.buffer = descriptorBufferInfo.buffer;
				bufferMemory_barrier.offset = descriptorBufferInfo.offset;
				bufferMemory_barrier.size = descriptorBufferInfo.range;

				//vkCmdPipelineBarrier2()
				vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR
					, 0
					, 0, NULL, 1, &bufferMemory_barrier, 0, NULL);
				//
			}
			//

			m_bFirst = false;

			SetPerfMarkerEnd(cmd_buf);
		}


		//BuildAccelerationStructure
		{
			SetPerfMarkerBegin(cmd_buf, "BuildAccelerationStructure");

			m_physicsRT.BuildFullLevelAccelerationStructure(cmd_buf);
			//
			
			//VkBufferMemoryBarrier2
			VkBufferMemoryBarrier	bufferMemory_barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
			{
				//VkDescriptorBufferInfo tlasDescriptorBufferInfo = m_physicsRT.GetTlasDescriptorBufferInfo();
				VkDescriptorBufferInfo descriptorBufferInfo = m_physicsRT.GetTlasDescriptorBufferInfo();

				// | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR 
				bufferMemory_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
				// | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR
				bufferMemory_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
				bufferMemory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferMemory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferMemory_barrier.buffer = descriptorBufferInfo.buffer;
				bufferMemory_barrier.offset = descriptorBufferInfo.offset;
				bufferMemory_barrier.size = descriptorBufferInfo.range;

				//vkCmdPipelineBarrier2()
				vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
					, 0, 0, NULL, 1, &bufferMemory_barrier, 0, NULL);
				//
			}

			SetPerfMarkerEnd(cmd_buf);
		}

		//Compute 
		{
			SetPerfMarkerBegin(cmd_buf, "ComputeParticle");



			std::vector<VkDescriptorSet>	array_storageDescriptorSet =
			{
				 m_particleData.GetDescriptorSet()
				,m_physicsRT.GetTlasDescriptorSet()
				,m_physicsRT.GetAabbDescriptorSet()
			};
			//
			VkDescriptorBufferInfo		renderParticlePSUniform_descriptorBufferInfo;
			if (m_uniformDescriptorSet_particleCS != VK_NULL_HANDLE)
			{
				//array_storageDescriptorSet.push_back(m_uniformDescriptorSet_particleCS);
				//
				RenderParticlePSUniform*	pRenderParticlePSUniform;
				m_pConstBufferDynamic_CPU2GPU->AllocBuffer(sizeof(RenderParticlePSUniform), (void**)&pRenderParticlePSUniform, NULL
					, &renderParticlePSUniform_descriptorBufferInfo);
				pRenderParticlePSUniform->model = m_currentWolrdMat4;
				pRenderParticlePSUniform->viewProjection = Cam.GetProjection() * Cam.GetView();
			}
			//
			//m_mainParticleCS.Draw(cmd_buf, NULL, NULL, m_numPartilce, 1, 1);
			m_mainParticleCS.Draw(cmd_buf, array_storageDescriptorSet
				, &renderParticlePSUniform_descriptorBufferInfo, m_uniformDescriptorSet_particleCS
				//, NULL, VK_NULL_HANDLE
				, m_numPartilce, 1, 1);
			//

			//VkBufferMemoryBarrier2
			VkBufferMemoryBarrier	bufferMemory_barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
			{
				//VkDescriptorBufferInfo descriptorBufferInfo = m_physicsRT.GetAabbDescriptorBufferInfo();
				VkDescriptorBufferInfo descriptorBufferInfo = m_particleData.GetPositionDescriptorBufferInfo();

				bufferMemory_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				//bufferMemory_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
				bufferMemory_barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
				bufferMemory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferMemory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				bufferMemory_barrier.buffer = descriptorBufferInfo.buffer;
				bufferMemory_barrier.offset = descriptorBufferInfo.offset;
				bufferMemory_barrier.size = descriptorBufferInfo.range;

				//vkCmdPipelineBarrier2()
				//vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
				vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR
					, 0, 0, NULL, 1, &bufferMemory_barrier, 0, NULL);
				//
			}


			SetPerfMarkerEnd(cmd_buf);
		}


	}

}

