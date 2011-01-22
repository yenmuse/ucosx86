LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := mem
LOCAL_SRC_FILES := mem.c test.cpp

include $(BUILD_MODULE)

