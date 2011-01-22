Q = @
ifeq ($(VERBOSE),y)
	Q = 
endif

#CROSS_COMPILE 	= arm-elf-
CROSS_COMPILE 		= $(Q)
MAKE			= make
CC 			= $(CROSS_COMPILE)gcc
LD 			= $(CROSS_COMPILE)ld
AR 			= $(CROSS_COMPILE)ar
OBJDUMP 		= $(CROSS_COMPILE)objdump
OBJCOPY 		= $(CROSS_COMPILE)objcopy
RANLIB 			= $(CROSS_COMPILE)runlib
STRIP 			= $(CROSS_COMPILE)strip
CPP 			= $(CROSS_COMPILE)cpp
CXX 			= $(CROSS_COMPILE)c++
AS 			= $(CROSS_COMPILE)as
NM 			= $(CROSS_COMPILE)nm
READELF 		= $(CROSS_COMPILE)readelf
SIZE 			= $(CROSS_COMPILE)size
NASM			= $(Q)nasm
NDISASM			= $(Q)ndisasm
ECHO 			= $(Q)echo
RM 			= $(Q)rm
CP 			= $(Q)cp

TAG			= test

# CFLAGS = -g -fno-builtin -I$(BASE_DIR)/include 
# ASFLAGS = -w $(CFLAGS) -D__ASSEMBLY__
# CXXFLAGS = $(CFLAGS) -D__cplusplus -fno-exceptions

TOP := $(shell cd .. && pwd)
BUILD_SYSTEM := $(TOP)/build
OUT := $(TOP)/target/$(TAG)
CLEAR_VARS := $(BUILD_SYSTEM)/clear_vars.mk
BUILD_MODULE := $(BUILD_SYSTEM)/build_module.mk
GOAL := $(MAKECMDGOALS)
CLEANGOAL := clean cleanall modules_clean modules_cleanall distclean
