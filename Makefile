# Cross toolchain variables
# If these are not in your path, you can make them absolute.
XT_PRG_PREFIX = mipsel-linux-gnu-
CC = $(XT_PRG_PREFIX)gcc
LD = $(XT_PRG_PREFIX)ld

# uMPS2-related paths

# Simplistic search for the umps2 install. prefix. If you have umps2
# installed on some weird location, set UMPS2_DIR_PREFIX by hand.
ifneq ($(wildcard /usr/bin/umps2),)
    UMPS2_DIR_PREFIX = /usr
else
    UMPS2_DIR_PREFIX = /usr/local
endif

UMPS2_DATA_DIR = $(UMPS2_DIR_PREFIX)/share/umps2
UMPS2_INCLUDE_DIR = $(UMPS2_DIR_PREFIX)/include/umps2

# Compiler options
CFLAGS_LANG = -ansi -std=gnu11
CFLAGS_MIPS = -mips1 -mfp32
CFLAGS = $(CFLAGS_LANG) $(CFLAGS_MIPS) -I$(UMPS2_INCLUDE_DIR) -I. -Wall -fstack-protector

# Linker options
LDFLAGS = -nostdlib -T $(UMPS2_DATA_DIR)/umpscore.ldscript

# Add the location of crt*.S to the search path
VPATH = $(UMPS2_DATA_DIR)

.PHONY : all clean

all : tests/p2/kernel.core.umps

tests/p1/kernel.core.umps : tests/p1/kernel
	umps2-elf2umps -k $<

tests/p1/kernel : tests/p1/p1test_rikaya_v0.o pcb/pcb.o asl/asl.o crtso.o libumps.o
	$(LD) -o $@ $^ $(LDFLAGS)

tests/p1.5/kernel.core.umps : tests/p1.5/kernel
	umps2-elf2umps -k $<

tests/p1.5/kernel : tests/p1.5/main.o crtso.o libumps.o pcb/pcb.o interrupt/interrupt.o scheduler/scheduler.o syscall/syscall.o tests/p1.5/p1.5test_rikaya_v0.o utils/utils.o
	$(LD) -o $@ $^ $(LDFLAGS)

tests/p2/kernel.core.umps : tests/p2/kernel
	umps2-elf2umps -k $<

tests/p2/kernel : tests/p2/main.o crtso.o libumps.o pcb/pcb.o asl/asl.o interrupt/interrupt.o scheduler/scheduler.o syscall/syscall.o tests/p2/p2test_rikaya_v0.2.o utils/utils.o
	$(LD) -o $@ $^ $(LDFLAGS)

clean :
	find . -name "*.o" -o -name "kernel" -o -name "kernel.*.umps" -o -name "term*.umps" -type f|xargs rm -f

# Pattern rule for assembly modules
%.o : %.S
	$(CC) $(CFLAGS) -c -o $@ $<
