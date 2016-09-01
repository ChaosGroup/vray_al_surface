#!/bin/bash

OUTPUT_PATH="./bin"
OUTPUT="${OUTPUT_PATH}/libvray_BRDFAlSurface.so"

if [ "${VRAY_SDK}" = "" ]
then
	VRAY_SDK=/Applications/ChaosGroup/V-Ray/Maya2016
	echo VRAY_SDK environment variable is not defined.
	echo Using default value: ${VRAY_SDK}
fi

if [ ! -e ${OUTPUT_PATH} ]
then
	echo ${OUTPUT_PATH} does not exist, creating...
	mkdir ${OUTPUT_PATH}
fi

clang++ -stdlib=libstdc++ -std=c++0x \
\
-Ofast -ffast-math -ffunction-sections -fdata-sections -fPIC -fvisibility=hidden -fexceptions -w \
\
-DREQUIRE_IOSTREAM -DBits64_ -D_REENTRANT -DVRAY_EXPORTS \
\
-I${VRAY_SDK}/include \
-I${VRAY_SDK}/samples/vray_plugins/brdfs/vray_brdfbump \
\
../common/albrdf.cpp \
../common/beckmann.cpp \
../common/sss.cpp \
vray_brdfalsurface.cpp \
\
-L${VRAY_SDK}/lib/mavericks_x64/gcc-4.2 \
-lvray \
-lplugman_s \
-lvutils_s \
-framework Cocoa \
\
-shared -lpthread -lc -lz \
\
-o ${OUTPUT}
