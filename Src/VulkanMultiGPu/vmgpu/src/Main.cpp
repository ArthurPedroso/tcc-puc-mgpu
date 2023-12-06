#include "Options.h"
#include "Vmgpu.h"
#include <QGuiApplication>
#include <QVulkanInstance>
#include <QLoggingCategory>
#include <bpScene/Mesh.h>
#include <iostream>
#include <iomanip>

#define DEBUG_SHADERS

using namespace std;
using bpScene::Mesh;

int main(int argc, char** argv)
{
	QGuiApplication app{argc, argv};

	Options options;
	try { options = parseOptions(argc, argv); } catch (int e) { return e; }
	QVulkanInstance instance;
	instance.setApiVersion(QVersionNumber(1, 3));

#ifndef DEBUG
	QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));
#ifdef DEBUG_SHADERS
	instance.setLayers(QByteArrayList() << "VK_DBG_LAYER_ACTION_LOG_MSG");
#else
	instance.setLayers(QByteArrayList() << "VK_LAYER_LUNARG_standard_validation");
#endif
#endif

	if (!instance.create())
		qFatal("Failed to create instance.");

	Vmgpu vmgpu{instance, options};
	vmgpu.setVSync(false);
	bpUtil::connect(vmgpu.framerateEvent, [](float fps){
		cout << "\rFramerate = " << setprecision(4) << fps << " FPS" << flush;
	});
	bpUtil::connect(vmgpu.loadMessageEvent, [](const string& msg){
		cout << msg << endl;
	});
	vmgpu.resize(options.width, options.height);
	vmgpu.show();

	return app.exec();
}
