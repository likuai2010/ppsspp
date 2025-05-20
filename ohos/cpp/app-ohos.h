// This is generic code that is included in all Android apps that use the
// Native framework by Henrik Rydgard (https://github.com/hrydgard/native).

// It calls a set of methods defined in NativeApp.h. These should be implemented
// by your game or app.

#include <cstdlib>
#include <cstdint>
#include "Common/File/DirListing.h"
#include "napi/native_api.h"
#include <native_window/external_window.h>
#include <ace/xcomponent/native_interface_xcomponent.h>

napi_value Native_Init(napi_env env, napi_callback_info info);

void DisplayRender(void *window);

void ExportApi(napi_env env, napi_value exports);

void setDisplayParameters(int xres, int yres);
void backbufferResize(int xres, int yres, int format);

void computeDesiredBackbufferDimensions();


void touchHandle(float x, float y, int code, int pointerId);
void mouseWheelHandle(float x, float y);

