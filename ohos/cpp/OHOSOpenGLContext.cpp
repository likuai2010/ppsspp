#include "OHOSOpenGLContext.h"
#include "Common/System/Display.h"

#include "Common/Log.h"
#include "Core/Config.h"
#include "Core/ConfigValues.h"
#include "Core/System.h"
#include "GPU/OpenGL/GLFeatures.h"


const int EGL_RED_SIZE_DEFAULT = 8;
const int EGL_GREEN_SIZE_DEFAULT = 8;
const int EGL_BLUE_SIZE_DEFAULT = 8;
const int EGL_ALPHA_SIZE_DEFAULT = 8;

/**
 * Context attributes.
 */
const EGLint CONTEXT_ATTRIBS[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
};

const EGLint ATTRIB_LIST[] = {
    // Key,value.
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RED_SIZE, EGL_RED_SIZE_DEFAULT,
    EGL_GREEN_SIZE, EGL_GREEN_SIZE_DEFAULT,
    EGL_BLUE_SIZE, EGL_BLUE_SIZE_DEFAULT,
    EGL_ALPHA_SIZE, EGL_ALPHA_SIZE_DEFAULT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    // End.
    EGL_NONE
};

OHOSOpenEGLGraphicsContext::OHOSOpenEGLGraphicsContext() {
	SetGPUBackend(GPUBackend::OPENGL);
}
void OHOSOpenEGLGraphicsContext::SwapBuffers(){
    eglSwapBuffers(m_eglDisplay, m_eglSurface);
}

bool OHOSOpenEGLGraphicsContext::CreateEnvironment()
{
 
    m_eglSurface = eglCreateWindowSurface(m_eglDisplay, m_eglConfig, m_eglWindow, NULL);

    if (nullptr == m_eglSurface) {
         ERROR_LOG(G3D,
            "eglCreateWindowSurface: unable to create surface");
        return false;
    }

    // Create context.
    m_eglContext = eglCreateContext(m_eglDisplay, m_eglConfig, EGL_NO_CONTEXT, CONTEXT_ATTRIBS);
    if (!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext)) {
         ERROR_LOG(G3D,"eglMakeCurrent failed");
        return false;
    }

    return true;
}


bool OHOSOpenEGLGraphicsContext::initializeEgl(void *wnd, int width, int height){
    INFO_LOG(G3D, "EglContextInit execute");
    if ((nullptr == wnd) || (0 >= width) || (0 >= height)) {
        INFO_LOG(G3D,  "EglContextInit: param error");
        return false;
    }

//     m_width = width;
//     m_height = height;
//     if (0 < m_width) {
//         m_widthPercent = FIFTY_PERCENT * m_height / m_width;
//     }
    m_eglWindow = reinterpret_cast<EGLNativeWindowType>(wnd);

    // Init display.
    m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == m_eglDisplay) {
        ERROR_LOG(G3D, "eglGetDisplay: unable to get EGL display");
        return false;
    }

    EGLint majorVersion;
    EGLint minorVersion;
    if (!eglInitialize(m_eglDisplay, &majorVersion, &minorVersion)) {
       ERROR_LOG(G3D,
            "eglInitialize: unable to get initialize EGL display");
        return false;
    }

    // Select configuration.
    const EGLint maxConfigSize = 1;
    EGLint numConfigs;
    if (!eglChooseConfig(m_eglDisplay, ATTRIB_LIST, &m_eglConfig, maxConfigSize, &numConfigs)) {
        ERROR_LOG(G3D, "eglChooseConfig: unable to choose configs");
        return false;
    }
    return CreateEnvironment();
}
bool OHOSOpenEGLGraphicsContext::InitFromRenderThread(void *wnd, int desiredBackbufferSizeX, int desiredBackbufferSizeY, int backbufferFormat, int androidVersion) {
	
    initializeEgl(wnd, 1170, 1170);
    
    INFO_LOG(G3D, "OHOSOpenGLGraphicsContext::InitFromRenderThread");
	if (!CheckGLExtensions()) {
		ERROR_LOG(G3D, "CheckGLExtensions failed - not gonna attempt starting up.");
		state_ = GraphicsContextState::FAILED_INIT;
		return false;
	}
	// OpenGL handles rotated rendering in the driver.
	g_display.rotation = DisplayRotation::ROTATE_0;
	g_display.rot_matrix.setIdentity();

	draw_ = Draw::T3DCreateGLContext(false);  // Can't fail
	renderManager_ = (GLRenderManager *)draw_->GetNativeObject(Draw::NativeObject::RENDER_MANAGER);
	renderManager_->SetInflightFrames(g_Config.iInflightFrames);

	if (!draw_->CreatePresets()) {
		// This can't really happen now that compilation is async - they're only really queued for compile here.
		_assert_msg_(false, "Failed to compile preset shaders");
		state_ = GraphicsContextState::FAILED_INIT;
		return false;
	}
	state_ = GraphicsContextState::INITIALIZED;
	return true;
}

void OHOSOpenEGLGraphicsContext::ShutdownFromRenderThread() {
	INFO_LOG(G3D, "OHOSOpenGLGraphicsContext::Shutdown");
	renderManager_ = nullptr;  // owned by draw_.
	delete draw_;
	draw_ = nullptr;
	state_ = GraphicsContextState::SHUTDOWN;
}
