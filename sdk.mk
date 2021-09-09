
SDK_OBJS += \
	sdk/platform/emlib/src/em_acmp.o\
	sdk/platform/emlib/src/em_adc.o\
	sdk/platform/emlib/src/em_aes.o\
	sdk/platform/emlib/src/em_assert.o\
	sdk/platform/emlib/src/em_burtc.o\
	sdk/platform/emlib/src/em_cmu.o\
	sdk/platform/emlib/src/em_core.o\
	sdk/platform/emlib/src/em_cryotimer.o\
	sdk/platform/emlib/src/em_crypto.o\
	sdk/platform/emlib/src/em_dac.o\
	sdk/platform/emlib/src/em_dbg.o\
	sdk/platform/emlib/src/em_dma.o\
	sdk/platform/emlib/src/em_ebi.o\
	sdk/platform/emlib/src/em_emu.o\
	sdk/platform/emlib/src/em_gpcrc.o\
	sdk/platform/emlib/src/em_gpio.o\
	sdk/platform/emlib/src/em_i2c.o\
	sdk/platform/emlib/src/em_idac.o\
	sdk/platform/emlib/src/em_int.o\
	sdk/platform/emlib/src/em_lcd.o\
	sdk/platform/emlib/src/em_ldma.o\
	sdk/platform/emlib/src/em_lesense.o\
	sdk/platform/emlib/src/em_letimer.o\
	sdk/platform/emlib/src/em_leuart.o\
	sdk/platform/emlib/src/em_mpu.o\
	sdk/platform/emlib/src/em_msc.o\
	sdk/platform/emlib/src/em_opamp.o\
	sdk/platform/emlib/src/em_pcnt.o\
	sdk/platform/emlib/src/em_prs.o\
	sdk/platform/emlib/src/em_rmu.o\
	sdk/platform/emlib/src/em_rtc.o\
	sdk/platform/emlib/src/em_rtcc.o\
	sdk/platform/emlib/src/em_system.o\
	sdk/platform/emlib/src/em_timer.o\
	sdk/platform/emlib/src/em_usart.o\
	sdk/platform/emlib/src/em_vcmp.o\
	sdk/platform/emlib/src/em_wdog.o\
	sdk/platform/Device/SiliconLabs/EFM32GG/Source/system_efm32gg.o\
	sdk/platform/Device/SiliconLabs/EFM32GG/Source/GCC/startup_efm32gg.o\

SDK_INCDIRS += \
	sdk/platform/CMSIS/Include/\
	sdk/platform/emlib/inc/\
	sdk/platform/Device/SiliconLabs/EFM32GG/Include/\
	sdk/hardware/kit/common/bsp/\
	sdk/hardware/kit/EFM32GG_STK3700/config/\

SDK_LDSCRIPT = sdk/platform/Device/SiliconLabs/EFM32GG/Source/GCC/efm32gg.ld

