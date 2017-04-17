LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS += -DOS_LINUX -DOS_ANDROID
LOCAL_LDLIBS +=

LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../h264/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libavo/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../avcodec/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../ffutils/include
LOCAL_C_INCLUDES += ../../sdk/include
					
LOCAL_SRC_FILES := $(wildcard source/*.c)
LOCAL_SRC_FILES += $(wildcard source/*.cpp)
LOCAL_SRC_FILES += $(wildcard source/live-player/*.c)
LOCAL_SRC_FILES += $(wildcard source/live-player/*.cpp)
LOCAL_SRC_FILES += $(wildcard source/file-player/*.c)
LOCAL_SRC_FILES += $(wildcard source/file-player/*.cpp)

LOCAL_MODULE := avplayer
include $(BUILD_STATIC_LIBRARY)
