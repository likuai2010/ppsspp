#pragma once

#include "OHOSGraphicsContext.h"

class VulkanContext;

class OHOSVulkanContext : public OHOSGraphicsContext {
public:
	OHOSVulkanContext();
	~OHOSVulkanContext();

	bool InitAPI();

	bool InitFromRenderThread(void *wnd, int desiredBackbufferSizeX, int desiredBackbufferSizeY, int backbufferFormat, int ohosVersion) override;
	void ShutdownFromRenderThread() override;  // Inverses InitFromRenderThread.

	void Shutdown() override;
	void Resize() override;

	void *GetAPIContext() override { return g_Vulkan; }
	Draw::DrawContext *GetDrawContext() override { return draw_; }

private:
	VulkanContext *g_Vulkan = nullptr;
	Draw::DrawContext *draw_ = nullptr;
};
