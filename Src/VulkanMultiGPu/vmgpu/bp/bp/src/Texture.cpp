#include <bp/Texture.h>
#include <bp/Util.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;

namespace bp
{

Texture::~Texture()
{
	if (isReady())
	{
		destroy();
		if (imageUsage & VK_IMAGE_USAGE_SAMPLED_BIT)
			vkDestroySampler(*device, sampler, nullptr);
	}
}

void Texture::init(Device& device, VkFormat format, VkImageUsageFlags usage, uint32_t width,
			   uint32_t height)
{
	Attachment::device = &device;
	Attachment::format = format;
	Attachment::width = width;
	Attachment::height = height;
	imageUsage = usage;

	if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		renderLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		renderAccessFlags = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
				    | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		renderPipelineStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	} else if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		renderLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		renderAccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
				    | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		renderPipelineStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	}

	create();

	if (!(usage & VK_IMAGE_USAGE_SAMPLED_BIT)) return;

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkResult result = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
	if (result != VK_SUCCESS)
		throw exception("Failed to create sampler.");

	descriptor.setType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	descriptor.addDescriptorInfo({sampler, imageView,
				      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
}

void Texture::load(Device& device, VkImageUsageFlags usage, const string& path)
{
	int x, y, n;
	unsigned char* raw = stbi_load(path.c_str(), &x, &y, &n, 4);

	init(device, VK_FORMAT_R8G8B8A8_UNORM, usage,
	     static_cast<uint32_t>(x), static_cast<uint32_t>(y));
	parallelCopy(image->map(), raw, static_cast<size_t>(x * y * 4));
	image->flushStagingBuffer();

	stbi_image_free(raw);
}

void Texture::resize(uint32_t width, uint32_t height)
{
	if (width != Attachment::width || height != Attachment::height)
	{
		destroy();
		Attachment::width = width;
		Attachment::height = height;
		create();

		if (imageUsage & VK_IMAGE_USAGE_SAMPLED_BIT)
		{
			descriptor.resetDescriptorInfos();
			descriptor.addDescriptorInfo({sampler, imageView,
						      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
		}
	}
}

void Texture::transitionShaderReadable(VkCommandBuffer cmdBuffer, VkPipelineStageFlags stage)
{
	image->transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT,
			  stage, cmdBuffer);
}

void Texture::before(VkCommandBuffer cmdBuffer)
{
	image->transition(renderLayout, renderAccessFlags, renderPipelineStage, cmdBuffer);
}

void Texture::create()
{
	image = new Image(*device, width, height, format, VK_IMAGE_TILING_OPTIMAL, imageUsage,
			  VMA_MEMORY_USAGE_GPU_ONLY);

	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = *image;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = format;

	if (format == VK_FORMAT_D16_UNORM)
	{
		imageViewInfo.components = {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
	} else
	{
		imageViewInfo.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
	}
	imageViewInfo.subresourceRange.aspectMask = format != VK_FORMAT_D16_UNORM
						    ? VK_IMAGE_ASPECT_COLOR_BIT
						    : VK_IMAGE_ASPECT_DEPTH_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;

	VkResult result = vkCreateImageView(*device, &imageViewInfo, nullptr, &imageView);
	if (result != VK_SUCCESS)
		throw exception("Failed to create image view.");
}

void Texture::destroy()
{
	vkDestroyImageView(*device, imageView, nullptr);
	delete image;
}

}