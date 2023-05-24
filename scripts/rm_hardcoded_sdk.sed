#!/usr/bin/sed

# modify .sh files
/^cmake.* -DCMAKE_TOOLCHAIN_FILE.*/i\
if [ -n ${MCUX_SDK_PATH} ]; then sdk_dir=${MCUX_SDK_PATH};\
else sdk_dir=../../../../..\
fi

s/CMAKE_TOOLCHAIN_FILE="..\/..\/..\/..\/../CMAKE_TOOLCHAIN_FILE="${sdk_dir}/

# modify CMakeLists.txt files
s/SET(SdkRootDirPath ${ProjDirPath}\/..\/..\/..\/..\/..)/\tif (DEFINED ENV{MCUX_SDK_PATH})\
\t\tSET(SdkRootDirPath $ENV{MCUX_SDK_PATH})\
\telse()\
\t\tSET(SdkRootDirPath ${ProjDirPath}\/..\/..\/..\/..\/..)\
\tendif()/
