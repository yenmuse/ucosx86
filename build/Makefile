include config.mk
include definitions.mk

ELF := ucos4pc.elf
.PHONY: default all modules clean cleanall modules_clean modules_cleanall distclean

all : $(OUT)/$(ELF)

ALL_MODULES := 
include $(call all-makefiles-under, $(TOP))
ALL_STATIC_LIBS := $(foreach m, $(ALL_MODULES), $(ALL_MODULES.$(m).BUILT))

LDFLAGS := -Tldscript.lds -nostdlib -u c_entry
$(OUT)/$(ELF): $(ALL_STATIC_LIBS)
	@echo "Generate ELF file $@..."
	$(CC) $(LDFLAGS) -o $@ $^

modules : $(ALL_STATIC_LIBS)

modules_clean : $(foreach m, $(ALL_MODULES), $(m).clean)

modules_cleanall : $(foreach m, $(ALL_MODULES), $(m).cleanall)

clean: modules_clean
	$(RM) $(OUT)/$(ELF) -f

cleanall: modules_cleanall
	$(RM) $(OUT)/$(ELF) -f

distclean:
	@echo "Remove dir $(OUT)..."
	$(RM) $(OUT) -rf

