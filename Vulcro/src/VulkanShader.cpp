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


VulkanShader::VulkanShader(
	VulkanContextRef ctx, 
	const char * vertPath, 
	const char * fragPath, 
	vector<VulkanVertexLayoutRef> vertexLayouts,
	vector<VulkanUniformSetLayoutRef> uniformLayouts) 
	
	:
	_ctx(ctx),
	_vertexLayouts(vertexLayouts),
	_uniformLayouts(uniformLayouts)
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
	
	//Uniforms
	for (auto &layout : _uniformLayouts) {
		_descriptorSetLayouts.push_back(layout->getDescriptorLayout());
	}


	//Attributes
	int binding = 0;

	for (auto & vertexLayout : _vertexLayouts) {

		auto nviads = vertexLayout->getVIADS(binding);
		_viads.insert(_viads.end(), nviads.begin(), nviads.end());


		_vibds.push_back(vertexLayout->getVIBD(binding));

		binding += 1;
	}

	_vis = vk::PipelineVertexInputStateCreateInfo(
		vk::PipelineVertexInputStateCreateFlags(),
		static_cast<uint32>(_vibds.size()),
		_vibds.size() > 0 ? &_vibds[0] : nullptr,
		static_cast<uint32>(_viads.size()),
		_viads.size() > 0 ? &_viads[0] : nullptr
	);
	
}

VulkanShader::~VulkanShader()
{
	_ctx->getDevice().destroyShaderModule(_vertModule);
	_ctx->getDevice().destroyShaderModule(_fragModule);
}
