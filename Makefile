.POSIX:

OBJS = \
    main.o\
    synth.o\
    usart.o\
    circular_buffer.o\
    button.o\
    timer.o\
    arpeggiator.o\
    leds.o\
    sound.o\
    channel.o

#### for EFM32GG990F1024 (Giant Gecko dev kit) ####

#OBJS += \
#    sdk/platform/Device/SiliconLabs/EFM32GG/Source/system_efm32gg.o\
#    sdk/platform/Device/SiliconLabs/EFM32GG/Source/GCC/startup_efm32gg.o
#MACHFLAGS = -mcpu=cortex-m3 -mthumb -specs=nano.specs
#CFLAGS += -D EFM32GG990F1024 -I sdk/platform/Device/SiliconLabs/EFM32GG/Include/ -g3 -O0
#LDFLAGS += -Tsdk/platform/Device/SiliconLabs/EFM32GG/Source/GCC/efm32gg.ld

#### for EFM32GG12B810F1024 (Thunderboard and PCB) ####

OBJS += \
    sdk/platform/Device/SiliconLabs/EFM32GG12B/Source/system_efm32gg12b.o\
    sdk/platform/Device/SiliconLabs/EFM32GG12B/Source/GCC/startup_efm32gg12b.o
MACHFLAGS = -mcpu=cortex-m4 -mthumb -specs=nano.specs
CFLAGS += -D EFM32GG12B810F1024GQ64 -I sdk/platform/Device/SiliconLabs/EFM32GG12B/Include/ -g3 -O0
LDFLAGS += -Tsdk/platform/Device/SiliconLabs/EFM32GG12B/Source/GCC/efm32gg12b.ld

#### choose one ####

CFLAGS += -Wall -pedantic
CFLAGS += $(MACHFLAGS) -I config/ -g
LDFLAGS += $(MACHFLAGS) -L sdk/platform/CMSIS/Lib/
ARFLAGS += -U

CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
SIMPLICITY_COMMANDER ?= commander

CLEAN += program.bin program.elf $(OBJS)

ifndef DEVKIT
	DEVICE = --device EFM32GG12B810F1024GQ64
endif

program.bin: program.elf

program.elf: $(OBJS) libspidrv.a libgpiointerrupt.a libdmadrv.a libemlib.a
	$(CC) $(LDFLAGS) -o '$@' $(OBJS) -lm -L . -l spidrv -l gpiointerrupt -l dmadrv -l emlib

-include $(OBJS:%.o=deps/%.d)

.PHONY: clean flash

clean:
	-rm -rf deps/
	-rm -f $(CLEAN)

flash: program.bin
	$(SIMPLICITY_COMMANDER) flash $(DEVICE) program.bin

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
