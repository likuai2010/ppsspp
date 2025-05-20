#pragma once

#include "OHOSGraphicsContext.h"
#include "Common/GPU/OpenGL/GLRenderManager.h"
#include "Common/GPU/thin3d_create.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

class OHOSOpenEGLGraphicsContext : public OHOSGraphicsContext {
public:
	OHOSOpenEGLGraphicsContext();
	~OHOSOpenEGLGraphicsContext() { delete draw_; }

	// This performs the actual initialization,
	bool InitFromRenderThread(void *wnd, int desiredBackbufferSizeX, int desiredBackbufferSizeY, int backbufferFormat, int androidVersion) override;

	void ShutdownFromRenderThread() override;
    bool initializeEgl(void *wnd, int width, int height) override;
    bool CreateEnvironment();
	void Shutdown() override {}
	void Resize() override {}

//     bool Initialized() override {
// 		return draw_ != nullptr;
// 	}
	Draw::DrawContext *GetDrawContext() override {
		return draw_;
	}

	void ThreadStart() override {
		renderManager_->ThreadStart(draw_);
	}

	bool ThreadFrame() override {
		return renderManager_->ThreadFrame();
	}

	void BeginAndroidShutdown() override {
		renderManager_->SetSkipGLCalls();
	}

	void ThreadEnd() override {
		renderManager_->ThreadEnd();
	}

	void StopThread() override {
		renderManager_->StopThread();
	}
    
    void SwapBuffers() override;

private:
    EGLNativeWindowType m_eglWindow;
    EGLConfig m_eglConfig = EGL_NO_CONFIG_KHR;
    EGLSurface m_eglSurface = EGL_NO_SURFACE;
    EGLContext m_eglContext = EGL_NO_CONTEXT;
    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
	Draw::DrawContext *draw_ = nullptr;
	GLRenderManager *renderManager_ = nullptr;
};

