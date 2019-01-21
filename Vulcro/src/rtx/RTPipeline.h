#include "VulkanContext.h"


class RTShaderBuilder {

public:

	RTShaderBuilder(VulkanContextRef ctx, const char * raygenPath, vector<VulkanSetLayoutRef> && setLayouts);
	~RTShaderBuilder();

	//void addGroup(vk::ArrayProxy<StageBuilder> stages);
	void addHitGroup(const char * closestHitPath, const char * anyHitPath);
	//void addProceduralGroup(const char * intersectionPath);
	void addMissGroup(const char * missPath);

	const vector<vk::DescriptorSetLayout> &getDescriptorSetLayouts() {
		return _descriptorSetLayouts;
	}

	const vector<vk::PipelineShaderStageCreateInfo> &getStages() {
		return _stages;
	}
	const vector<vk::RayTracingShaderGroupCreateInfoNV> &getGroups() {
		return _groups;
	}

	uint64_t getNumHitGroups() {
		return _numHitGroups;
	}



protected:

	vector<vk::RayTracingShaderGroupCreateInfoNV > _groups;
	vector<vk::PipelineShaderStageCreateInfo> _stages;
	vector<VulkanSetLayoutRef> _setLayouts;
	vector<vk::ShaderModule> _modules;
	VulkanContextRef _ctx;
	vector<vk::DescriptorSetLayout> _descriptorSetLayouts;
	uint64_t _numHitGroups;
};

class RTPipeline {
public:

	RTPipeline(VulkanContextRef ctx, RTShaderBuilderRef shader);
	~RTPipeline();

	void bind(vk::CommandBuffer * cmd);
	vk::Pipeline getPipeline() {
		return _pipeline;
	}
	void bindSets(vk::CommandBuffer * cmd, vector<VulkanSetRef> && sets);
	void traceRays(vk::CommandBuffer * cmd, glm::uvec2 resolution);

protected:
	VulkanContextRef _ctx;
	vk::Pipeline _pipeline;
	vk::PipelineLayout _pipelineLayout;
	vk::DescriptorSet _descriptorSets[16];
	RTShaderBuilderRef  _shader;
	VulkanBufferRef _sbtBuffer;
	vk::PhysicalDeviceRayTracingPropertiesNV _RTProps;


};

