#include "VulkanShader.h"
#include <iostream>
#include <fstream>

void readFile(const char * filepath, uint64 &size, uint32 ** data) {

	ifstream file(filepath, ios::out | ios::app | ios::binary);
	char * memblock;

	if (file.is_open()) {
		size = file.tellg();
		memblock = new char[size];
		file.seekg(0, ios::beg);
		file.read(memblock, size);
		file.close();
		*data = (uint32*)memblock;
	}

	file.close();

}


VulkanShader::VulkanShader(VulkanContextRef ctx, const char * name, const char * vertPath, const char * fragPath) :
	_ctx(ctx)
{ 

	uint64 vsize;
	uint32 *vdata;
	readFile(vertPath, vsize, &vdata);

	_vertModule = _ctx->getDevice().createShaderModule(
		vk::ShaderModuleCreateInfo(
			vk::ShaderModuleCreateFlags(),
			vsize,
			vdata
		)
	);

	uint64 fsize;
	uint32 *fdata;
	readFile(fragPath, fsize, &fdata);

	_fragModule = _ctx->getDevice().createShaderModule(
		vk::ShaderModuleCreateInfo(
			vk::ShaderModuleCreateFlags(),
			fsize,
			fdata
		)
	);

	//delete data?
	

	_stages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eVertex,
			_vertModule,
			name,
			nullptr
		)
	);


	_stages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eFragment,
			_vertModule,
			name,
			nullptr
		)
	);
	
}

VulkanShader::~VulkanShader()
{
}
