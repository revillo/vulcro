#include "VulkanShader.h"
#include <iostream>
#include <fstream>

void readFile(const char * filepath, uint64 &size, uint32 ** data) {

	ifstream file(filepath, ios::in | ios::binary);
	char * memblock;

	if (file.is_open()) {
		// get length of file:
		file.seekg(0, file.end);
		size = file.tellg();
		file.seekg(0, file.beg);

		memblock = new char[size];
		file.read(memblock, size);
		file.close();
		*data = (uint32*)memblock;
	}


}


VulkanShader::VulkanShader(VulkanContextRef ctx, const char * vertPath, const char * fragPath) :
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
	
}

VulkanShader::~VulkanShader()
{
}
