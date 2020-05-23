#pragma once

#include "../vulkan-core/VulkanContext.h"


class RTShaderBuilder {

public:

	RTShaderBuilder(VulkanContextPtr ctx, const char * raygenPath, vk::ArrayProxy<const VulkanSetLayoutRef> setLayouts);
	~RTShaderBuilder();

	//void addGroup(vk::ArrayProxy<StageBuilder> stages);
	void addHitGroup(const char * closestHitPath, const char * anyHitPath = nullptr, const char * intersectionPath = nullptr);
	//void addProceduralGroup(const char * intersectionPath);
	void addMissGroup(const char * missPath);

    void addCallableGroup(const char * callablePath);

	const vector<vk::DescriptorSetLayout> &getDescriptorSetLayouts() {
		return _descriptorSetLayouts;
	}

	const vector<vk::PipelineShaderStageCreateInfo> &getStages() {
		return _stages;
	}
	const vector<vk::RayTracingShaderGroupCreateInfoNV> &getGroups() {
		return _groups;
	}

    const uint64_t getNumHitGroups() const {
		return _numHitGroups;
	}

    const uint64_t getNumMissGroups() const {
        return _numMissGroups;
    }

protected:

	vector<vk::RayTracingShaderGroupCreateInfoNV > _groups;
	vector<vk::PipelineShaderStageCreateInfo> _stages;
	vector<VulkanSetLayoutRef> _setLayouts;
	vector<vk::ShaderModule> _modules;
	VulkanContextPtr _ctx;
	vector<vk::DescriptorSetLayout> _descriptorSetLayouts;
	uint64_t _numHitGroups;
    uint64_t _numMissGroups;
};

class RTPipeline {
public:

	RTPipeline(VulkanContextPtr ctx, RTShaderBuilderRef shader);
	~RTPipeline();

	void bind(vk::CommandBuffer * cmd);
	vk::Pipeline getPipeline() {
		return _pipeline;
	}
	void bindSets(vk::CommandBuffer * cmd, vk::ArrayProxy<const VulkanSetRef> sets);
	void traceRays(vk::CommandBuffer * cmd, glm::uvec2 resolution);
	void traceRays(vk::CommandBuffer * cmd, glm::uvec3 resolution);

protected:
	VulkanContextPtr _ctx;
	vk::Pipeline _pipeline;
	vk::PipelineLayout _pipelineLayout;
	vk::DescriptorSet _descriptorSets[16];
	RTShaderBuilderRef  _shader;
	VulkanBufferRef _sbtBuffer;
	vk::PhysicalDeviceRayTracingPropertiesNV _RTProps;


};

