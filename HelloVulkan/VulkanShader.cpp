#include "VulkanShader.h"
#include <iostream>
#include <fstream>

void readFile(const char * filepath, uint32 &size, uint32 ** data) {

	ifstream file(filepath, ios::in | ios::binary);
	char * memblock;

	if (file.is_open()) {
		// get length of file:
		file.seekg(0, file.end);
		size = static_cast<uint32>(file.tellg());
		file.seekg(0, file.beg);

		memblock = new char[size];
		file.read(memblock, size);
		file.close();
		*data = (uint32*)memblock;
	}


}


VulkanShader::VulkanShader(VulkanContextRef ctx, const char * vertPath, const char * fragPath, vector<VulkanUniformLayoutRef> layouts) :
	_ctx(ctx),
	_layouts(layouts)
{ 

	uint32 vsize;
	uint32 *vdata;
	readFile(vertPath, vsize, &vdata);

	_vertModule = _ctx->getDevice().createShaderModule(
		vk::ShaderModuleCreateInfo(
			vk::ShaderModuleCreateFlags(),
			vsize,
			vdata
		)
	);


	uint32 fsize;
	uint32 *fdata;
	readFile(fragPath, fsize, &fdata);

	_fragModule = _ctx->getDevice().createShaderModule(
		vk::ShaderModuleCreateInfo(
			vk::ShaderModuleCreateFlags(),
			fsize,
			fdata
		)
	);

	delete vdata;
	delete fdata;
	

	_stages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eVertex,
			_vertModule,
			"main",
			nullptr
		)
	);


	_stages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eFragment,
			_fragModule,
			"main",
			nullptr
		)
	);

	for (auto &layout : layouts) {
		_descriptorSetLayouts.push_back(layout->getDescriptorLayout());
	}

	//createDescriptors();
	
}

VulkanShader::~VulkanShader()
{
	_ctx->getDevice().destroyShaderModule(_vertModule);
	_ctx->getDevice().destroyShaderModule(_fragModule);

	/*
	if (layoutCreated) {
		_ctx->getDevice().destroyDescriptorSetLayout(_descriptorLayout);

		for (int i = 0; i < _uniformSets.size(); i++) {
			_ctx->getDevice().freeDescriptorSets(
				_ctx->getDescriptorPool(_uniformSets[i].type, _uniformSets[i].descriptorCount),
				1,
				&_descriptorSets[i]
			);
		}
	}*/
}

/*
void VulkanShader::createDescriptors()
{

	if (_uniformSets.size() == 0) {
		return;
	}

	vector <vk::DescriptorSetLayoutBinding> bindings;

	for (auto &set : _uniformSets) {

		bindings.push_back(vk::DescriptorSetLayoutBinding(
			bindings.size(),
			set.type,
			set.descriptorCount,
			vk::ShaderStageFlagBits::eAllGraphics,
			set.samplers
		));
	}

	_descriptorLayout = _ctx->getDevice().createDescriptorSetLayout(
		vk::DescriptorSetLayoutCreateInfo(
			vk::DescriptorSetLayoutCreateFlags(),
			bindings.size(),
			&bindings[0]
		)
	);


	for (auto &set : _uniformSets) {
		_descriptorSets.push_back(_ctx->getDevice().allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				_ctx->getDescriptorPool(set.type, set.descriptorCount),
					set.descriptorCount,
					&_descriptorLayout
				)
			)[0]
		);
	}

	layoutCreated = true;
}
*/