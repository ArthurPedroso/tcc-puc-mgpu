#include "Vmgpu.h"
#include <set>
#include <stdexcept>
#include <iostream>
#include <QMouseEvent>
#include <QKeyEvent>
#include <glm/gtx/component_wise.hpp>

using namespace bp;
using namespace std;

void Vmgpu::initRenderResources(uint32_t width, uint32_t height)
{
	//Select devices to use for rendering

	//First device both renders and takes care of compositing
	//Using device provided by the window
	devices.emplace_back(&device, [](Device*){});

	//Select secondary devices if requested
	if (options.deviceCount > 1)
	{
		DeviceRequirements requirements;
		requirements.queues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
		requirements.features.samplerAnisotropy = VK_TRUE;
		requirements.features.geometryShader = VK_TRUE;

		auto physicalDevices = bpQt::queryDevices(*vulkanInstance(), requirements);
		set<VkPhysicalDevice> selected;
		selected.insert(device);
		for (VkPhysicalDevice d : physicalDevices)
		{
			if (selected.size() == options.deviceCount) break;
			if (selected.find(d) == selected.end()) selected.insert(d);
		}

		if (selected.size() < options.deviceCount && !options.simulateMultiGPU)
			qWarning() << options.deviceCount << " devices requested, but only "
				   << selected.size() << " devices are available.";

		selected.erase(device);
		for (VkPhysicalDevice d : selected)
			devices.emplace_back(new Device(d, requirements));

		if (options.simulateMultiGPU)
		{
			unsigned deviceCount = static_cast<unsigned>(devices.size());
			for (unsigned i = deviceCount; i < options.deviceCount; i++)
				devices.emplace_back(new Device(*devices[i % deviceCount],
								requirements));
		}
	}

	cout << "GPUs:" << endl;
	for (unsigned i = 0; i < devices.size(); i++)
	{
		const auto& props = devices[i]->getProperties();
		cout << "GPU" << i << ": " << props.deviceName << '[' << props.vendorID << ':'
		     << props.deviceID << ']' << endl;
	}

	bpUtil::connect(scene.loadMessageEvent, loadMessageEvent);
	scene.load(options);
	float sceneSize = glm::compMax(scene.maxVertex - scene.minVertex);
	cameraFar = sceneSize * 3.f;
	float cameraTranslate = 2.f * (options.zUp ? -scene.minVertex.y : scene.maxVertex.z);
	cameraNode.translate(0.f, 0.f, cameraTranslate);
	camera.setPerspectiveProjection(glm::radians(60.f),
					static_cast<float>(width) / static_cast<float>(height),
					0.1f, cameraFar);
	cameraController.setCameraNode(cameraNode);
	cameraController.setSpeed(sceneSize / 2.f);
	cameraController.update(0.f);
	cameraNode.update();
	camera.update();

	switch (options.strategy)
	{
	case Strategy::Single:
	case Strategy::SortFirst: initSortFirst(width, height); break;
	case Strategy::SortLast: initSortLast(width, height); break;
	}

	depthAttachment.init(device, VK_FORMAT_D16_UNORM,
			     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, width, height);
	depthAttachment.setClearValue({1.f, 0.f});
	framebuffer.setAttachment(mainRenderer->getColorAttachmentSlot(), swapchain);
	framebuffer.setAttachment(mainRenderer->getDepthAttachmentSlot(), depthAttachment);
	framebuffer.init(mainRenderer->getRenderPass(), width, height);

	cout << "Rendering..." << endl;
}

void Vmgpu::resizeRenderResources(uint32_t width, uint32_t height)
{
	mainRenderer->resize(width, height);
	depthAttachment.resize(width, height);
	framebuffer.resize(width, height);
	camera.setPerspectiveProjection(glm::radians(60.f),
					static_cast<float>(width) / static_cast<float>(height),
					0.1f, cameraFar);
}

void Vmgpu::specifyDeviceRequirements(DeviceRequirements& requirements)
{
	requirements.features.samplerAnisotropy = VK_TRUE;
	requirements.features.geometryShader = VK_TRUE;
}

void Vmgpu::render(VkCommandBuffer cmdBuffer)
{
	//redistributeGeometry();
	mainRenderer->render(framebuffer, cmdBuffer);
}

void Vmgpu::update(double frameDeltaTime)
{
	cameraController.update(static_cast<float>(frameDeltaTime));
	camera.update();
	if (rotate)
	{
		scene.root.rotate(static_cast<float>(frameDeltaTime), {0.f, 1.f, 0.f});
		scene.root.update();
	}
}

void Vmgpu::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		mouseButton = true;
		previousMousePos = event->globalPos();
	}
	QWindow::mousePressEvent(event);
}

void Vmgpu::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		mouseButton = false;
		QPoint delta = event->globalPos() - previousMousePos;
		cameraController.motion(static_cast<float>(delta.x()),
					static_cast<float>(delta.y()));
		previousMousePos = event->globalPos();
	}
	QWindow::mouseReleaseEvent(event);
}

void Vmgpu::mouseMoveEvent(QMouseEvent* event)
{
	if (mouseButton)
	{
		QPoint delta = event->globalPos() - previousMousePos;
		cameraController.motion(static_cast<float>(delta.x()),
					static_cast<float>(delta.y()));
		previousMousePos = event->globalPos();
	}
	QWindow::mouseMoveEvent(event);
}

void Vmgpu::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Q: cameraController.setDownPressed(true); break;
	case Qt::Key_W: cameraController.setForwardPressed(true); break;
	case Qt::Key_E: cameraController.setUpPressed(true); break;
	case Qt::Key_A: cameraController.setLeftPressed(true); break;
	case Qt::Key_S: cameraController.setBackwardPressed(true); break;
	case Qt::Key_D: cameraController.setRightPressed(true); break;
	case Qt::Key_Space: rotate = !rotate;
	default: break;
	}
	QWindow::keyPressEvent(event);
}

void Vmgpu::keyReleaseEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Q: cameraController.setDownPressed(false); break;
	case Qt::Key_W: cameraController.setForwardPressed(false); break;
	case Qt::Key_E: cameraController.setUpPressed(false); break;
	case Qt::Key_A: cameraController.setLeftPressed(false); break;
	case Qt::Key_S: cameraController.setBackwardPressed(false); break;
	case Qt::Key_D: cameraController.setRightPressed(false); break;
	default: break;
	}
	QWindow::keyReleaseEvent(event);
}
