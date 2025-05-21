#pragma once

#include <native_window/external_window.h>
#include <ace/xcomponent/native_interface_xcomponent.h>

#include "Common/GPU/thin3d.h"
#include "Common/GraphicsContext.h"

enum {
	OHOS_VERSION_GINGERBREAD = 9,
	OHOS_VERSION_ICS = 14,
	OHOS_VERSION_JELLYBEAN = 16,
	OHOS_VERSION_KITKAT = 19,
	OHOS_VERSION_LOLLIPOP = 21,
	OHOS_VERSION_MARSHMALLOW = 23,
	OHOS_VERSION_NOUGAT = 24,
	OHOS_VERSION_NOUGAT_1 = 25,
};

enum class GraphicsContextState {
	PENDING,
	INITIALIZED,
	FAILED_INIT,
	SHUTDOWN,
};

class OHOSGraphicsContext : public GraphicsContext {
public:
	// This is different than the base class function since on
	// OHOS (EGL, Vulkan) we do have all this info on the render thread.
	virtual bool InitFromRenderThread(void *wnd, int desiredBackbufferSizeX, int desiredBackbufferSizeY, int backbufferFormat, int androidVersion) = 0;
	virtual void BeginAndroidShutdown() {}
    virtual bool initializeEgl(void *wnd, int width, int height) {
		return true;
	}
	virtual GraphicsContextState GetState() const { return state_; }
    virtual void SwapBuffers() {};

protected:
	GraphicsContextState state_ = GraphicsContextState::PENDING;

private:
	using GraphicsContext::InitFromRenderThread;
};
