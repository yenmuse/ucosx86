LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := driver
LOCAL_CPPFLAGS := 
LOCAL_SRC_FILES := pci.c
include $(BUILD_MODULE)
