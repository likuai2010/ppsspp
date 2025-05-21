# the minimum version of CMake.
cmake_minimum_required(VERSION 3.4.1)
project(vulkanohos)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 17)



ADD_DEFINITIONS(-DVK_USE_PLATFORM_OHOS=1)

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()



find_library(
    # Sets the name of the path variable.
    EGL-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    EGL
)

find_library(
    # Sets the name of the path variable.
    GLES-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    GLESv3
)

find_library(
    # Sets the name of the path variable.
    hilog-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    hilog_ndk.z
)

find_library(
    # Sets the name of the path variable.
    libace-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    ace_ndk.z
)

find_library(
    # Sets the name of the path variable.
    libnapi-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    ace_napi.z
)

find_library(
    # Sets the name of the path variable.
    libuv-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    uv
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../ "${CMAKE_CURRENT_BINARY_DIR}/ppsspp")
include_directories(${NATIVERENDER_ROOT_PATH}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../
                    ${NATIVERENDER_ROOT_PATH}/include)

add_library(entry SHARED
    napi_init.cpp
    render/egl_core.cpp
    render/plugin_render.cpp
    manager/plugin_manager.cpp
)
target_link_libraries(entry PUBLIC
					libace_napi.z.so
					vulkan
                    libace_ndk.z.so
                    libnative_window.so
                    ppsspp_core
                    libohfileuri.so

   ${EGL-lib} ${GLES-lib} ${hilog-lib} ${libace-lib} ${libnapi-lib} ${libuv-lib} libc++.a
)
