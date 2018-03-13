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


VulkanShader::VulkanShader(VulkanContextRef ctx, const char * vertPath, const char * fragPath) :
	_ctx(ctx)
{ 

	uint64 vsize;
	uint32 *vdata;
	readFile(vertPath, vsize, &vdata);

	auto vertModule = _ctx->getDevice().createShaderModule(
		vk::ShaderModuleCreateInfo(
			vk::ShaderModuleCreateFlags(),
			vsize,
			vdata
		)
	);

	uint64 fsize;
	uint32 *fdata;
	readFile(fragPath, fsize, &fdata);

	auto fragModule = _ctx->getDevice().createShaderModule(
		vk::ShaderModuleCreateInfo(
			vk::ShaderModuleCreateFlags(),
			fsize,
			fdata
		)
	);



	//delete data?
	
}

VulkanShader::~VulkanShader()
{
}
