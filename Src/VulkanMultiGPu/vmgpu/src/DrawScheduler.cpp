#include "Vmgpu.h"
#include "SLRenderer.h"
#include <bpMulti/SortLastCompositor.h>

using namespace bp;
using namespace bpMulti;
using namespace std;

void Vmgpu::redistributeGeometry()
{
	if (options.strategy != Strategy::SortLast) return;


	
	for (int i = 0; i < renderers.size(); i++)
	{
		ResourceManager& resourceManager = (*static_pointer_cast<SLRenderer>(renderers[i])).getResourceManager();

		resourceManager.removeAllMeshesAndModelsInstances();
	}
	
	if ((options.list || options.directory) && options.basic
		&& scene.meshes.size() >= options.deviceCount)
	{
		for (unsigned i = 0; i < scene.meshes.size(); i++)
		{
			auto& renderer =
				*static_pointer_cast<SLRenderer>(renderers[i % renderers.size()]);
			auto& rm = renderer.getResourceManager();
			const auto& mesh = scene.meshes[i];
			unsigned meshId = rm.addMesh(mesh);
			rm.addMeshInstance(meshId, scene.node);
		}
	}
	else if (!options.basic && scene.models.size() >= options.deviceCount)
	{
		for (unsigned i = 0; i < scene.models.size(); i++)
		{
			auto& renderer =
				*static_pointer_cast<SLRenderer>(renderers[i % renderers.size()]);
			auto& rm = renderer.getResourceManager();
			const auto& model = scene.models[i];
			unsigned modelId = rm.addModel(model);
			rm.addModelInstance(modelId, scene.node);
		}
	}
	else
	{
		throw runtime_error("Unable to distribute the geometry to the sortlast compositor");
	}
	
}