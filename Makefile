.POSIX:

OBJS = test.o

include sdk.mk

MACHFLAGS += -mcpu=cortex-m3 -mthumb
CFLAGS += $(MACHFLAGS) -D EFM32GG990F1024 $(SDK_INCDIRS:%=-I %)
LDFLAGS += $(MACHFLAGS) -T$(SDK_LDSCRIPT)
ARFLAGS += -U

CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy

program.bin: program.elf

program.elf: $(OBJS) libsdk.a
	$(CC) -L . -l sdk $(LDFLAGS) -o $@ $(OBJS)

libsdk.a: ${SDK_OBJS:%=libsdk.a(%)}

.PHONY: clean

clean:
	-rm -f program.bin program.elf libsdk.a $(OBJS)

.SUFFIXES: .bin .elf

.elf.bin:
	$(OBJCOPY) -O binary $< $@
