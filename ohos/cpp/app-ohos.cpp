// This is generic code that is included in all Android apps that use the
// Native framework by Henrik Rydgard (https://github.com/hrydgard/native).

// It calls a set of methods defined in NativeApp.h. These should be implemented
// by your game or app.

#include <cstdlib>
#include <cstdint>

#include "app-ohos.h"
#include "Core/PSPLoaders.h"
#include "napi-utils.h"
#include "File/VFS/DirectoryReader.h"
#include "OHOSGraphicsContext.h"
#include <sstream>
#include <queue>
#include <mutex>
#include <sys/mman.h>
#include <thread>
#include <atomic>

#include "System/Display.h"
#include "System/Request.h"
#include "TimeUtil.h"
#include "ohos/cpp/OHOSAudio.h"
#include "ohos/cpp/ohos-log.h"

#include "Common/Log.h"
#include "Common/LogReporting.h"

#include "Common/Net/Resolve.h"
#include "Common/GPU/OpenGL/GLFeatures.h"

#include "Common/System/NativeApp.h"
#include "Common/System/System.h"
#include "Common/Thread/ThreadUtil.h"
#include "Common/File/Path.h"
#include "Common/File/DirListing.h"
#include "Common/File/VFS/VFS.h"
#include "Common/Input/InputState.h"
#include "Common/Data/Text/Parsers.h"
#include "Common/GPU/Vulkan/VulkanLoader.h"

#include "Common/GraphicsContext.h"
#include "Common/StringUtils.h"
#include "Core/Config.h"
#include "Core/ConfigValues.h"
#include "Core/Loaders.h"
#include "Core/FileLoaders/LocalFileLoader.h"
#include "Core/KeyMap.h"
#include "Core/System.h"
#include "Core/HLE/sceUsbCam.h"
#include "Common/CPUDetect.h"
#include "Common/Log.h"
#include "ohos/cpp/OHOSOpenGLContext.h"
#include "ohos/cpp/OHOSVulkanContext.h"

// Need to use raw Android logging before NativeInit.
#define EARLY_LOG(...)  Hi(H, "PPSSPP", __VA_ARGS__)

bool useCPUThread = true;

enum class EmuThreadState {
	DISABLED,
	START_REQUESTED,
	RUNNING,
	QUIT_REQUESTED,
	STOPPED,
};

static std::thread emuThread;
static std::thread vulkanEmuThread;
static std::atomic<int> emuThreadState((int)EmuThreadState::DISABLED);

OHOSAudioState *g_audioState;

struct FrameCommand {
	FrameCommand() {}
	FrameCommand(std::string cmd, std::string prm) : command(cmd), params(prm) {}

	std::string command;
	std::string params;
};

static std::mutex frameCommandLock;
static std::queue<FrameCommand> frameCommands;

static std::string systemName;
static std::string langRegion;
static std::string mogaVersion;
static std::string boardName;

std::string g_externalDir;  // Original external dir (root of Android storage).
std::string g_extFilesDir;  // App private external dir.
std::string g_nativeLibDir;  // App native library dir

static std::vector<std::string> g_additionalStorageDirs;

static int optimalFramesPerBuffer = 0;
static int optimalSampleRate = 0;
static int sampleRate = 0;
static int framesPerBuffer = 0;
static int apiVersion;
static int deviceType;

// Should only be used for display detection during startup (for config defaults etc)
// This is the ACTUAL display size, not the hardware scaled display size.
// Exposed so it can be displayed on the touchscreen test.
static int display_xres;
static int display_yres;
static int display_dpi;
static float display_scale;
static int backbuffer_format;	// Android PixelFormat enum

static int desiredBackbufferSizeX;
static int desiredBackbufferSizeY;

static float g_safeInsetLeft = 0.0;
static float g_safeInsetRight = 0.0;
static float g_safeInsetTop = 0.0;
static float g_safeInsetBottom = 0.0;


static std::atomic<bool> exitRenderLoop;
static std::atomic<bool> renderLoopRunning;
static bool renderer_inited = false;
static std::mutex renderLock;

static bool sustainedPerfSupported = false;

static std::map<SystemPermission, PermissionStatus> permissions;


#ifndef LOG_APP_NAME
#define LOG_APP_NAME "PPSSPP"
#endif

#define MessageBox(a, b, c, d) OH_LOG_Print(LOG_APP_NAME, level, LOG_DOMAIN, "EglCore","%{public}s %s",(b), (c));

bool canJit(){
    unsigned char code[] = {
        0xc0, 0x03, 0x5f, 0xd6      // ret                    (返回)
    };
    size_t code_size = sizeof(code);
    void *mem = mmap(NULL, code_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        return false;
    }
    memcpy(mem, code, code_size);
    if (mprotect(mem, code_size, PROT_READ | PROT_EXEC) == -1) {
        munmap(code, code_size);
        return false;
    } else {
       return true;
    }
}


static OHOSGraphicsContext *graphicsContext;


static  napi_threadsafe_function postCommand_tsfn;
static  FrameCommand FrameCommandInfo;

static void ProcessFrameCommands() {
	napi_call_threadsafe_function(postCommand_tsfn, &FrameCommandInfo, napi_tsfn_nonblocking);
	
}
static void CallPostCommand(napi_env env, napi_value jsCb, void *context, void *data){
    napi_value params[2];
	while (!frameCommands.empty()) {
		if(!frameCommands.empty()){
			FrameCommand frameCmd = frameCommands.front();
			INFO_LOG(Log::System, "frameCommand '%s' '%s'", frameCmd.command.c_str(), frameCmd.params.c_str());
			napi_create_string_utf8(env, frameCmd.command.c_str(), frameCmd.command.size(), &params[0]);
			napi_create_string_utf8(env, frameCmd.params.c_str(), frameCmd.params.size(), &params[1]);
			napi_call_function(env, nullptr, jsCb, 2, params, nullptr);
			frameCommands.pop();
		}
	}
}

static napi_value onPostCommand(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value jsCb = nullptr;
    napi_get_cb_info(env, info, &argc, &jsCb, nullptr, nullptr);
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, " postCommand", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_threadsafe_function(env, jsCb, nullptr, resourceName, 0, 1, nullptr, nullptr, nullptr, CallPostCommand, &postCommand_tsfn);
    return nullptr;
}

static void recalculateDpi() {
	
	int old_w = g_display.pixel_xres;
	int old_h = g_display.pixel_yres;

	// pixel_*res is the backbuffer resolution.
	//backbuffer_format = format;
//	if (IsVREnabled()) {
//		GetVRResolutionPerEye(&pixel_xres, &pixel_yres);
//	}
	// Compute display scale factor. Always < 1.0f (well, as long as we use buffers sized smaller than the screen...)
	display_scale = (float)display_xres / (float)display_xres;
	float dpi = (1.0f / display_scale) * (240.0f / (float)display_dpi);
	bool new_size = g_display.Recalculate(display_xres, display_yres, dpi, UIScaleFactorToMultiplier(g_Config.iUIScaleFactor));
	
	INFO_LOG(Log::G3D, "RecalcDPI: display_xres=%d display_yres=%d pixel_xres=%d pixel_yres=%d", display_xres, display_yres, g_display.pixel_xres, g_display.pixel_yres);
	INFO_LOG(Log::G3D, "RecalcDPI: g_dpi=%d scaled_dpi=%f display_scale=%f g_dpi_scale=%f dp_xres=%d dp_yres=%d", display_dpi, dpi, display_scale, g_display.dpi_scale, g_display.dp_xres, g_display.dp_yres);
	if (new_size) {
		INFO_LOG(Log::G3D, "Size change detected (previously %d,%d) - calling NativeResized()", old_w, old_h);
		NativeResized();
	} else {
		INFO_LOG(Log::G3D, "NativeApp::backbufferResize: Size didn't change.");
	}
}

static int refreshRate;
static double displayDpi;
void setDisplayParameters(int xres, int yres){
   
    INFO_LOG(Log::G3D, "NativeApp.setDisplayParameters(%d x %d, dpi=%d, refresh=%0.2f)", xres, yres, displayDpi, refreshRate);
    bool changed = false;
	changed = changed || display_xres != xres || display_yres != yres;
	changed = changed || display_dpi != displayDpi || display_dpi != displayDpi;
	changed = changed || g_display.display_hz != refreshRate;
	if (changed) {
		display_xres = xres;
		display_yres = yres;
		display_dpi = displayDpi;
		g_display.display_hz = refreshRate;
		recalculateDpi();
		NativeResized();
	}
}
void backbufferResize(int bufw, int bufh, int format){
    INFO_LOG(Log::System, "NativeApp.backbufferResize(%d x %d)", bufw, bufh);

	bool new_size = g_display.pixel_xres != bufw || g_display.pixel_yres != bufh;
	int old_w = g_display.pixel_xres;
	int old_h = g_display.pixel_yres;
	// pixel_*res is the backbuffer resolution.
	g_display.pixel_xres = bufw;
	g_display.pixel_yres = bufh;
	backbuffer_format = format;

	recalculateDpi();

	if (new_size) {
		INFO_LOG(Log::G3D, "Size change detected (previously %d,%d) - calling NativeResized()", old_w, old_h);
		NativeResized();
	} else {
		INFO_LOG(Log::G3D, "NativeApp::backbufferResize: Size didn't change.");
	}
}


// Only used in OpenGL mode.
static void EmuThreadFunc(napi_env env) {
	SetCurrentThreadName("EmuThread");
    INFO_LOG(Log::System, "Entering emu thread");
    // Wait for render loop to get started.
    INFO_LOG(Log::System, "Runloop: Waiting for displayInit...");
	while (!graphicsContext || graphicsContext->GetState() == GraphicsContextState::PENDING) {
		sleep(1);
	}
    // Check the state of the graphics context before we try to feed it into NativeInitGraphics.
    if (graphicsContext->GetState() != GraphicsContextState::INITIALIZED) {
        ERROR_LOG(Log::G3D, "Failed to initialize the graphics context! %d", (int)graphicsContext->GetState());
        emuThreadState = (int)EmuThreadState::QUIT_REQUESTED;
        return;
    }
    
    if (!NativeInitGraphics(graphicsContext)) {
        _assert_msg_(false, "NativeInitGraphics failed, might as well bail");
        emuThreadState = (int)EmuThreadState::QUIT_REQUESTED;
        return;
    }
    INFO_LOG(Log::System, "Graphics initialized. Entering loop.");
    
    // There's no real requirement that NativeInit happen on this thread.
    // We just call the update/render loop here.
    emuThreadState = (int)EmuThreadState::RUNNING;
    while (emuThreadState != (int)EmuThreadState::QUIT_REQUESTED) {
        {
            std::lock_guard<std::mutex> renderGuard(renderLock);
            NativeFrame(graphicsContext);
        }
        std::lock_guard<std::mutex> guard(frameCommandLock);
        ProcessFrameCommands();
    }
    
    INFO_LOG(Log::System, "QUIT_REQUESTED found, left EmuThreadFunc loop. Setting state to STOPPED.");
    emuThreadState = (int)EmuThreadState::STOPPED;
    NativeShutdownGraphics();
    // Also ask the main thread to stop, so it doesn't hang waiting for a new frame.
    graphicsContext->StopThread();
    INFO_LOG(Log::System, "Leaving emu thread");
}

static void EmuThreadStart(napi_env env) {
	INFO_LOG(Log::System, "EmuThreadStart");
	emuThreadState = (int)EmuThreadState::START_REQUESTED;
	emuThread = std::thread(&EmuThreadFunc, env);
}



static void PushCommand(std::string cmd, std::string param) {
	std::lock_guard<std::mutex> guard(frameCommandLock);
	frameCommands.push(FrameCommand(cmd, param));
}


struct BridgeCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    void* window;
};

void EmuRenderThread(void *wnd){
    SetCurrentThreadName("EmuThread");
	if (!graphicsContext) {
		ERROR_LOG(Log::G3D, "runVulkanRenderLoop: Tried to enter without a created graphics context.");
		renderLoopRunning = false;
		exitRenderLoop = false;
		return;
	}
	if (exitRenderLoop) {
		WARN_LOG(Log::G3D, "runVulkanRenderLoop: ExitRenderLoop requested at start, skipping the whole thing.");
		renderLoopRunning = false;
		exitRenderLoop = false;
		return;
	}
	// This is up here to prevent race conditions, in case we pause during init.
	renderLoopRunning = true;

	WARN_LOG(Log::G3D, "runVulkanRenderLoop. display_xres=%d display_yres=%d desiredBackbufferSizeX=%d desiredBackbufferSizeY=%d",
		display_xres, display_yres, desiredBackbufferSizeX, desiredBackbufferSizeY);

	if (!graphicsContext->InitFromRenderThread(wnd, desiredBackbufferSizeX, desiredBackbufferSizeY, backbuffer_format, apiVersion)) {
		// On Android, if we get here, really no point in continuing.
		// The UI is supposed to render on any device both on OpenGL and Vulkan. If either of those don't work
		// on a device, we blacklist it. Hopefully we should have already failed in InitAPI anyway and reverted to GL back then.
		ERROR_LOG(Log::G3D, "Failed to initialize graphics context.");
		System_Toast("Failed to initialize graphics context.");

		delete graphicsContext;
		graphicsContext = nullptr;
		renderLoopRunning = false;
		return;
	}

	if (!exitRenderLoop) {
        if (!useCPUThread && !NativeInitGraphics(graphicsContext)) {
            ERROR_LOG(Log::G3D, "Failed to initialize graphics.");
            // Gonna be in a weird state here..
        }
		graphicsContext->ThreadStart();
		renderer_inited = true;
		while (!exitRenderLoop) {
			{

                if(useCPUThread){
                    graphicsContext->ThreadFrame();
                    graphicsContext->SwapBuffers();
                }else{
                    std::lock_guard<std::mutex> renderGuard(renderLock);
				    NativeFrame(graphicsContext);
                    {
                        std::lock_guard<std::mutex> guard(frameCommandLock);
                        ProcessFrameCommands();
			        }
                }
			}
			
		}
		INFO_LOG(Log::G3D, "Leaving Vulkan main loop.");
	} else {
		INFO_LOG(Log::G3D, "Not entering main loop.");
	}

	NativeShutdownGraphics();

	renderer_inited = false;
	graphicsContext->ThreadEnd();

	// Shut the graphics context down to the same state it was in when we entered the render thread.
	INFO_LOG(Log::G3D, "Shutting down graphics context...");
	graphicsContext->ShutdownFromRenderThread();
	renderLoopRunning = false;
	exitRenderLoop = false;

	WARN_LOG(Log::G3D, "Render loop function exited.");
}


void DisplayRender(void * window) {
    if(renderer_inited)
        return ;
    INFO_LOG(Log::G3D, "Starting Vulkan submission thread");
    g_Config.bRenderMultiThreading = true;
    vulkanEmuThread = std::thread(&EmuRenderThread, window);

}


// Call EmuThreadStop first, then keep running the GPU (or eat commands)
// as long as emuThreadState isn't STOPPED and/or there are still things queued up.
// Only after that, call EmuThreadJoin.
static void EmuThreadStop(const char *caller) {
	INFO_LOG(Log::System, "EmuThreadStop - stopping (%s)...", caller);
	emuThreadState = (int)EmuThreadState::QUIT_REQUESTED;
}

static void EmuThreadJoin() {
	emuThread.join();
	emuThread = std::thread();
	INFO_LOG(Log::System, "EmuThreadJoin - joined");
}



void System_ShowKeyboard() {
	PushCommand("showKeyboard", "");
}

void System_Vibrate(int length_ms) {
	char temp[32];
	snprintf(temp, sizeof(temp), "%d", length_ms);
	PushCommand("vibrate", temp);
}



std::string System_GetProperty(SystemProperty prop) {
	switch (prop) {
	case SYSPROP_NAME:
		return systemName;
	case SYSPROP_LANGREGION:	// "en_US"
		return langRegion;
	case SYSPROP_MOGA_VERSION:
		return mogaVersion;
	case SYSPROP_BOARDNAME:
		return boardName;
	case SYSPROP_BUILD_VERSION:
		return PPSSPP_GIT_VERSION;
	default:
		return "";
	}
}

std::vector<std::string> System_GetPropertyStringVec(SystemProperty prop) {
	switch (prop) {
	case SYSPROP_ADDITIONAL_STORAGE_DIRS:
		return g_additionalStorageDirs;

	case SYSPROP_TEMP_DIRS:
	default:
		return std::vector<std::string>();
	}
}

int64_t System_GetPropertyInt(SystemProperty prop) {
    switch (prop) {
    case SYSPROP_SYSTEMVERSION:
        return apiVersion;
    case SYSPROP_DEVICE_TYPE:
        return deviceType;
    case SYSPROP_DISPLAY_XRES:
        return display_xres;
    case SYSPROP_DISPLAY_YRES:
        return display_yres;
    case SYSPROP_AUDIO_SAMPLE_RATE:
        return sampleRate;
    case SYSPROP_AUDIO_FRAMES_PER_BUFFER:
        return framesPerBuffer;
    case SYSPROP_AUDIO_OPTIMAL_SAMPLE_RATE:
        return optimalSampleRate;
    case SYSPROP_AUDIO_OPTIMAL_FRAMES_PER_BUFFER:
        return optimalFramesPerBuffer;
    default:
        return -1;
    }
}


// Easy way for the Java side to ask the C++ side for configuration options, such as
// the rotation lock which must be controlled from Java on Android.
static std::string QueryConfig(std::string query) {
	char temp[128];
	if (query == "screenRotation") {
		INFO_LOG(Log::G3D, "g_Config.screenRotation = %d", g_Config.iScreenRotation);
		snprintf(temp, sizeof(temp), "%d", g_Config.iScreenRotation);
		return std::string(temp);
	} else if (query == "immersiveMode") {
		return std::string(g_Config.bImmersiveMode ? "1" : "0");
	} else if (query == "sustainedPerformanceMode") {
		return std::string(g_Config.bSustainedPerformanceMode ? "1" : "0");
	} else if (query == "androidJavaGL") {
		// If we're using Vulkan, we say no... need C++ to use Vulkan.
		if (GetGPUBackend() == GPUBackend::VULKAN) {
			return "false";
		}
		// Otherwise, some devices prefer the Java init so play it safe.
		return "true";
	} else {
		return "";
	}
}



static void parse_args(std::vector<std::string> &args, const std::string value) {
	// Simple argument parser so we can take args from extra params.
	const char *p = value.c_str();

	while (*p != '\0') {
		while (isspace(*p)) {
			p++;
		}
		if (*p == '\0') {
			break;
		}

		bool done = false;
		bool quote = false;
		std::string arg;

		while (!done) {
			size_t sz = strcspn(p, "\"\\ \r\n\t");
			arg += std::string(p, sz);
			p += sz;

			switch (*p) {
			case '"':
				quote = !quote;
				p++;
				break;

			case '\\':
				p++;
				arg += std::string(p, 1);
				p++;
				break;

			case '\0':
				done = true;
				break;

			default:
				// If it's not the above, it's whitespace.
				if (!quote) {
					done = true;
				} else {
					sz = strspn(p, " \r\n\t");
					arg += std::string(p, sz);
					p += sz;
				}
				break;
			}
		}

		args.push_back(arg);

		while (isspace(*p)) {
			p++;
		}
	}
}



void System_AskForPermission(SystemPermission permission) {
	switch (permission) {
	case SYSTEM_PERMISSION_STORAGE:
		PushCommand("ask_permission", "storage");
		break;
	}
}

PermissionStatus System_GetPermissionStatus(SystemPermission permission) {
	return PERMISSION_STATUS_GRANTED;
}
void correctRatio(int &sz_x, int &sz_y, float scale) {
	float x = (float)sz_x;
	float y = (float)sz_y;
	float ratio = x / y;
	INFO_LOG(Log::G3D, "CorrectRatio: Considering size: %0.2f/%0.2f=%0.2f for scale %f", x, y, ratio, scale);
	float targetRatio;

	// Try to get the longest dimension to match scale*PSP resolution.
	if (x >= y) {
		targetRatio = 480.0f / 272.0f;
		x = 480.f * scale;
		y = 272.f * scale;
	} else {
		targetRatio = 272.0f / 480.0f;
		x = 272.0f * scale;
		y = 480.0f * scale;
	}

	float correction = targetRatio / ratio;
	INFO_LOG(Log::G3D, "Target ratio: %0.2f ratio: %0.2f correction: %0.2f", targetRatio, ratio, correction);
	if (ratio < targetRatio) {
		y *= correction;
	} else {
		x /= correction;
	}

	sz_x = x;
	sz_y = y;
	INFO_LOG(Log::G3D, "Corrected ratio: %dx%d", sz_x, sz_y);
}

void getDesiredBackbufferSize(int &sz_x, int &sz_y) {
	sz_x = display_xres;
	sz_y = display_yres;

	int scale = g_Config.iAndroidHwScale;
	// Override hw scale for TV type devices.
	if (System_GetPropertyInt(SYSPROP_DEVICE_TYPE) == DEVICE_TYPE_TV)
		scale = 0;

	if (scale == 1) {
		// If g_Config.iInternalResolution is also set to Auto (1), we fall back to "Device resolution" (0). It works out.
		scale = g_Config.iInternalResolution;
	} else if (scale >= 2) {
		scale -= 1;
	}

	int max_res = std::max(System_GetPropertyInt(SYSPROP_DISPLAY_XRES), System_GetPropertyInt(SYSPROP_DISPLAY_YRES)) / 480 + 1;

	scale = std::min(scale, max_res);

	if (scale > 0) {
		correctRatio(sz_x, sz_y, scale);
	} else {
		sz_x = 0;
		sz_y = 0;
	}
}

std::vector<std::string> System_GetCameraDeviceList() {
	std::vector<std::string> deviceListVector;
	return deviceListVector;
}


napi_value audioInit(napi_env env, napi_callback_info info);



napi_value Native_Init(napi_env env, napi_callback_info info)
{
 	audioInit(env, info);
	GET_NAPI_ARGS(env, info, 3);
  	GET_NAPI_ARG(std::string, resDir, env, args[0]);
	GET_NAPI_ARG(std::string, filesDir, env, args[1]);
	GET_NAPI_ARG(std::string, cacheDir, env, args[2]);
    DEBUG_LOG(Log::System, "NativeApp.init() -- begin");
    std::lock_guard<std::mutex> guard(renderLock);
	renderer_inited = false;
	exitRenderLoop = false;
	apiVersion = 18;
	deviceType = DEVICE_TYPE_MOBILE;
    
    Path assetsPath(resDir + "/assets");
    g_VFS.Register("", new DirectoryReader(assetsPath));
    systemName = "HarmonyOS Next";
	langRegion = "CN";
  	DEBUG_LOG(Log::System, "NativeApp.init(): device name: '%s'", systemName.c_str());
    
    std::string externalStorageDir = resDir;
	std::string additionalStorageDirsString = "";
	std::string externalFilesDir = "";
	std::string nativeLibDir = "";

	g_externalDir = externalStorageDir;
	g_extFilesDir = externalFilesDir;
	g_nativeLibDir = nativeLibDir;
    
    std::string user_data_path = filesDir;
   
    if (user_data_path.size() > 0)
        user_data_path += "/";
    std::string shortcut_param = ""; 
    
    std::string app_name;
    std::string app_nice_name;
    std::string version;
    bool landscape;
    NativeGetAppInfo(&app_name, &app_nice_name, &landscape, &version);
    
    
    std::vector<const char *> cargs;
    std::vector<std::string> temp;
    cargs.push_back(app_name.c_str());
    if (!shortcut_param.empty()) {
		DEBUG_LOG(Log::System, "NativeInit shortcut param %s", shortcut_param.c_str());
		parse_args(temp, shortcut_param);
		for (const auto &arg : temp) {
			cargs.push_back(arg.c_str());
		}
	}
    NativeInit((int)cargs.size(), &cargs[0], user_data_path.c_str(), externalStorageDir.c_str(), cacheDir.c_str());
    // In debug mode, don't allow creating software Vulkan devices (reject by VulkanMaybeAvailable).
	// Needed for #16931.
    #ifdef NDEBUG
        if (!VulkanMayBeAvailable()) {
            // If VulkanLoader decided on no viable backend, let's force Vulkan off in release builds at least.
            g_Config.iGPUBackend = 0;
        }
    #endif
   
    retry:
    	switch (g_Config.iGPUBackend) {
    	case (int)GPUBackend::OPENGL:
    		useCPUThread = true;
    		INFO_LOG(Log::System, "NativeApp.init() -- creating OpenGL context (EGL)");
    		graphicsContext = new OHOSOpenEGLGraphicsContext();
    		INFO_LOG(Log::System, "NativeApp.init() - launching emu thread");
    		EmuThreadStart(env);
    		break;
    	case (int)GPUBackend::VULKAN:
    	{
    		INFO_LOG(Log::System, "NativeApp.init() -- creating Vulkan context");
    		useCPUThread = false;
    		// The Vulkan render manager manages its own thread.
    		// We create and destroy the Vulkan graphics context in the app main thread though.
    		OHOSVulkanContext *ctx = new OHOSVulkanContext();
    		if (!ctx->InitAPI()) {
    			INFO_LOG(Log::System, "Failed to initialize Vulkan, switching to OpenGL");
    			g_Config.iGPUBackend = (int)GPUBackend::OPENGL;
    			SetGPUBackend(GPUBackend::OPENGL);
    			goto retry;
    		} else {
    			graphicsContext = ctx;
    		}
    		break;
    	}
    	default:
    		ERROR_LOG(Log::System, "NativeApp.init(): iGPUBackend %d not supported. Switching to OpenGL.", (int)g_Config.iGPUBackend);
    		g_Config.iGPUBackend = (int)GPUBackend::OPENGL;
    		goto retry;
    	}
    return nullptr;
}



void System_LaunchUrl(LaunchUrlType urlType, const char *url) {
    switch (urlType) {
    case LaunchUrlType::BROWSER_URL:
        PushCommand("launchBrowser", url);
        break;
    case LaunchUrlType::MARKET_URL:
        PushCommand("launchMarket", url);
        break;
    case LaunchUrlType::EMAIL_ADDRESS:
        PushCommand("launchEmail", url);
        break;
    }
}

// OHOS implementation of callbacks to the js part of the app
void System_Toast(std::string_view text) { PushCommand("toast", std::string(text)); }


bool System_MakeRequest(SystemRequestType type, int requestId, const std::string &param1, const std::string &param2,
                        int64_t param3, int64_t param4) {
    switch (type) {
    case SystemRequestType::EXIT_APP:
        PushCommand("finish", "");
        return true;
    case SystemRequestType::RESTART_APP:
        PushCommand("restart", param1);
        return true;
    case SystemRequestType::RECREATE_ACTIVITY:
        PushCommand("recreate", param1);
        return true;
    case SystemRequestType::INPUT_TEXT_MODAL: {
        std::string serialized = StringFromFormat("%d:@:%s:@:%s", requestId, param1.c_str(), param2.c_str());
        PushCommand("inputbox", serialized.c_str());
        return true;
    }
    case SystemRequestType::BROWSE_FOR_IMAGE:
        PushCommand("browse_image", StringFromFormat("%d", requestId));
        return true;
    case SystemRequestType::BROWSE_FOR_FILE: {
        BrowseFileType fileType = (BrowseFileType)param3;
        std::string params = StringFromFormat("%d", requestId);
        switch (fileType) {
        case BrowseFileType::SOUND_EFFECT:
            PushCommand("browse_file_audio", params);
            break;
        case BrowseFileType::ZIP:
            PushCommand("browse_file_zip", params);
            break;
        default:
            PushCommand("browse_file", params);
            break;
        }
        return true;
    }
    case SystemRequestType::BROWSE_FOR_FOLDER:
        PushCommand("browse_folder", StringFromFormat("%d", requestId));
        return true;

    case SystemRequestType::CAMERA_COMMAND:
        PushCommand("camera_command", param1);
        return true;
    case SystemRequestType::GPS_COMMAND:
        PushCommand("gps_command", param1);
        return true;
    case SystemRequestType::INFRARED_COMMAND:
        PushCommand("infrared_command", param1);
        return true;
    case SystemRequestType::MICROPHONE_COMMAND:
        PushCommand("microphone_command", param1);
        return true;
    case SystemRequestType::SHARE_TEXT:
        PushCommand("share_text", param1);
        return true;
    case SystemRequestType::SET_KEEP_SCREEN_BRIGHT:
        PushCommand("set_keep_screen_bright", param3 ? "on" : "off");
        return true;
    case SystemRequestType::SHOW_FILE_IN_FOLDER:
        PushCommand("show_folder", param1);
        return true;
    default:
        return false;
    }
}

void System_Notify(SystemNotification notification) {
    switch (notification) {
    case SystemNotification::ROTATE_UPDATED:
        PushCommand("rotate", "");
        break;
    case SystemNotification::FORCE_RECREATE_ACTIVITY:
        PushCommand("recreate", "");
        break;
    case SystemNotification::IMMERSIVE_MODE_CHANGE:
        PushCommand("immersive", "");
        break;
    case SystemNotification::SUSTAINED_PERF_CHANGE:
        PushCommand("sustainedPerfMode", "");
        break;
    case SystemNotification::TEST_JAVA_EXCEPTION:
        PushCommand("testException", "This is a test exception");
        break;
    default:
        break;
    }
}


float System_GetPropertyFloat(SystemProperty prop) {
    switch (prop) {
    case SYSPROP_DISPLAY_REFRESH_RATE:
        return g_display.display_hz;
    case SYSPROP_DISPLAY_SAFE_INSET_LEFT:
        return g_safeInsetLeft * display_scale * g_display.dpi_scale;
    case SYSPROP_DISPLAY_SAFE_INSET_RIGHT:
        return g_safeInsetRight * display_scale * g_display.dpi_scale;
    case SYSPROP_DISPLAY_SAFE_INSET_TOP:
        return g_safeInsetTop * display_scale * g_display.dpi_scale;
    case SYSPROP_DISPLAY_SAFE_INSET_BOTTOM:
        return g_safeInsetBottom * display_scale * g_display.dpi_scale;
    default:
        return -1;
    }
}

bool System_GetPropertyBool(SystemProperty prop) {
    switch (prop) {
    case SYSPROP_SUPPORTS_PERMISSIONS:
        return false;
    case SYSPROP_SUPPORTS_SUSTAINED_PERF_MODE:
        return sustainedPerfSupported; // 7.0 introduced sustained performance mode as an optional feature.
    case SYSPROP_HAS_TEXT_INPUT_DIALOG:
        return true;
    case SYSPROP_HAS_OPEN_DIRECTORY:
        return false; // We have this implemented but it may or may not work depending on if a file explorer is
                      // installed.
    case SYSPROP_HAS_ADDITIONAL_STORAGE:
        return !g_additionalStorageDirs.empty();
    case SYSPROP_HAS_BACK_BUTTON:
        return true;
    case SYSPROP_HAS_IMAGE_BROWSER:
        return deviceType != DEVICE_TYPE_VR;
    case SYSPROP_HAS_FILE_BROWSER:
        // It's only really needed with scoped storage, but why not make it available
        // as far back as possible - works just fine.
        return (apiVersion >= 9) && (deviceType != DEVICE_TYPE_VR); // when ACTION_OPEN_DOCUMENT was added
    case SYSPROP_HAS_FOLDER_BROWSER:
        // Uses OPEN_DOCUMENT_TREE to let you select a folder.
        // Doesn't actually mean it's usable though, in many early versions of Android
        // this dialog is complete garbage and only lets you select subfolders of the Downloads folder.
        return (apiVersion >= 9) && (deviceType != DEVICE_TYPE_VR); // when ACTION_OPEN_DOCUMENT_TREE was added
    case SYSPROP_SUPPORTS_OPEN_FILE_IN_EDITOR:
        return false; // Update if we add support in FileUtil.cpp: OpenFileInEditor
    case SYSPROP_APP_GOLD:
#ifdef GOLD
        return true;
#else
        return false;
#endif
    case SYSPROP_CAN_JIT:
        return canJit();// TODO 检查是否支持
    case SYSPROP_ANDROID_SCOPED_STORAGE:
         return true;
    case SYSPROP_HAS_KEYBOARD:
        return deviceType != DEVICE_TYPE_VR;
    case SYSPROP_HAS_ACCELEROMETER:
        return deviceType == DEVICE_TYPE_MOBILE;
    case SYSPROP_CAN_CREATE_SHORTCUT:
        return false; // We can't create shortcuts directly from game code, but we can from the Android UI.
#ifndef HTTPS_NOT_AVAILABLE
    case SYSPROP_SUPPORTS_HTTPS:
        return !g_Config.bDisableHTTPS;
#endif
    default:
        return false;
    }
}

void computeDesiredBackbufferDimensions(){
    getDesiredBackbufferSize(desiredBackbufferSizeX, desiredBackbufferSizeY);
}


static napi_value setDisplayParameters(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value params[4];
    napi_get_cb_info(env, info, &argc, params, nullptr, nullptr);
    
    int xres;
    int yres;
    napi_get_value_int32(env, params[0], &xres);
    napi_get_value_int32(env, params[1], &yres);
    napi_get_value_double(env, params[2], &displayDpi);
    napi_get_value_int32(env, params[3], &refreshRate);
    setDisplayParameters(xres, yres);
    return nullptr;
}
static napi_value backbufferResize(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value params[4];
    napi_get_cb_info(env, info, &argc, params, nullptr, nullptr);
    
    int bufw;
    int bufh;
    int format;
    napi_get_value_int32(env, params[0], &bufw);
    napi_get_value_int32(env, params[1], &bufh);
    napi_get_value_int32(env, params[3], &format);
    backbufferResize(bufw, bufh, format);
    return nullptr;
   
}


static napi_value queryConfig(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value params[1];
    napi_get_cb_info(env, info, &argc, params, nullptr, nullptr);
    size_t length;
    napi_get_value_string_utf8(env, params[0], nullptr, 0, &length);
    char *buf = (char *)malloc(length + 1);
    napi_get_value_string_utf8(env, params[0], buf, length + 1 , NULL);
    
    std::string result = QueryConfig(buf);
    napi_value jsResult;
    
    napi_create_string_utf8(env, result.c_str(), result.size(), &jsResult);
    return jsResult;
}
static napi_value sendMessage(napi_env env, napi_callback_info info){
	GET_NAPI_ARGS(env, info, 2)
	GET_NAPI_ARG(std::string, msg, env, args[0])
	GET_NAPI_ARG(std::string, prm, env, args[1])
    if (msg == "safe_insets") {
		INFO_LOG(Log::System, "Got insets: %s", prm.c_str());
		// We don't bother with supporting exact rectangular regions. Safe insets are good enough.
		int left, right, top, bottom;
		if (4 == sscanf(prm.c_str(), "%d:%d:%d:%d", &left, &right, &top, &bottom)) {
			g_safeInsetLeft = (float)left;
			g_safeInsetRight = (float)right;
			g_safeInsetTop = (float)top;
			g_safeInsetBottom = (float)bottom;
		}
		
	}
	 if (msg == "load_game") {
		INFO_LOG(Log::System, "load game: %s", prm.c_str());
		System_PostUIMessage(UIMessage::REQUEST_GAME_BOOT, prm.c_str());
	}
	
    return nullptr;
    
}
static napi_value sendRequestResult(napi_env env, napi_callback_info info) {
    
    size_t argc = 4;
    napi_value params[4];
    napi_get_cb_info(env, info, &argc, params, nullptr, nullptr);
    if (!renderer_inited) {
		return nullptr; 
	}
    
    size_t length;
    napi_get_value_string_utf8(env, params[2], nullptr, 0, &length);
    char *buf = (char *)malloc(length + 1);
    napi_get_value_string_utf8(env, params[2], buf, length + 1 , NULL);
    bool result;
	napi_get_value_bool(env, params[1], &result);
    int requestID;
    napi_get_value_int32(env, params[0], &requestID);
    if (result) {
		g_requestManager.PostSystemSuccess(requestID, buf);
	} else {
		g_requestManager.PostSystemFailure(requestID);
	}
    free(buf);
    return nullptr;
}
static napi_value keyDown(napi_env env, napi_callback_info info) {
    
    size_t argc = 3;
    napi_value params[3];
    napi_get_cb_info(env, info, &argc, params, nullptr, nullptr);
    if (!renderer_inited) {
		return nullptr; 
	}
    int deviceId;
    int key;
    bool isRepeat;
    napi_get_value_int32(env, params[0], &deviceId);
    napi_get_value_int32(env, params[1], &key);
    napi_get_value_bool(env, params[2], &isRepeat);
  	KeyInput keyInput;
	keyInput.deviceId = (InputDeviceID)deviceId;
	keyInput.keyCode = (InputKeyCode)key;
	keyInput.flags = KEY_DOWN;
	if (isRepeat) {
		keyInput.flags |= KEY_IS_REPEAT;
	}
    bool result = NativeKey(keyInput);
    napi_value jsResult;
    napi_get_boolean(env, result, &jsResult);
    return jsResult;
}
static napi_value keyUp(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value params[2];
    napi_get_cb_info(env, info, &argc, params, nullptr, nullptr);
    if (!renderer_inited) {
		return nullptr; 
	}
    int deviceId;
    int key;
    napi_get_value_int32(env, params[0], &deviceId);
    napi_get_value_int32(env, params[1], &key);
    
    napi_value jsResult;
   
	if (key == 0 && deviceId >= DEVICE_ID_PAD_0 && deviceId <= DEVICE_ID_PAD_9) {
		// Ignore keycode 0 from pads. Stadia controllers seem to produce them when pressing L2/R2 for some reason, confusing things.
        napi_get_boolean(env, false, &jsResult);
		return jsResult;  // need to eat the key so it doesn't go through legacy path
	}
  	KeyInput keyInput;
	keyInput.deviceId = (InputDeviceID)deviceId;
	keyInput.keyCode = (InputKeyCode)key;
	keyInput.flags = KEY_UP;
    bool result = NativeKey(keyInput);
   
    napi_get_boolean(env, result,&jsResult);
    return jsResult;
}

napi_value touchHandle(napi_env env, napi_callback_info info){
	GET_NAPI_ARGS(env, info, 4)
	GET_NAPI_ARG(float, x, env, args[0])
	GET_NAPI_ARG(float, y, env, args[1])
	GET_NAPI_ARG(int32_t, code, env, args[2])
	GET_NAPI_ARG(int32_t, pointerId, env, args[3])
    TouchInput touch;
    touch.id = pointerId;
    touch.x = x * display_scale * g_display.dpi_scale ;
    touch.y = y * display_scale *  g_display.dpi_scale ;
    touch.flags = code;
    NativeTouch(touch);
	return nullptr;
}

static napi_value joystickAxis(napi_env env, napi_callback_info info) {
	
    size_t argc = 4;
    napi_value params[4];
    napi_get_cb_info(env, info, &argc, params, nullptr, nullptr);
    if (!renderer_inited) {
		return nullptr; 
	}
    
    int deviceId;
    int count;
    
    napi_get_value_int32(env, params[0], &deviceId);
    napi_get_value_int32(env, params[3], &count);
    uint32_t axisIdsLength;
    uint32_t valuesLength;
    napi_get_array_length(env, params[1], &axisIdsLength);
    napi_get_array_length(env, params[2], &valuesLength);
    _dbg_assert_(count <= axisIdsLength);
	_dbg_assert_(count <= valuesLength);
    int* axisIds = (int*)malloc(axisIdsLength * sizeof(int));
    float* values = (float*)malloc(valuesLength * sizeof(float));
    for (uint32_t i = 0; i < count; i++) {
        napi_value element;
        napi_get_element(env, params[1], i, &element);
        int id;
        napi_get_value_int32(env, element, &id);
        axisIds[i] = id;
        napi_get_element(env, params[2], i, &element);
        double value;
        napi_get_value_double(env, element, &value);
        values[i] = value;
    }
    AxisInput *axis = new AxisInput[count];
    for (int i = 0; i < count; i++) {
		axis[i].deviceId = (InputDeviceID)(int)deviceId;
		axis[i].axisId = (InputAxis)(int)axisIds[i];
		axis[i].value = values[i];
	}
    NativeAxis(axis, count);
	delete[] axis;
    delete[] values;
    napi_value jsResult;
    napi_get_boolean(env, true, &jsResult);
    return jsResult;
}
static napi_value mouse(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value params[4];
    napi_get_cb_info(env, info, &argc, params, nullptr, nullptr);
    if (!renderer_inited) {
		return nullptr; 
	}
    double x;
    double y;
    int button;
    int action;
    napi_get_value_double(env, params[0], &x);
    napi_get_value_double(env, params[1], &y);
    napi_get_value_int32(env, params[2], &button);
    napi_get_value_int32(env, params[3], &action);
    
    TouchInput input{};

	static float last_x = 0.0f;
	static float last_y = 0.0f;

	if (x == -1.0f) {
		x = last_x;
	} else {
		last_x = x;
	}
	if (y == -1.0f) {
		y = last_y;
	} else {
		last_y = y;
	}
    x *= g_display.dpi_scale;
	y *= g_display.dpi_scale;
    if (button == 0) {
		// It's a pure mouse move.
		input.flags = TOUCH_MOUSE | TOUCH_MOVE;
		input.x = x;
		input.y = y;
		input.id = 0;
	} else {
		//input.buttons = button;
		input.x = x;
		input.y = y;
		switch (action) {
		case 1:
			input.flags = TOUCH_MOUSE | TOUCH_DOWN;
			break;
		case 2:
			input.flags = TOUCH_MOUSE | TOUCH_UP;
			break;
		}
		input.id = 0;
	}
	INFO_LOG(Log::System, "New-style mouse event: %f %f %d %d -> x: %f y: %f buttons: %d flags: %04x", x, y, button, action, input.x, input.y,  input.flags);
	NativeTouch(input);
    if (button) {
		KeyInput input{};
		input.deviceId = DEVICE_ID_MOUSE;
		switch (button) {
		case 1: input.keyCode = NKCODE_EXT_MOUSEBUTTON_1; break;
		case 2: input.keyCode = NKCODE_EXT_MOUSEBUTTON_2; break;
		case 3: input.keyCode = NKCODE_EXT_MOUSEBUTTON_3; break;
		default: WARN_LOG(Log::System, "Unexpected mouse button %d", button);
		}
		input.flags = action == 1 ? KEY_DOWN : KEY_UP;
		if (input.keyCode != 0) {
			NativeKey(input);
		}
	}
    napi_value jsResult;
    napi_get_boolean(env, true, &jsResult);
    return jsResult;
}
void mouseWheelHandle(float  x, float y){
    int wheelDelta = y * 30.0f;
	if (wheelDelta > 500) wheelDelta = 500;
	if (wheelDelta < -500) wheelDelta = -500;
    KeyInput key;
	key.deviceId = DEVICE_ID_MOUSE;
	if (wheelDelta < 0) {
		key.keyCode = NKCODE_EXT_MOUSEWHEEL_DOWN;
		wheelDelta = -wheelDelta;
	} else {
		key.keyCode = NKCODE_EXT_MOUSEWHEEL_UP;
	}
    // There's no separate keyup event for mousewheel events,
	// so we release it with a slight delay.
	key.flags = KEY_DOWN | KEY_HASWHEELDELTA | (wheelDelta << 16);
    NativeKey(key);
}
static napi_value mouseWheelEvent(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value params[2];
    napi_get_cb_info(env, info, &argc, params, nullptr, nullptr);
    double x;
    double y;
    napi_get_value_double(env, params[0], &x);
    napi_get_value_double(env, params[1], &y);
    if (!renderer_inited) {
		return nullptr; // could probably return true here too..
	}
    mouseWheelHandle(x,y);
    return nullptr;
}

static napi_value mouseDelta(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value params[2];
    napi_get_cb_info(env, info, &argc, params, nullptr, nullptr);
    double  x;
    double y;
    napi_get_value_double(env, params[0], &x);
    napi_get_value_double(env, params[1], &y);

    napi_value jsResult;
    if (!renderer_inited) {
        napi_get_boolean(env, false, &jsResult);
		return jsResult; // could probably return true here too..
	}
    NativeMouseDelta(x, y);
    napi_get_boolean(env, true, &jsResult);
    return jsResult;
}

static napi_value isAtTopLevel(napi_env env, napi_callback_info info) {
    
    napi_value jsResult;
    bool result = NativeIsAtTopLevel();
    napi_get_boolean(env, result, &jsResult);
    return jsResult;
}
napi_value audioInit(napi_env env, napi_callback_info info) {
    sampleRate = optimalSampleRate;
	if (optimalSampleRate == 0) {
		sampleRate = 44100;
	}
	if (optimalFramesPerBuffer > 0) {
		framesPerBuffer = optimalFramesPerBuffer;
	} else {
		framesPerBuffer = 512;
	}
    // Some devices have totally bonkers buffer sizes like 8192. They will have terrible latency anyway, so to avoid having to
	// create extra smart buffering code, we'll just let their regular mixer deal with it, missing the fast path (as if they had one...)
	if (framesPerBuffer > 512) {
		framesPerBuffer = 512;
		sampleRate = 44100;
	}
    INFO_LOG(Log::Audio, "NativeApp.audioInit() -- Using OpenSL audio! frames/buffer: %i	 optimal sr: %i	 actual sr: %i", optimalFramesPerBuffer, optimalSampleRate, sampleRate);
	if (!g_audioState) {
		g_audioState = OHOSAudio_Init(&NativeMix, framesPerBuffer, sampleRate);
	} else {
		ERROR_LOG(Log::Audio, "Audio state already initialized");
	}
    OHOSAudio_Resume(g_audioState);
    napi_value jsResult;
    return jsResult;
}
static napi_value audioShutdown(napi_env env, napi_callback_info info) {
    if (g_audioState) {
		OHOSAudio_Shutdown(g_audioState);
		g_audioState = nullptr;
	} else {
		ERROR_LOG(Log::Audio, "Audio state already shutdown!");
	}
	return nullptr;
}
static napi_value audioRecording_1SetSampleRate(napi_env env, napi_callback_info info) {
	OHOSAudio_Recording_SetSampleRate(g_audioState, sampleRate);
    return nullptr;
}

static napi_value audioRecording_1Start(napi_env env, napi_callback_info info) {
	OHOSAudio_Recording_Start(g_audioState);
    return nullptr;
}

static napi_value audioRecording_1Stop(napi_env env, napi_callback_info info) {
	OHOSAudio_Recording_Stop(g_audioState);
    return nullptr;
}


void ExportApi(napi_env env, napi_value exports){
     napi_property_descriptor desc[] = {
        { "native_init", nullptr, Native_Init, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "audioInit", nullptr, audioInit, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "audioShutdown", nullptr, audioShutdown, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "audioRecording_1SetSampleRate", nullptr, audioRecording_1SetSampleRate, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "audioRecording_1Start", nullptr, audioRecording_1Start, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "audioRecording_1Stop", nullptr, audioRecording_1Stop, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "queryConfig", nullptr, queryConfig, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "backbufferResize", nullptr, backbufferResize, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "setDisplayParameters", nullptr, setDisplayParameters, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "onPostCommand", nullptr, onPostCommand, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "sendRequestResult", nullptr, sendRequestResult, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "sendMessage", nullptr, sendMessage, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "keyDown", nullptr, keyDown, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "keyUp", nullptr, keyUp, nullptr, nullptr, nullptr, napi_default, nullptr },
	  	{ "sendTouchEvent", nullptr, touchHandle, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "joystickAxis", nullptr, joystickAxis, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "mouseWheelEvent", nullptr, mouseWheelEvent, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "mouseDelta", nullptr, mouseDelta, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "isAtTopLevel", nullptr, isAtTopLevel, nullptr, nullptr, nullptr, napi_default, nullptr }
    };
    if (napi_ok != napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc)) {
        ERROR_LOG(Log::System, "Export: napi_define_properties failed");
    }
}


EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    ExportApi(env, exports);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "ppsspp",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void) { napi_module_register(&demoModule); }
