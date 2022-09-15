LLVM_DIR		?= $(shell llvm-config --obj-root)
CLANGXX		?= $(LLVM_DIR)/bin/clang++
OPT		?= $(LLVM_DIR)/bin/opt

ifeq ($(shell $(LLVM_DIR)/bin/llvm-config --build-mode),Debug)
ISDEBUG = 1
$(info debug mode on)
endif
comma :=,

OX          := -O3

CFLAGS		:= -Wall -Wnarrowing -std=c++11 -MD
LFLAGS		+= #-lboost_regex 
HEADERS		:= src
INCLUDE		+= -I${HEADERS} -I../common/src
LIB		:= 

SIMILARBITS	?= 2

MIOSIX_EABI     := /opt/arm-miosix-eabi/arm-miosix-eabi
MIOSIX_CFLAGS   := -target arm-none-linux-eabi --sysroot=$(MIOSIX_EABI) -I$(MIOSIX_EABI)/include/ -I$(MIOSIX_EABI)/include/c++/4.7.3/ -I$(MIOSIX_EABI)/include/c++/4.7.3/arm-miosix-eabi/ -D_MIOSIX -D_MIOSIX_GCC_PATCH_VERSION=1 -mcpu=cortex-m3 -mthumb -ffunction-sections -Wall


MODULE		:= blackscholes.out

CPP_FILES       := blackscholes.c
OBJ_FILES       := $(addprefix obj/,$(notdir $(CPP_FILES:.c=.o))) 
OBJ_FILES_FIX   := $(addprefix obj/,$(notdir $(CPP_FILES:.c=.fixp.o)))
LL_FILES_ERR    := $(addprefix obj/,$(notdir $(CPP_FILES:.c=.err.ll)))
IMIX_STAT       := $(addprefix stats/,$(notdir $(CPP_FILES:.c=.mix.txt))) $(addprefix stats/,$(notdir $(CPP_FILES:.c=.fixp.mix.txt)))
MLFEAT_STAT     := $(addprefix stats/,$(notdir $(CPP_FILES:.c=.mlfeat.txt))) $(addprefix stats/,$(notdir $(CPP_FILES:.c=.fixp.mlfeat.txt)))
LLVM_STAT       := $(addprefix stats/,$(notdir $(CPP_FILES:.c=.llvm.txt)))

.PHONY: all clean stats

all: $(MODULE) stats

.PRECIOUS: obj/%.ll obj/%.o obj/%.s obj/%.init.ll obj/%.vra.ll obj/%.tuner.ll obj/%.fixp.ll

DIR:
	@echo ${CPP_FILES}
	@echo ${OBJ_FILES}
	mkdir -p bin
	mkdir -p obj
	mkdir -p stats


$(MODULE): DIR $(OBJ_FILES) $(OBJ_FILES_FIX)
	$(CLANGXX) $(OBJ_FILES) $(LIB) $(LFLAGS) -o bin/$@
	$(CLANGXX) $(OBJ_FILES_FIX) $(LIB) $(LFLAGS) -o bin/$@.fixp

stats: $(MODULE) $(LL_FILES_ERR) $(IMIX_STAT) $(LLVM_STAT) $(MLFEAT_STAT)

obj/%.ll: src/%.c
	$(CLANGXX) -S -emit-llvm $(MIOSIX_FLAGS) $(CFLAGS) $(INCLUDE) -DINPUT=$(INPUT) -DOUTPUT=$(OUTPUT) $< -o $@

obj/%.init.ll: obj/%.ll
	$(OPT) -load=$(INITLIB) -S -taffoinit -mem2reg $(if $(ISDEBUG), -debug-only=taffo-init$(comma)annotation) -stats $< -o $@ 2> stats/init.log

obj/%.vra.ll: obj/%.init.ll
	$(OPT) -load=$(VRALIB) -S -taffoVRA $(if $(ISDEBUG), -debug-only=taffo-vra$(comma)ValueRangeAnalysis) -stats $< -o $@ 2> stats/vra.log

obj/%.tuner.ll : obj/%.vra.ll
	$(OPT) -load=$(TUNERLIB) -S -taffodta --similarbits=$(SIMILARBITS) -globaldce $(if $(ISDEBUG), -debug-only=taffo-dta$(comma)tunerfunction) -stats $< -o $@ 2> stats/tuner.log

obj/%.fixp.ll: obj/%.tuner.ll
	$(OPT) -load=$(PASSLIB) -S -flttofix -dce $(if $(ISDEBUG), -debug-only=taffo-conversion) -stats $< -o $@ 2> stats/flttofix.log

obj/%.err.ll: obj/%.fixp.ll
ifndef DISABLE_ERROR
	$(OPT) -load=$(ERRORLIB) -S -errorprop -startonly $(if $(ISDEBUG), -debug-only=errorprop) $< -o $@ 2> stats/errorprop.log
else
	echo > $@
	echo > stats/errorprop.log
endif

obj/%.s: obj/%.ll
	$(CLANGXX) -S $(MIOSIX_FLAGS) $< -o $@ $(OX)

obj/%.o: obj/%.s
	$(CLANGXX) -c $(MIOSIX_FLAGS) $< -o $@

stats/%.mix.txt: obj/%.ll
	$(INSTMIX) $< > $@

stats/%.mlfeat.txt: obj/%.ll
	$(TAFFO_MLFEAT) $< > $@

stats/%.llvm.txt: obj/%.ll
	$(OPT) -load=$(PASSLIB) -S -flttofix -dce -stats $< -o /dev/null 2> $@

miosix_cflags:
	$(eval MIOSIX_FLAGS += $(MIOSIX_CFLAGS))

miosix_o: miosix_cflags DIR $(OBJ_FILES) $(OBJ_FILES_FIX) $(LL_FILES_ERR) $(FLT_IMIX_STAT) $(FIX_IMIX_STAT) $(LLVM_STAT)
	cp obj/blackscholes.o $(MIOSIX_ROOT)/$(basename $(MODULE)).o
	cp obj/blackscholes.fixp.o $(MIOSIX_ROOT)/$(basename $(MODULE)).fixp.o

miosix_float: miosix_o
	make clean -C $(MIOSIX_ROOT)
	make BENCH=$(basename $(MODULE)).o -j 8 -C $(MIOSIX_ROOT)
	#make program -C $(MIOSIX_ROOT)

miosix_fix: miosix_o
	make clean -C $(MIOSIX_ROOT)
	make BENCH=$(basename $(MODULE)).fixp.o -j 8 -C $(MIOSIX_ROOT)
	#make program -C $(MIOSIX_ROOT)

clean:
	rm -rf *.o
	rm -rf *.d
	rm -rf *.out
	rm -rf bin
	rm -rf obj
	rm -rf *.ll
	rm -rf *.s
	rm -rf stats
