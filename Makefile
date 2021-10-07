.POSIX:

OBJS = main.o synth.o

#### for EFM32GG990F1024 (Giant Gecko dev kit) ####

OBJS += \
    sdk/platform/Device/SiliconLabs/EFM32GG/Source/system_efm32gg.o\
    sdk/platform/Device/SiliconLabs/EFM32GG/Source/GCC/startup_efm32gg.o
MACHFLAGS = -mcpu=cortex-m3
CFLAGS += -D EFM32GG990F1024 -I sdk/platform/Device/SiliconLabs/EFM32GG/Include/
LDFLAGS += -Tsdk/platform/Device/SiliconLabs/EFM32GG/Source/GCC/efm32gg.ld
LIBM = arm_cortexM3l_math

#### for EFM32GG12B810F1024 (Thunderboard and PCB) ####

#OBJS += \
#    sdk/platform/Device/SiliconLabs/EFM32GG12B/Source/system_efm32gg12b.c\
#    sdk/platform/Device/SiliconLabs/EFM32GG12B/Source/GCC/startup_efm32gg12b.c
#MACHFLAGS = -mcpu=cortex-m4
#CFLAGS += -D EFM32GG12B810F1024 -I sdk/platform/Device/SiliconLabs/EFM32GG12B/Include/
#LDFLAGS += -Tsdk/platform/Device/SiliconLabs/EFM32GG12B/Source/GCC/efm32gg12b.ld
#LIBM = arm_cortexM4l_math

#### choose one ####

CFLAGS += $(MACHFLAGS) -I config/ -g
LDFLAGS += $(MACHFLAGS) -L sdk/platform/CMSIS/Lib/
ARFLAGS += -U

CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
SIMPLICITY_COMMANDER ?= commander

CLEAN += program.bin program.elf $(OBJS)

program.bin: program.elf

program.elf: $(OBJS) libspidrv.a libgpiointerrupt.a libdmadrv.a libemlib.a
	$(CC) $(LDFLAGS) -o '$@' $(OBJS) -L . -l spidrv -l gpiointerrupt -l dmadrv -l emlib

-include $(OBJS:%.o=deps/%.d)

.PHONY: clean flash

clean:
	-rm -rf deps/
	-rm -f $(CLEAN)

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

include sdk.mk
