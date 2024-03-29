#include "ResourceManager.h"
#include <bp/Util.h>
#include <glm/gtx/component_wise.hpp>

using namespace bp;
using namespace bpScene;

void ResourceManager::init(Device& device, RenderPass& renderPass, Camera& camera,
			   bool generateNormals)
{
	ResourceManager::device = &device;
	ResourceManager::camera = &camera;
	ResourceManager::generateNormals = generateNormals;

	//Load shader byte code
	auto shaderCode = readBinaryFile(generateNormals ? "spv/noNormal.vert.spv"
							 : "spv/basic.vert.spv");
	vertexBasic.init(device, VK_SHADER_STAGE_VERTEX_BIT,
			 static_cast<uint32_t>(shaderCode.size()),
			 reinterpret_cast<const uint32_t*>(shaderCode.data()));

	shaderCode = readBinaryFile(generateNormals ? "spv/noNormalUv.vert.spv"
						    : "spv/basicUv.vert.spv");
	vertexBasicUv.init(device, VK_SHADER_STAGE_VERTEX_BIT,
			   static_cast<uint32_t>(shaderCode.size()),
			   reinterpret_cast<const uint32_t*>(shaderCode.data()));

	if (generateNormals)
	{
		shaderCode = readBinaryFile("spv/generateNormals.geom.spv");
		geomNormals.init(device, VK_SHADER_STAGE_GEOMETRY_BIT,
				 static_cast<uint32_t>(shaderCode.size()),
				 reinterpret_cast<const uint32_t*>(shaderCode.data()));

		shaderCode = readBinaryFile("spv/generateNormalsUv.geom.spv");
		geomNormalsUv.init(device, VK_SHADER_STAGE_GEOMETRY_BIT,
				   static_cast<uint32_t>(shaderCode.size()),
				   reinterpret_cast<const uint32_t*>(shaderCode.data()));
	}

	shaderCode = readBinaryFile("spv/basic.frag.spv");
	fragmentBasic.init(device, VK_SHADER_STAGE_FRAGMENT_BIT,
			   static_cast<uint32_t>(shaderCode.size()),
			   reinterpret_cast<const uint32_t*>(shaderCode.data()));

	/*
	shaderCode = readBinaryFile("spv/material.frag.spv");
	fragmentColored.init(device, VK_SHADER_STAGE_FRAGMENT_BIT,
			     static_cast<uint32_t>(shaderCode.size()),
			     reinterpret_cast<const uint32_t*>(shaderCode.data()));
	*/
				 


	shaderCode = readBinaryFile("spv/material_tr.frag.spv");
	fragmentColored.init(device, VK_SHADER_STAGE_FRAGMENT_BIT,
		static_cast<uint32_t>(shaderCode.size()),
		reinterpret_cast<const uint32_t*>(shaderCode.data()));

	/*
	shaderCode = readBinaryFile("spv/materialTextured.frag.spv");
	fragmentTextured.init(device, VK_SHADER_STAGE_FRAGMENT_BIT,
			      static_cast<uint32_t>(shaderCode.size()),
			      reinterpret_cast<const uint32_t*>(shaderCode.data()));
				  */

	shaderCode = readBinaryFile("spv/materialTextured_tr.frag.spv");
	fragmentTextured.init(device, VK_SHADER_STAGE_FRAGMENT_BIT,
		static_cast<uint32_t>(shaderCode.size()),
		reinterpret_cast<const uint32_t*>(shaderCode.data()));

	//Setup descriptor set layouts
	setLayoutColored.addLayoutBinding({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
					    VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	setLayoutColored.init(device);

	setLayoutTextured.addLayoutBinding({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
					    VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	setLayoutTextured.addLayoutBinding({1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
					      VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	setLayoutTextured.init(device);

	//Setup pipeline layouts
	pipelineLayoutBasic.addPushConstantRange({VK_SHADER_STAGE_VERTEX_BIT
						  | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
						  sizeof(PushConstantResource::Matrices)});
	pipelineLayoutBasic.init(device);

	pipelineLayoutColored.addDescriptorSetLayout(setLayoutColored);
	pipelineLayoutColored.addPushConstantRange({VK_SHADER_STAGE_VERTEX_BIT
						    | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
						    sizeof(PushConstantResource::Matrices)});
	pipelineLayoutColored.init(device);

	pipelineLayoutTextured.addDescriptorSetLayout(setLayoutTextured);
	pipelineLayoutTextured.addPushConstantRange({VK_SHADER_STAGE_VERTEX_BIT
						     | VK_SHADER_STAGE_GEOMETRY_BIT, 0,
						     sizeof(PushConstantResource::Matrices)});
	pipelineLayoutTextured.init(device);
	
	//Setup pipelines
	pipelineBasic.addShaderStageInfo(vertexBasic.getPipelineShaderStageInfo());
	pipelineBasic.addShaderStageInfo(fragmentBasic.getPipelineShaderStageInfo());
	pipelineBasic.addVertexBindingDescription({0, sizeof(glm::vec3),
						   VK_VERTEX_INPUT_RATE_VERTEX});
	pipelineBasic.addVertexAttributeDescription({0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0});
	if (generateNormals)
	{
		pipelineBasic.addShaderStageInfo(geomNormals.getPipelineShaderStageInfo());
	} else
	{
		pipelineBasic.addVertexBindingDescription({1, sizeof(glm::vec3),
							   VK_VERTEX_INPUT_RATE_VERTEX});
		pipelineBasic.addVertexAttributeDescription({1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0});
	}
	pipelineBasic.init(device, renderPass, pipelineLayoutBasic);

	pipelineColored.addShaderStageInfo(vertexBasic.getPipelineShaderStageInfo());
	pipelineColored.addShaderStageInfo(fragmentColored.getPipelineShaderStageInfo());
	pipelineColored.addVertexBindingDescription({0, sizeof(glm::vec3),
						   VK_VERTEX_INPUT_RATE_VERTEX});
	pipelineColored.addVertexAttributeDescription({0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0});
	if (generateNormals)
	{
		pipelineColored.addShaderStageInfo(geomNormals.getPipelineShaderStageInfo());
	} else
	{
		pipelineColored.addVertexBindingDescription({1, sizeof(glm::vec3),
							     VK_VERTEX_INPUT_RATE_VERTEX});
		pipelineColored.addVertexAttributeDescription({1, 1, VK_FORMAT_R32G32B32_SFLOAT,
							       0});
	}
	pipelineColored.init(device, renderPass, pipelineLayoutColored);

	pipelineTextured.addShaderStageInfo(vertexBasicUv.getPipelineShaderStageInfo());
	pipelineTextured.addShaderStageInfo(fragmentTextured.getPipelineShaderStageInfo());
	pipelineTextured.addVertexBindingDescription({0, sizeof(glm::vec3),
						      VK_VERTEX_INPUT_RATE_VERTEX});
	pipelineTextured.addVertexAttributeDescription({0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0});

	if (generateNormals)
	{
		pipelineTextured.addShaderStageInfo(geomNormalsUv.getPipelineShaderStageInfo());
		pipelineTextured.addVertexBindingDescription({1, sizeof(glm::vec2),
							      VK_VERTEX_INPUT_RATE_VERTEX});
		pipelineTextured.addVertexAttributeDescription({1, 1, VK_FORMAT_R32G32_SFLOAT, 0});
	} else
	{
		pipelineTextured.addVertexBindingDescription({1, sizeof(glm::vec3),
							      VK_VERTEX_INPUT_RATE_VERTEX});
		pipelineTextured.addVertexBindingDescription({2, sizeof(glm::vec2),
							      VK_VERTEX_INPUT_RATE_VERTEX});
		pipelineTextured.addVertexAttributeDescription({1, 1, VK_FORMAT_R32G32B32_SFLOAT,
								0});
		pipelineTextured.addVertexAttributeDescription({2, 2, VK_FORMAT_R32G32_SFLOAT, 0});
	}
	pipelineTextured.init(device, renderPass, pipelineLayoutTextured);
}

unsigned ResourceManager::addModel(const Model& model)
{
	unsigned id = models.createResource();
	DescriptorSetLayout* setLayout;
	modelIsTextured.push_back(model.getMaterial(0).isTextured());
	if (modelIsTextured[id]) setLayout = &setLayoutTextured;
	else setLayout = &setLayoutColored;
	models[id].init(*device, *setLayout, 1, 0, model);
	return id;
}

unsigned ResourceManager::addMesh(const bpScene::Mesh& mesh, uint32_t offset, uint32_t count)
{
	unsigned id = meshes.createResource();
	meshes[id].init(*device, mesh, offset, count);
	return id;
}

ResourceManager::ModelInstanceIndexes ResourceManager::addModelInstance(unsigned modelIndex, Node& node)
{
	unsigned drawableId = modelDrawables.createResource();

	modelDrawables[drawableId].init(modelIsTextured[modelIndex] ? pipelineTextured
								    : pipelineColored,
					models[modelIndex]);
	unsigned pushId = pushConstants.createResource();
	pushConstants[pushId].init(modelIsTextured[modelIndex] ? pipelineLayoutTextured
							       : pipelineLayoutColored,
				   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, node,
				   *camera);
	bpUtil::connect(modelDrawables[drawableId].resourceBindingEvent, pushConstants[pushId],
			&PushConstantResource::bind);
	subpass.addDrawable(modelDrawables[drawableId]);

	return ResourceManager::ModelInstanceIndexes{ drawableId, pushId };
}

ResourceManager::MeshInstanceIndexes ResourceManager::addMeshInstance(unsigned meshId, bpScene::Node& node)
{
	auto& mesh = meshes[meshId];
	unsigned drawableId = meshDrawables.createResource();
	meshDrawables[drawableId].init(pipelineBasic, mesh, mesh.getOffset(),
				       mesh.getElementCount());
	unsigned pushId = pushConstants.createResource();
	pushConstants[pushId].init(pipelineLayoutBasic,
				   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, node,
				   *camera);
	bpUtil::connect(meshDrawables[drawableId].resourceBindingEvent, pushConstants[pushId],
			&PushConstantResource::bind);
	subpass.addDrawable(meshDrawables[drawableId]);

	return ResourceManager::MeshInstanceIndexes{ drawableId, pushId };
}

void ResourceManager::removeModelInstance(ModelInstanceIndexes _indexes)
{
	if (modelDrawables.hasElementAt(_indexes.DrawableID) && pushConstants.hasElementAt(_indexes.PushID))
	{
		subpass.removeDrawable(modelDrawables[_indexes.DrawableID]);

		modelDrawables.freeResource(_indexes.DrawableID);
		pushConstants.freeResource(_indexes.PushID);
	}
	else
		throw std::exception("invalid indexes!");
}

void ResourceManager::removeMeshInstance(MeshInstanceIndexes _indexes)
{
	if (meshDrawables.hasElementAt(_indexes.DrawableID) && pushConstants.hasElementAt(_indexes.PushID))
	{
		subpass.removeDrawable(meshDrawables[_indexes.DrawableID]);

		meshDrawables.freeResource(_indexes.DrawableID);
		pushConstants.freeResource(_indexes.PushID);
	}
	else
		throw std::exception("invalid indexes!");
}

void ResourceManager::setClipTransform(const glm::mat4& transform)
{
	if (pushConstants.size() <= 0) return;
	auto it = pushConstants.begin();
	do
	{
		if ((*(it.elementIterator)))
			(**(it.elementIterator)).setClipTransform(transform);
		it++;
	} while (it.elementIterator != it.end);
}

void ResourceManager::updatePushConstants()
{
	if (pushConstants.size() <= 0) return;
	auto it = pushConstants.begin();
	do
	{
		if ((*(it.elementIterator)))
			(**(it.elementIterator)).update();
		it++;
	} while (it.elementIterator != it.end);
}

void ResourceManager::removeAllMeshesAndModelsInstances()
{
	if (meshDrawables.size() > 0)
	{
		auto it = meshDrawables.begin();
		do
		{
			if((*(it.elementIterator)))
				subpass.removeDrawable(**(it.elementIterator));
			it++;
		}
		while (it.elementIterator != it.end);
	}
	if (modelDrawables.size() > 0)
	{
		auto it = modelDrawables.begin();
		do
		{
			if ((*(it.elementIterator)))
				subpass.removeDrawable(**(it.elementIterator));
			it++;
		} while (it.elementIterator != it.end);
	}
	pushConstants.clear();
}
