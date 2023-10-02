#!/bin/sh
if [ -d "CMakeFiles" ];then rm -rf CMakeFiles; fi
if [ -f "Makefile" ];then rm -f Makefile; fi
if [ -f "cmake_install.cmake" ];then rm -f cmake_install.cmake; fi
if [ -f "CMakeCache.txt" ];then rm -f CMakeCache.txt; fi
if [ ! -z ${MCUX_SDK_PATH} ]; then
    sdk_dir=${MCUX_SDK_PATH}
else
    sdk_dir=../../../../../../..
fi
cmake -DCMAKE_TOOLCHAIN_FILE="${sdk_dir}/core/tools/cmake_toolchain_files/armgcc.cmake" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=flash_debug  .
make -j 2>&1 | tee build_log.txt
