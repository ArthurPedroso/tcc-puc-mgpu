#include <bp/Util.h>
#include <bpMulti/SortLastCompositor.h>
#include <future>
#include "SortLastCompositingSpv.inc"

using namespace std;

namespace bpMulti
{

void SortLastCompositor::init(vector<pair<bp::Device*, SortLastRenderer*>> configurations,
			      VkFormat colorFormat, uint32_t width, uint32_t height)
{
	deviceCount = static_cast<unsigned int>(configurations.size());
	auto iter = configurations.begin();
	bp::Device& primaryDevice = *iter->first;
	primaryRenderer = iter->second;
	iter++;
	for (; iter != configurations.end(); iter++)
	{
		secondaryDevices.push_back(iter->first);
		secondaryRenderers.push_back(iter->second);
	}
	Compositor::init(primaryDevice, colorFormat, width, height);
}

void SortLastCompositor::resize(uint32_t width, uint32_t height)
{
	primaryRenderDeviceSteps.resize(width, height);
	for (auto& c : primaryContributions) c.resize(width, height, false);

	for (auto& steps : secondaryRenderDeviceSteps) steps.resize(width, height);
	for (auto& c : secondaryContributions) c.resize(width, height);

	Renderer::resize(width, height);
	renderFirstFrame();
}

VkExtent2D SortLastCompositor::getContributionSize(unsigned deviceIndex)
{
	return {getWidth(), getHeight()};
}

void SortLastCompositor::initShaders()
{
	auto shaderCode = bp::readBinaryFile("spv/fullscreenQuad.vert.spv");
	vertexShader.init(getDevice(), VK_SHADER_STAGE_VERTEX_BIT,
		static_cast<uint32_t>(shaderCode.size()),
		reinterpret_cast<const uint32_t*>(shaderCode.data()));

	shaderCode = bp::readBinaryFile("spv/directTextureDepthTest.frag.spv");
	fragmentShader.init(getDevice(), VK_SHADER_STAGE_FRAGMENT_BIT,
		static_cast<uint32_t>(shaderCode.size()),
		reinterpret_cast<const uint32_t*>(shaderCode.data()));
}

void SortLastCompositor::initDescriptorSetLayout()
{
	descriptorSetLayout.addLayoutBinding({0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					      1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	descriptorSetLayout.addLayoutBinding({1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					      1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	descriptorSetLayout.init(getDevice());
}

void SortLastCompositor::initPipelineLayout()
{
	pipelineLayout.addDescriptorSetLayout(descriptorSetLayout);
	pipelineLayout.init(getDevice());
}

void SortLastCompositor::initPipeline()
{
	pipeline.addShaderStageInfo(vertexShader.getPipelineShaderStageInfo());
	pipeline.addShaderStageInfo(fragmentShader.getPipelineShaderStageInfo());
	pipeline.setFrontFace(VK_FRONT_FACE_CLOCKWISE);
	pipeline.init(getDevice(), getRenderPass(), pipelineLayout);
}

void SortLastCompositor::initDescriptorPool()
{
	descriptorPool.init(getDevice(),
			    {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, deviceCount * 2 + 2}},
			    deviceCount + 1);
}

void SortLastCompositor::setupContribution(Contribution& contribution)
{
	contribution.createTexture(VK_FORMAT_R8G8B8A8_UNORM, true);
	contribution.createTexture(VK_FORMAT_D16_UNORM, true);
}

}
