#include "RTPipeline.h"
#include "../vulkan-core/VulkanShader.h"
#include "../vulkan-core/VulkanSet.h"

RTPipeline::RTPipeline(VulkanContextPtr ctx, RTShaderBuilderRef shader):
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
		static_cast<uint32_t>(shader->getStages().size()),
		shader->getStages().data(),
		static_cast<uint32_t>(shader->getGroups().size()),
		shader->getGroups().data(),
		1, //Max Recursion
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
	devProps.properties = vk::PhysicalDeviceProperties{ };
	_ctx->getPhysicalDevice().getProperties2(&devProps, _ctx->getDynamicDispatch());

	uint64_t size = _RTProps.shaderGroupHandleSize * shader->getGroups().size();

	_sbtBuffer = _ctx->makeBuffer(vk::BufferUsageFlagBits::eRayTracingNV | vk::BufferUsageFlagBits::eTransferSrc, size, vk::MemoryPropertyFlagBits::eHostVisible);

	auto ptr = _sbtBuffer->getMapped();

	vk::Result res = _ctx->getDevice().getRayTracingShaderGroupHandlesNV(_pipeline, 0, static_cast<uint32_t>(shader->getGroups().size()), size, ptr, _ctx->getDynamicDispatch());

	uint16_t* up = (uint16_t*)ptr;

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

void RTPipeline::bindSets(vk::CommandBuffer * cmd, vk::ArrayProxy<const VulkanSetRef> sets)
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
	traceRays(cmd, glm::uvec3(resolution, 1.0));
}

void RTPipeline::traceRays(vk::CommandBuffer * cmd, glm::uvec3 resolution)
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
		_sbtBuffer->getBuffer(), //Callables
		stride * (1 + _shader->getNumHitGroups() + _shader->getNumMissGroups()),
		stride,
		resolution.x,
		resolution.y,
		resolution.z,
		_ctx->getDynamicDispatch()
	);
}




RTShaderBuilder::RTShaderBuilder(VulkanContextPtr ctx, const char * raygenPath, vk::ArrayProxy<const VulkanSetLayoutRef> setLayouts)
	:_ctx(ctx),
	_setLayouts((VulkanSetLayoutRef*)setLayouts.begin(), (VulkanSetLayoutRef*)setLayouts.end()),
	_numHitGroups(0),
    _numMissGroups(0)
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
	groupInfo.setGeneralShader(static_cast<uint32_t>(_stages.size()) - 1);

	_groups.push_back(groupInfo);

    _numMissGroups++;
}

void RTShaderBuilder::addCallableGroup(const char * callablePath)
{
    vk::RayTracingShaderGroupCreateInfoNV groupInfo;

    groupInfo.setType(vk::RayTracingShaderGroupTypeNV::eGeneral);
    groupInfo.setGeneralShader(0);
    groupInfo.setAnyHitShader(VK_SHADER_UNUSED_NV);
    groupInfo.setClosestHitShader(VK_SHADER_UNUSED_NV);
    groupInfo.setIntersectionShader(VK_SHADER_UNUSED_NV);

    vk::ShaderModule module = VulkanShader::createModule(_ctx, callablePath);
    _modules.push_back(module);

    vk::PipelineShaderStageCreateInfo stage;
    stage.setStage(vk::ShaderStageFlagBits::eCallableNV);
    stage.setPName("main");
    stage.setModule(module);

    _stages.push_back(stage);
    groupInfo.setGeneralShader(static_cast<uint32_t>(_stages.size()) - 1);

    _groups.push_back(groupInfo);


}

void RTShaderBuilder::addHitGroup(const char * closestHitPath, const char * anyHitPath, const char * intersectionPath)
{
	vk::RayTracingShaderGroupCreateInfoNV groupInfo;

    if (intersectionPath)   
    	groupInfo.setType(vk::RayTracingShaderGroupTypeNV::eProceduralHitGroup);
    else
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
		groupInfo.setClosestHitShader(static_cast<uint32_t>(_stages.size()) - 1);

	}

	if (anyHitPath != nullptr) {
		vk::ShaderModule module = VulkanShader::createModule(_ctx, anyHitPath);
		_modules.push_back(module);

		vk::PipelineShaderStageCreateInfo stage;
		stage.setStage(vk::ShaderStageFlagBits::eAnyHitNV);
		stage.setPName("main");
		stage.setModule(module);

		_stages.push_back(stage);
		groupInfo.setAnyHitShader(static_cast<uint32_t>(_stages.size()) - 1);
	}

    if (intersectionPath != nullptr) {
        vk::ShaderModule module = VulkanShader::createModule(_ctx, intersectionPath);
        _modules.push_back(module);

        vk::PipelineShaderStageCreateInfo stage;
        stage.setStage(vk::ShaderStageFlagBits::eIntersectionNV);
        stage.setPName("main");
        stage.setModule(module);

        _stages.push_back(stage);
        groupInfo.setIntersectionShader(static_cast<uint32_t>(_stages.size()) - 1);
    }

	_groups.push_back(groupInfo);

	_numHitGroups++;
}
