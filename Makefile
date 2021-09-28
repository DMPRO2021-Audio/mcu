.POSIX:

OBJS = main.o synth.o

include sdk.mk

MACHFLAGS = -mcpu=cortex-m3 -mthumb
CFLAGS += $(MACHFLAGS) -D EFM32GG990F1024 -I config/ $(SDK_INCDIRS:%=-I '%')
LDFLAGS += $(MACHFLAGS) -T$(SDK_LDSCRIPT)
ARFLAGS += -U

CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
SIMPLICITY_COMMANDER ?= commander

program.bin: program.elf

program.elf: $(OBJS) libsdk.a
	$(CC) $(LDFLAGS) -o '$@' $(OBJS) -L . -l sdk

libsdk.a: ${SDK_OBJS:%=libsdk.a(%)}

.PHONY: clean flash

clean:
	-rm -rf deps/
	-rm -f program.bin program.elf libsdk.a $(OBJS) $(SDK_OBJS)

flash: program.bin
	$(SIMPLICITY_COMMANDER) flash program.bin

.SUFFIXES: .bin .elf .c .o .a

.elf.bin:
	$(OBJCOPY) -O binary '$<' '$@'

.c.o:
	@mkdir -p "$$(dirname 'deps/$*.d')"
	$(CC) -MMD -MT '$@' -MP -MF 'deps/$*.d' $(CFLAGS) -c -o '$@' '$<'

.c.a:
	@mkdir -p "$$(dirname 'deps/$@/$*.d')"
	$(CC) -MMD -MT '$@($*.o)' -MP -MF 'deps/$@/$*.d' $(CFLAGS) -c -o '$*.o' '$<'
	$(AR) $(ARFLAGS) '$@' '$*.o'
	rm -f '$*.o'

-include $(OBJS:%.o=deps/%.d)
-include $(SDK_OBJS:%.o=deps/libsdk.a/%.d)
