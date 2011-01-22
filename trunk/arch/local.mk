LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := arch 
LOCAL_SRC_FILES := 8259A.c irq.c main.c ctors.cpp

include $(BUILD_MODULE)

