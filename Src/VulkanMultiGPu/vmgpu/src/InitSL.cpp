#include "Vmgpu.h"
#include "SLRenderer.h"
#include <bpMulti/SortLastCompositor.h>

using namespace bp;
using namespace bpMulti;
using namespace std;

void Vmgpu::initSortLast(uint32_t width, uint32_t height)
{
	loadMessageEvent("Initializing renderers...");
	vector<pair<Device*, SortLastRenderer*>> configurations;
	for (auto& device : devices)
	{
		SLRenderer* renderer = new SLRenderer();
		renderer->setCamera(camera);
		renderer->setGenerateNormals(options.generateNormals);
		renderers.emplace_back(renderer);
		configurations.emplace_back(device.get(), renderer);
	}

	if (devices.size() > 1)
	{
		loadMessageEvent("Initializing sort-last compositor...");
		SortLastCompositor* compositor = new SortLastCompositor();
		compositor->init(move(configurations), swapchain.getFormat(), width, height);
		mainRenderer.reset(compositor);
	} else
	{
		mainRenderer = renderers[0];
		mainRenderer->init(device, swapchain.getFormat(), width, height);
	}

	for (auto& r : renderers)
	{
		auto& rm = static_pointer_cast<SLRenderer>(r)->getResourceManager();
		bpUtil::connect(rm.loadMessageEvent, loadMessageEvent);
	}

	if (!options.basic && scene.models.size() >= options.deviceCount)
	{
		SLRenderer& mainr = *static_pointer_cast<SLRenderer>(renderers[0]);
		int nonTrModels = 0;

		for (const bpScene::Model& model : scene.models)
		{
			bool hasTr = model.getMaterial(0).hasTransparency();
			SLRenderer& renderer = hasTr ?
				mainr : *static_pointer_cast<SLRenderer>(renderers[(nonTrModels++ % (renderers.size() - 1)) + 1]);
			auto& rm = renderer.getResourceManager();
			unsigned modelId = rm.addModel(model);
			rm.addModelInstance(modelId, scene.node);
		}
	} else
	{
		throw runtime_error("Unable to distribute the provided geometry");
	}
}
