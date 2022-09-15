LLVM_DIR		?= $(shell llvm-config --obj-root)
CLANGXX		?= $(LLVM_DIR)/bin/clang++
OPT		?= $(LLVM_DIR)/bin/opt

ifeq ($(shell $(LLVM_DIR)/bin/llvm-config --build-mode),Debug)
ISDEBUG = 1
$(info debug mode on)
endif
comma :=,

OX          := -O3

CFLAGS		+= -Wall -Wnarrowing -std=c++11
LFLAGS		+= #-lboost_regex
HEADERS		:= src
INCLUDE		+= -I${HEADERS} -I../common/src

MIOSIX_EABI     := /opt/arm-miosix-eabi/arm-miosix-eabi
MIOSIX_CFLAGS   := -target arm-none-linux-eabi --sysroot=$(MIOSIX_EABI) -I$(MIOSIX_EABI)/include/ -I$(MIOSIX_EABI)/include/c++/4.7.3/ -I$(MIOSIX_EABI)/include/c++/4.7.3/arm-miosix-eabi/ -D_MIOSIX -D_MIOSIX_GCC_PATCH_VERSION=1 -mcpu=cortex-m3 -mthumb -ffunction-sections -Wall

MODULE		:= sobel.out

CPP_FILES := $(wildcard src/*.cpp)
LL_FILES  := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.ll))) 
OBJ_FLOAT := obj/main.o
OBJ_FIX   := obj/main.fixp.o

.PHONY: all clean stats

all: DIR $(MODULE) stats

.PRECIOUS: obj/%.ll obj/%.o obj/%.s obj/%.init.ll obj/%.vra.ll obj/%.fixp.ll

DIR:
	@echo ${CPP_FILES}
	mkdir -p bin
	mkdir -p obj
	mkdir -p stats

$(MODULE): DIR $(OBJ_FLOAT) $(OBJ_FIX)
	$(CLANGXX) $(OBJ_FLOAT) $(LFLAGS) -o bin/$@
	$(CLANGXX) $(OBJ_FIX) $(LFLAGS) -o bin/$@.fixp

stats: $(MODULE) obj/main.err.ll stats/main.fixp.mix.txt stats/main.mix.txt stats/main.llvm.txt stats/main.fixp.mlfeat.txt stats/main.mlfeat.txt

obj/%.ll : src/%.cpp
	$(CLANGXX) -S -emit-llvm $(MIOSIX_FLAGS) $(CFLAGS) $(INCLUDE) -DINPUT=$(INPUT) -DOUTPUT=$(OUTPUT) $< -o $@

obj/main.ll : $(LL_FILES)
	$(LLVM_DIR)/bin/llvm-link -S $^ -o $@

$(OBJ_FLOAT) : obj/main.ll
	$(CLANGXX) -S $(MIOSIX_FLAGS) $^ -o obj/main.s $(OX)
	$(CLANGXX) -c $(MIOSIX_FLAGS) obj/main.s -o $@

obj/main.init.ll : obj/main.ll
	$(OPT) -load=$(INITLIB) -S -taffoinit -mem2reg $(if $(ISDEBUG), -debug-only=taffo-init$(comma)annotation) -stats $< -o $@ 2> stats/init.log

obj/main.vra.ll: obj/main.init.ll
	$(OPT) -load=$(VRALIB) -S -taffoVRA $(if $(ISDEBUG), -debug-only=taffo-vra$(comma)ValueRangeAnalysis) -stats $< -o $@ 2> stats/vra.log

obj/main.tuner.ll : obj/main.vra.ll
	$(OPT) -load=$(TUNERLIB) -S -taffodta -globaldce $(if $(ISDEBUG), -debug-only=taffo-dta$(comma)tunerfunction) -stats $< -o $@ 2> stats/tuner.log

obj/main.fixp.ll : obj/main.tuner.ll
	$(OPT) -load=$(PASSLIB) -S -flttofix -dce -stats $(if $(ISDEBUG), -debug-only=taffo-conversion) $< -o $@ 2> stats/flttofix.log

$(OBJ_FIX) : obj/main.fixp.ll
	$(CLANGXX) -S $(MIOSIX_FLAGS) $^ -o obj/main.fixp.s $(OX)
	$(CLANGXX) -c $(MIOSIX_FLAGS) obj/main.fixp.s -o $@

obj/main.err.ll : obj/main.fixp.ll
ifndef DISABLE_ERROR
	$(OPT) -load=$(ERRORLIB) -S -mem2reg -errorprop -startonly -nounroll $(if $(ISDEBUG), -debug-only=errorprop) $< -o $@ 2> stats/errorprop.log
else
	echo > $@
	echo > stats/errorprop.log
endif

stats/%.mix.txt: obj/%.ll
	$(INSTMIX) $< > $@

stats/%.mlfeat.txt: obj/%.ll
	$(TAFFO_MLFEAT) $< > $@

stats/%.llvm.txt: obj/%.ll
	$(OPT) -load=$(PASSLIB) -S -flttofix -dce -stats $< -o /dev/null 2> $@

miosix_cflags:
	$(eval MIOSIX_FLAGS += $(MIOSIX_CFLAGS))

miosix_o: miosix_cflags DIR $(OBJ_FLOAT) $(OBJ_FIX) obj/main.err.ll stats/main.fixp.mix.txt stats/main.mix.txt stats/main.llvm.txt
	cp obj/main.o $(MIOSIX_ROOT)/$(basename $(MODULE)).o
	cp obj/main.fixp.o $(MIOSIX_ROOT)/$(basename $(MODULE)).fixp.o

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
	rm -rf stats
