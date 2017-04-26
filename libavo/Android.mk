LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS += -DOS_ANDROID
LOCAL_LDLIBS += -llog -landroid -lOpenSLES -lEGL -lGLESv2

LOCAL_C_INCLUDES := .
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../avcodec/include

LOCAL_SRC_FILES := $(wildcard src/*.c)
LOCAL_SRC_FILES += $(wildcard src/*.cpp)
LOCAL_SRC_FILES += $(wildcard src/audio_input/*.c)
LOCAL_SRC_FILES += $(wildcard src/audio_input/*.cpp)
LOCAL_SRC_FILES += $(wildcard src/audio_output/*.c)
LOCAL_SRC_FILES += $(wildcard src/audio_output/*.cpp)
LOCAL_SRC_FILES += $(wildcard src/render/*.c)
LOCAL_SRC_FILES += $(wildcard src/render/*.cpp)
LOCAL_SRC_FILES += src/audio_input/opensles/opensles_input.c
LOCAL_SRC_FILES += src/audio_output/opensles/opensles_output.c
LOCAL_SRC_FILES += src/render/opengles2/gles2_render.c

LOCAL_MODULE := avo
include $(BUILD_SHARED_LIBRARY)
