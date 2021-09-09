#!/bin/sh

set -e

sdk_archive="$1"
dir=sdk
makefile=sdk.mk
makefile_prefix="SDK_"

srcfiles='
	platform/emlib/src/*.c
	platform/Device/SiliconLabs/EFM32GG/Source/system_efm32gg.c
	platform/Device/SiliconLabs/EFM32GG/Source/GCC/startup_efm32gg.c
'

incdirs='
	platform/CMSIS/Include/
	platform/emlib/inc/
	platform/Device/SiliconLabs/EFM32GG/Include/
	hardware/kit/common/bsp/
	hardware/kit/EFM32GG_STK3700/config/
'

ldscript='platform/Device/SiliconLabs/EFM32GG/Source/GCC/efm32gg.ld'

if [ ! "$sdk_archive" ]
then
	echo "usage: $0 sdk_archive [extra_paths]" >&2
	exit 1
fi
shift

rm -rf "$dir"
mkdir "$dir"
bsdtar -xvf "$sdk_archive" -C "$dir" $incdirs $srcfiles $ldscript "$@"

srcfiles=$(printf "$dir/%s\\n" $srcfiles)
incdirs=$(printf "$dir/%s\\n" $incdirs)
ldscript="$dir/$ldscript"

<<EOF cat >$makefile

${makefile_prefix}OBJS += \\
$(printf '\t%s\n' $srcfiles | sed 's|[.][^./]*$|.o\\|')

${makefile_prefix}INCDIRS += \\
$(printf '\t%s\\\n' $incdirs)

${makefile_prefix}LDSCRIPT = $ldscript

EOF

echo "generated makefile $makefile" >&2
