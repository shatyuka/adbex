set(NDK_VERSION 26.2.11394342)
set(ANDROID_ARM_MODE arm)
set(ANDROID_ABI "arm64-v8a")
set(ANDROID_PLATFORM 23)

set(ANDROID_SDK_ROOT "$ENV{ANDROID_SDK_ROOT}")
if ("${ANDROID_SDK_ROOT}" STREQUAL "")
    if (WIN32)
        STRING(REGEX REPLACE "\\\\" "/" USERPROFILE $ENV{USERPROFILE})
        set(PROGRAM_LIBRARY ${USERPROFILE}/AppData/Local)
    elseif (APPLE)
        set(PROGRAM_LIBRARY ~/Library)
    endif ()
    set(ANDROID_SDK_ROOT ${PROGRAM_LIBRARY}/Android/Sdk)
    if (NOT EXISTS ${ANDROID_SDK_ROOT})
        message(FATAL_ERROR "Please set ANDROID_SDK_ROOT environment variable")
    endif ()
endif ()

set(NDK ${ANDROID_SDK_ROOT}/ndk/${NDK_VERSION})
if (NOT EXISTS ${NDK})
    message(FATAL_ERROR "NDK version ${NDK_VERSION} does not exist")
endif ()
set(CMAKE_TOOLCHAIN_FILE ${NDK}/build/cmake/android.toolchain.cmake)
