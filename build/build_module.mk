###########################################################
## Standard rules for building a static library.
##
## Additional inputs from base_rules.make:
## None.
##
## LOCAL_MODULE_SUFFIX will be set for you.
###########################################################
LOCAL_MODULE := $(strip $(LOCAL_MODULE))
ifeq ($(LOCAL_MODULE),)
	$(error $(LOCAL_PATH): LOCAL_MODULE is not defined)
endif

ifeq ($(strip $(LOCAL_MODULE_SUFFIX)),)
LOCAL_MODULE_SUFFIX := .a
endif

intermediates := $(OUT)/modules/$(LOCAL_MODULE)_intermediates
LOCAL_BUILT_MODULE := $(intermediates)/$(LOCAL_MODULE)$(LOCAL_MODULE_SUFFIX)
$(LOCAL_BUILT_MODULE) : PRIVATE_MODULE := $(LOCAL_MODULE)

LOCAL_CPPFLAGS += -I$(TOP)/include 
LOCAL_CXXFLAGS += -D__cplusplus -fno-exceptions
LOCAL_ASFLAGS += -w -D__ASSEMBLY__
LOCAL_CFLAGS += -fno-builtin

###########################################################
## C : Compile .c to .o
###########################################################
c_sources := $(filter %.c, $(LOCAL_SRC_FILES))
c_objects := $(addprefix $(intermediates)/, $(c_sources:.c=.o))
c_depends := $(addprefix $(intermediates)/, $(c_sources:.c=.d))

ifneq ($(strip $(c_objects)),)
$(c_objects) $(c_depends) : PRIVATE_CFLAGS := $(LOCAL_CFLAGS)
$(c_objects) $(c_depends) : PRIVATE_CPPFLAGS := $(LOCAL_CPPFLAGS)

$(c_objects) : $(intermediates)/%.o : $(LOCAL_PATH)/%.c $(LOCAL_EXT_DEPS)
	@echo "Compile $@..."
	@mkdir -p $(dir $@)
	$(CC) $(PRIVATE_CPPFLAGS) $(PRIVATE_CFLAGS) -c -o $@ $<

$(c_depends) : $(intermediates)/%.d : $(LOCAL_PATH)/%.c
	@echo "Gen depends $@..."
	@mkdir -p $(dir $@)
	$(CC) $(PRIVATE_CPPFLAGS) $(PRIVATE_CFLAGS) -MM $< | sed "1,1 s,^,$(@D)/,i" > $@

ifeq ($(filter $(CLEANGOAL), $(GOAL)),)
-include $(c_depends)
endif

endif

###########################################################
## CXX : Compile .cpp to .o
###########################################################
cpp_sources := $(filter %.cpp, $(LOCAL_SRC_FILES))
cpp_objects := $(addprefix $(intermediates)/, $(cpp_sources:.cpp=.o))
cpp_depends := $(addprefix $(intermediates)/, $(cpp_sources:.cpp=.d))

ifneq ($(strip $(cpp_objects)),)
$(cpp_objects) $(cpp_depends) : PRIVATE_CXXFLAGS := $(LOCAL_CXXFLAGS)
$(cpp_objects) $(cpp_depends) : PRIVATE_CPPFLAGS := $(LOCAL_CPPFLAGS)

$(cpp_objects) : $(intermediates)/%.o : $(LOCAL_PATH)/%.cpp $(LOCAL_EXT_DEPS)
	@echo "Compile $@..."
	@mkdir -p $(dir $@)
	$(CC) $(PRIVATE_CPPFLAGS) $(PRIVATE_CXXFLAGS) -c -o $@ $<

$(cpp_depends) : $(intermediates)/%.d : $(LOCAL_PATH)/%.cpp
	@echo "Gen depends $@..."
	@mkdir -p $(dir $@)
	$(CC) $(PRIVATE_CPPFLAGS) $(PRIVATE_CXXFLAGS) -MM $< | sed "1,1 s,^,$(@D)/,i" > $@

ifeq ($(filter $(CLEANGOAL), $(GOAL)),)
-include $(cpp_depends)
endif

endif

###########################################################
## AS : Compile .S to .o
###########################################################
s_sources := $(filter %.S, $(LOCAL_SRC_FILES))
s_objects := $(addprefix $(intermediates)/, $(s_sources:.S=.o))
s_depends := $(addprefix $(intermediates)/, $(s_sources:.S=.d))
ifneq ($(strip $(s_objects)),)
$(s_objects) $(s_depends) : PRIVATE_ASFLAGS := $(LOCAL_ASFLAGS)
$(s_objects) $(s_depends) : PRIVATE_CPPFLAGS := $(LOCAL_CPPFLAGS)
$(s_objects) : $(intermediates)/%.o : $(LOCAL_PATH)/%.S $(LOCAL_EXT_DEPS)
	@echo "Compile $@..."
	@mkdir -p $(dir $@)
	$(CC) $(PRIVATE_CPPFLAGS) $(PRIVATE_ASFLAGS) -c -o $@ $<

$(s_depends) : $(intermediates)/%.d : $(LOCAL_PATH)/%.S
	@echo "Gen depends $@..."
	@mkdir -p $(dir $@)
	$(CC) $(PRIVATE_CPPFLAGS) $(PRIVATE_ASFLAGS) -MM $< | sed "1,1 s,^,$(@D)/,i" > $@

ifeq ($(filter $(CLEANGOAL), $(GOAL)),)
-include $(s_depends)
endif
endif

all_objects := $(c_objects) $(s_objects) $(cpp_objects)
all_objects := $(strip $(all_objects))

$(LOCAL_BUILT_MODULE) : PRIVATE_ALLOBJS := $(all_objects)
$(LOCAL_BUILT_MODULE) : $(all_objects)
	@echo "Build Module $@..."
	@mkdir -p $(dir $@)
	$(AR) -rc $@ $(PRIVATE_ALLOBJS)

$(LOCAL_MODULE).clean: PRIVATE_CLEAN := $(s_objects) \
					$(c_objects) \
					$(cpp_objects) \
					$(LOCAL_BUILT_MODULE)
$(LOCAL_MODULE).clean:
	@echo "Remove $(PRIVATE_CLEAN)"
	$(RM) $(PRIVATE_CLEAN) -f

$(LOCAL_MODULE).cleanall: PRIVATE_CLEANALL := $(s_depends) \
					$(c_depends) \
					$(cpp_depends)

$(LOCAL_MODULE).cleanall: $(LOCAL_MODULE).clean
	@echo "Remove $(PRIVATE_CLEANALL)"
	$(RM) $(PRIVATE_CLEANALL) -f	

ALL_MODULES += $(LOCAL_MODULE)
ALL_MODULES.$(LOCAL_MODULE).BUILT := $(LOCAL_BUILT_MODULE)
