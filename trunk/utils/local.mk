LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := utils
LOCAL_SRC_FILES := utils.c memory.c

include $(BUILD_MODULE)

