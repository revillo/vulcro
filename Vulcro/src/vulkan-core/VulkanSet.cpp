#include "VulkanSet.h"

#include "VulkanSetLayout.h"
#include "../vulkan-rtx/RTAccelerationStructure.h"
#include "../vulkan-rtx/RTScene.h"

VulkanSet::VulkanSet(VulkanContextPtr ctx, VulkanSetLayoutRef layout) :
	_ctx(ctx),
	_layout(layout)
{
	_descriptorSet = layout->allocateDescriptorSet();
}

void VulkanSet::bindBuffer(uint32_t binding, VulkanBufferRef buffer)
{
    if (!buffer) return;

	vector<VulkanBufferRef> bv = { buffer };
	bindBufferArray(binding, bv);
}

void VulkanSet::bindBufferArray(uint32_t binding, vk::ArrayProxy<VulkanBufferRef> buffers)
{	
    if (buffers.size() == 0)
    {
        return;
    }

	auto type = buffers.front()->getDescriptorType();
	auto count = buffers.size();

	vk::WriteDescriptorSet write;

	if ((type == vk::DescriptorType::eStorageTexelBuffer) || (type == vk::DescriptorType::eUniformTexelBuffer)) {

		std::vector<vk::BufferView> views;
		views.reserve(count);

		for (auto & buffer : buffers) {
			if(buffer)
				views.push_back(buffer->getView());
		}

		write = vk::WriteDescriptorSet(
			_descriptorSet,
			binding,
			0,
			views.size(),
			type,
			nullptr,
			nullptr,
			views.data()
		);

		_ctx->getDevice().updateDescriptorSets(1, &write, 0, nullptr);
	}
	else {

		std::vector<vk::DescriptorBufferInfo> dbis;
		dbis.reserve(count);

		for (auto & buffer : buffers) {
			if(buffer)
				dbis.push_back(buffer->getDBI());
		}
		write = vk::WriteDescriptorSet(
			_descriptorSet,
			binding,
			0,
			dbis.size(),
			type,
			nullptr,
			dbis.data(),
			nullptr
		);

		_ctx->getDevice().updateDescriptorSets(
			1,
			&write,
			0,
			nullptr
		);

	}


}

void VulkanSet::bindBuffer(uint32_t binding, vk::DescriptorBufferInfo dbi, vk::DescriptorType type)
{

	auto write = vk::WriteDescriptorSet(
		_descriptorSet,
		binding,
		0,
		1,
		type,
		nullptr,
		&dbi,
		nullptr

	);

	_ctx->getDevice().updateDescriptorSets(
		1,
		&write,
		0,
		nullptr
	);
}


void VulkanSet::bindBuffer(uint32_t binding, iuboRef uboref)
{
	bindBuffer(binding, uboref->getDBI());
}

void VulkanSet::bindImageArray(uint32_t binding, vk::ArrayProxy<VulkanImageRef> images, vk::DescriptorType type)
{
	vector<vk::DescriptorImageInfo> diis;

	for (auto & img : images) {
		if (img) {
			diis.push_back(img->getDII());
		}
	}

    if (diis.size() == 0)
    {
        return;
    }

	auto write = vk::WriteDescriptorSet(
		_descriptorSet,
		binding,
		0,
		diis.size(),
		type,
		diis.data(),
		nullptr,
		nullptr
	);

	_ctx->getDevice().updateDescriptorSets(
		1,
		&write,
		0,
		nullptr
	);
}

void VulkanSet::bindImage(uint32_t binding, VulkanImageRef image, vk::DescriptorType type, uint16_t mipLevel)
{

	auto dii = image->getDII(mipLevel);

	if (type == vk::DescriptorType::eStorageImage) {
		dii.imageLayout = vk::ImageLayout::eGeneral;
	}

	auto write = vk::WriteDescriptorSet(
		_descriptorSet,
		binding,
		0,
		1,
		type,
		&dii,
		nullptr,
		nullptr

	);

	_ctx->getDevice().updateDescriptorSets(
		1,
		&write,
		0,
		nullptr
	);

}

void VulkanSet::bindTopStructure(uint32_t binding, RTTopStructureRef topStructure)
{
    if (!topStructure) return;

    auto writeas = topStructure->getWriteDescriptor();

    auto write = vk::WriteDescriptorSet(
        _descriptorSet,
        binding,
        0,
        1,
        vk::DescriptorType::eAccelerationStructureNV,
        nullptr,
        nullptr,
        nullptr
    );

    write.setPNext(&writeas);

    _ctx->getDevice().updateDescriptorSets(
        1,
        &write,
        0,
        nullptr,
        _ctx->getDynamicDispatch()
    );
}

void VulkanSet::bindRTScene(uint32_t binding, RTSceneRef rtscene)
{

	auto writeas = rtscene->getWriteDescriptor();

	auto write = vk::WriteDescriptorSet(
		_descriptorSet,
		binding,
		0,
		1,
		vk::DescriptorType::eAccelerationStructureNV,
		nullptr,
		nullptr,
		nullptr
	);

	write.setPNext(&writeas);

	_ctx->getDevice().updateDescriptorSets(
		1,
		&write,
		0,
		nullptr,
		_ctx->getDynamicDispatch()
	);
}

void VulkanSet::update() {

	if (_writes.size() == 0) return;

	_ctx->getDevice().updateDescriptorSets(
		static_cast<uint32_t>(_writes.size()),
		&_writes[0],
		0,
		nullptr,
        _ctx->getDynamicDispatch()
	);

	_writes.clear();

}

VulkanSet::~VulkanSet()
{

	_layout->freeDescriptorSet(_descriptorSet);

}
