#include "RTPipeline.h"
#include "VulkanShader.h"
#include "VulkanSet.h"

RTPipeline::RTPipeline(VulkanContextRef ctx, RTShaderBuilderRef shader):
	_ctx(ctx),
	_shader(shader)
{
	uint32_t numShaders = static_cast<uint32_t>(shader->getStages().size());
	
	auto &uniLayouts = shader->getDescriptorSetLayouts();

	_pipelineLayout = _ctx->getDevice().createPipelineLayout(
		vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			static_cast<uint32_t>(uniLayouts.size()),
			uniLayouts.data(),
			0,
			nullptr //Push constant Ranges
		),
		nullptr,
		_ctx->getDynamicDispatch()
	);

	auto createInfo = vk::RayTracingPipelineCreateInfoNV(
		vk::PipelineCreateFlags(),
		shader->getStages().size(),
		shader->getStages().data(),
		shader->getGroups().size(),
		shader->getGroups().data(),
		1,
		_pipelineLayout,
		nullptr,
		0
	);

	_pipeline = ctx->getDevice().createRayTracingPipelineNV(
		nullptr,
		createInfo,
		nullptr,
		_ctx->getDynamicDispatch()
	);

	vk::PhysicalDeviceProperties2 devProps;
	devProps.pNext = &_RTProps;
	devProps.properties = { };
	_ctx->getPhysicalDevice().getProperties2(&devProps, _ctx->getDynamicDispatch());

	uint64_t size = _RTProps.shaderGroupHandleSize * shader->getGroups().size();

	_sbtBuffer = _ctx->makeBuffer(vk::BufferUsageFlagBits::eRayTracingNV | vk::BufferUsageFlagBits::eTransferSrc, size, vk::MemoryPropertyFlagBits::eHostVisible);

	auto ptr = _sbtBuffer->getMapped();

	vk::Result res = _ctx->getDevice().getRayTracingShaderGroupHandlesNV(_pipeline, 0, shader->getGroups().size(), size, ptr, _ctx->getDynamicDispatch());

	uint16_t* up = (uint16_t*)ptr;

	std::cout << *up << std::endl;

	_sbtBuffer->unmap();

}

RTPipeline::~RTPipeline()
{
	_ctx->getDevice().destroyPipelineLayout(_pipelineLayout, nullptr, _ctx->getDynamicDispatch());
	_ctx->getDevice().destroyPipeline(_pipeline, nullptr, _ctx->getDynamicDispatch());
}

void RTPipeline::bind(vk::CommandBuffer * cmd)
{
	cmd->bindPipeline(
		vk::PipelineBindPoint::eRayTracingNV,
		getPipeline(),
		_ctx->getDynamicDispatch()
	);

}

void RTPipeline::bindSets(vk::CommandBuffer * cmd, vector<VulkanSetRef> && sets)
{
	int i = 0;

	for (auto &set : sets) {
		if (set != nullptr) {
			_descriptorSets[i++] = set->getDescriptorSet();
		}
	}


	cmd->bindDescriptorSets(
		vk::PipelineBindPoint::eRayTracingNV,
		_pipelineLayout,
		0,
		i,
		i > 0 ? _descriptorSets : nullptr,
		0,
		nullptr,
		_ctx->getDynamicDispatch()
	);
}

void RTPipeline::traceRays(vk::CommandBuffer * cmd, glm::uvec2 resolution)
{

	uint64_t stride = _RTProps.shaderGroupHandleSize;

	cmd->traceRaysNV(
		_sbtBuffer->getBuffer(),
		0,
		_sbtBuffer->getBuffer(), //Miss groups
		stride * (1 + _shader->getNumHitGroups()),
		stride,
		_sbtBuffer->getBuffer(), //Hit Groups
		stride * 1,
		stride,
		nullptr,
		0,
		stride,
		resolution.x,
		resolution.y,
		1,
		_ctx->getDynamicDispatch()
	);
}




RTShaderBuilder::RTShaderBuilder(VulkanContextRef ctx, const char * raygenPath, vector<VulkanSetLayoutRef> && setLayouts)
	:_ctx(ctx),
	_setLayouts(setLayouts),
	_numHitGroups(0)
{
	vk::RayTracingShaderGroupCreateInfoNV groupInfo;


	for (auto &layout : _setLayouts) {
		_descriptorSetLayouts.push_back(layout->getDescriptorLayout());
	}

	groupInfo.setType(vk::RayTracingShaderGroupTypeNV::eGeneral);
	groupInfo.setGeneralShader(0);
	groupInfo.setAnyHitShader(VK_SHADER_UNUSED_NV);
	groupInfo.setClosestHitShader(VK_SHADER_UNUSED_NV);
	groupInfo.setIntersectionShader(VK_SHADER_UNUSED_NV);

	vk::ShaderModule module = VulkanShader::createModule(ctx, raygenPath);

	vk::PipelineShaderStageCreateInfo stage;
	stage.setStage(vk::ShaderStageFlagBits::eRaygenNV);
	stage.setPName("main");
	stage.setModule(module);

	_modules.push_back(module);
	_stages.push_back(stage);
	_groups.push_back(groupInfo);
}

RTShaderBuilder::~RTShaderBuilder()
{
	for (auto & module : _modules) {
		_ctx->getDevice().destroyShaderModule(module);
	}
}

void RTShaderBuilder::addMissGroup(const char * missPath)
{
	vk::RayTracingShaderGroupCreateInfoNV groupInfo;

	groupInfo.setType(vk::RayTracingShaderGroupTypeNV::eGeneral);
	groupInfo.setGeneralShader(0);
	groupInfo.setAnyHitShader(VK_SHADER_UNUSED_NV);
	groupInfo.setClosestHitShader(VK_SHADER_UNUSED_NV);
	groupInfo.setIntersectionShader(VK_SHADER_UNUSED_NV);


	vk::ShaderModule module = VulkanShader::createModule(_ctx, missPath);
	_modules.push_back(module);

	vk::PipelineShaderStageCreateInfo stage;
	stage.setStage(vk::ShaderStageFlagBits::eMissNV);
	stage.setPName("main");
	stage.setModule(module);

	_stages.push_back(stage);
	groupInfo.setGeneralShader(_stages.size() - 1);

	_groups.push_back(groupInfo);
}


void RTShaderBuilder::addHitGroup(const char * closestHitPath, const char * anyHitPath)
{
	vk::RayTracingShaderGroupCreateInfoNV groupInfo;

	groupInfo.setType(vk::RayTracingShaderGroupTypeNV::eTrianglesHitGroup);
	groupInfo.setGeneralShader(VK_SHADER_UNUSED_NV);
	groupInfo.setAnyHitShader(VK_SHADER_UNUSED_NV);
	groupInfo.setClosestHitShader(VK_SHADER_UNUSED_NV);
	groupInfo.setIntersectionShader(VK_SHADER_UNUSED_NV);


	if (closestHitPath != nullptr) {
		vk::ShaderModule module = VulkanShader::createModule(_ctx, closestHitPath);
		_modules.push_back(module);

		vk::PipelineShaderStageCreateInfo stage;
		stage.setStage(vk::ShaderStageFlagBits::eClosestHitNV);
		stage.setPName("main");
		stage.setModule(module);

		_stages.push_back(stage);
		groupInfo.setClosestHitShader(_stages.size() - 1);

	}

	if (anyHitPath != nullptr) {
		vk::ShaderModule module = VulkanShader::createModule(_ctx, anyHitPath);
		_modules.push_back(module);

		vk::PipelineShaderStageCreateInfo stage;
		stage.setStage(vk::ShaderStageFlagBits::eAnyHitNV);
		stage.setPName("main");
		stage.setModule(module);

		_stages.push_back(stage);
		groupInfo.setAnyHitShader(_stages.size() - 1);
	}
	_groups.push_back(groupInfo);

	_numHitGroups++;
}
