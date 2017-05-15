LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS +=
LOCAL_LDLIBS +=

LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/ffmpeg/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../avcodec/include

LOCAL_SRC_FILES := source/ffinit.c
LOCAL_SRC_FILES += source/ffdecoder.c
LOCAL_SRC_FILES += source/avdecoder.c

LOCAL_SHARED_LIBRARIES := ijkffmpeg

LOCAL_MODULE := ffutils
include $(BUILD_STATIC_LIBRARY)
