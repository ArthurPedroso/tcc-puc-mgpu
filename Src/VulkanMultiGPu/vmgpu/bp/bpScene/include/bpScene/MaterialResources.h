#ifndef BP_SCENE_MATERIALRESOURCES_H
#define BP_SCENE_MATERIALRESOURCES_H

#include "Material.h"
#include <bp/Texture.h>
#include <bp/DescriptorPool.h>
#include <bp/DescriptorSet.h>
#include <bp/BufferDescriptor.h>
#include <bpUtil/Event.h>

namespace bpScene
{

class MaterialResources
{
public:
	struct MaterialUniform
	{
		glm::vec4 ambient, diffuse;
		glm::float_t opacity;
	};

	void init(bp::Device& device, const Material& material, bp::DescriptorPool& descriptorPool,
		  bp::DescriptorSetLayout& descriptorSetLayout,
		  uint32_t textureBinding, uint32_t uniformBinding,
		  bp::Buffer& uniformBuffer, VkDeviceSize offset);

	bp::DescriptorSet& getDescriptorSet() { return descriptorSet; }

	bpUtil::Event<const std::string&> loadMessageEvent;

private:
	bp::DescriptorSet descriptorSet;
	bp::Texture texture;
	bp::BufferDescriptor uniformBufferDescriptor;
};

}

#endif