LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := boot
LOCAL_SRC_FILES := start.S stage1.S

include $(BUILD_MODULE)
