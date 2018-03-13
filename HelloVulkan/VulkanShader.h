#pragma once
#include "VulkanContext.h"

class VulkanShader
{
public:
	VulkanShader(VulkanContextRef ctx, const char * vertPath, const char * fragPath);
	~VulkanShader();

private:
	VulkanContextRef _ctx;
};

