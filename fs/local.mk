LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := fs
FSL_FILES := FSL/fat_data.c \
		FSL/fat_in.c \
		FSL/fat_misc.c \
		FSL/fat_dir.c \
		FSL/fat_ioct.c \
		FSL/fat_out.c \
		FSL/fat_open.c

API_FILES := API/api_dir.c \
		API/api_out.c \
		API/fs_info.c \
		API/api_in.c \
		API/api_misc.c

CLIB_FILES := CLIB/clibmisc.c

DEVICE_IDE_FILES := DEVICE/IDE/ide_drv.c \
		DEVICE/IDE/ide_x_hw.c

DEVICE_RAM_FILES := DEVICE/ram/r_misc.c

LBL_FILES := LBL/lb_misc.c

OS_FILES := OS/fs_x_ucos_ii.c

SAMPLE_FILES := sample/fs_test.c

LOCAL_SRC_FILES := $(FSL_FILES) \
		$(API_FILES) \
		$(CLIB_FILES) \
		$(DEVICE_IDE_FILES) \
		$(DEVICE_RAM_FILES) \
		$(LBL_FILES) \
		$(OS_FILES) \
		$(SAMPLE_FILES)
	
LOCAL_CFLAGS += -I$(LOCAL_PATH)/API \
		-I$(LOCAL_PATH)/CONFIG/Win32 \
		-I$(LOCAL_PATH)/CLIB \
		-I$(LOCAL_PATH)/FSL \
		-I$(LOCAL_PATH)/OS \
		-I$(LOCAL_PATH)/LBL 

include $(BUILD_MODULE)
