#!/bin/bash

if [ "$VRAY_SDK" = "" ]
then
	echo VRAY_SDK environment variable not defined.
	exit
fi

CPP_OPTIONS="-O2 -ffast-math -ffunction-sections -fdata-sections -fgcse-after-reload -finline-functions -funswitch-loops -fPIC -DREQUIRE_IOSTREAM -DBits64_ -D_REENTRANT -DVRAY_EXPORTS -fvisibility=hidden -fexceptions -Wno-write-strings"

LINK_OPTIONS="-shared -lpthread -lrt -lc -lz -lvray -lplugman_s -lvutils_s -Wl,-discard-all -Wl,--export-dynamic"

OUTPUT_PATH="./bin"

OUTPUT="$OUTPUT_PATH/libvray_BRDFAlSurface.so"

if [ ! -e $OUTPUT_PATH ]
then
	echo $OUTPUT_PATH does not exist, creating...
	mkdir $OUTPUT_PATH
fi

g++ $CPP_OPTIONS -I $VRAY_SDK/include -I $VRAY_SDK/samples/vray_plugins/brdfs/vray_brdfbump vray_brdfalsurface.cpp ../common/albrdf.cpp ../common/beckmann.cpp ../common/sss.cpp -L $VRAY_SDK/lib/linux_x64/gcc-4.4 $LINK_OPTIONS -o $OUTPUT

