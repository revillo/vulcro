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
	else {
		throw new std::system_error(std::error_code(), "Shader not found");
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

	
	_modules.push_back(loadModule(vertPath));
	_modules.push_back(loadModule(fragPath));
	

	_stages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eVertex,
			_modules[0],
			"main",
			nullptr
		)
	);


	_stages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eFragment,
			_modules[1],
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

VulkanShader::VulkanShader(VulkanContextRef ctx, 
	const char * vertPath, const char * tessControlPath, const char * tessEvalPath,
	const char * tessGeomPath, const char * fragPath, 
	vector<VulkanVertexLayoutRef> vertexLayouts, vector<VulkanUniformSetLayoutRef> uniformLayouts)
	:_ctx(ctx),
	_uniformLayouts(uniformLayouts)
{

	_modules.push_back(loadModule(vertPath));
	_modules.push_back(loadModule(tessControlPath));
	_modules.push_back(loadModule(tessEvalPath));
	if (tessGeomPath != nullptr) {
		_modules.push_back(loadModule(tessGeomPath));
	}
	_modules.push_back(loadModule(fragPath));

	int stageIndex = 0;

	_stages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eVertex,
			_modules[stageIndex++],
			"main",
			nullptr
		)
	);


	_stages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eTessellationControl,
			_modules[stageIndex++],
			"main",
			nullptr
		)
	);

	_stages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eTessellationEvaluation ,
			_modules[stageIndex++],
			"main",
			nullptr
		)
	);

	if (tessGeomPath != nullptr) {

		_stages.push_back(
			vk::PipelineShaderStageCreateInfo(
				vk::PipelineShaderStageCreateFlags(),
				vk::ShaderStageFlagBits::eGeometry,
				_modules[stageIndex++],
				"main",
				nullptr
			)
		);
	}

	_stages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eFragment,
			_modules[stageIndex++],
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

VulkanShader::VulkanShader(VulkanContextRef ctx, const char * computePath, vector<VulkanUniformSetLayoutRef> uniformLayouts)
	: _ctx(ctx)
	, _uniformLayouts(uniformLayouts)
{
	
	_modules.push_back(loadModule(computePath));

	_stages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eCompute,
			_modules[0],
			"main",
			nullptr
		)
	);

	//Uniforms
	for (auto &layout : _uniformLayouts) {
		_descriptorSetLayouts.push_back(layout->getDescriptorLayout());
	}
}



vk::ShaderModule VulkanShader::loadModule(const char * path)
{
	uint32 csize;
	uint32 *cdata;
	readFile(path, csize, &cdata);

	auto module = _ctx->getDevice().createShaderModule(
		vk::ShaderModuleCreateInfo(
			vk::ShaderModuleCreateFlags(),
			csize,
			cdata
		)
	);

	delete cdata;
	
	return module;
}

VulkanShader::~VulkanShader()
{

	for (auto &mod : _modules) {
		_ctx->getDevice().destroyShaderModule(mod);
	}
}