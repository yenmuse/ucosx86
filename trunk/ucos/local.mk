LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := kernel
LOCAL_CPPFLAGS := -DUCOS
LOCAL_SRC_FILES := ucos_ii.c os_cpu_a.S os_cpu_c.c
include $(BUILD_MODULE)
