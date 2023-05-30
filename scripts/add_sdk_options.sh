#!/usr/bin/bash

files=$(find . -path "*/*.sh")
for filename in ${files}; do
    replace_str=$(rg -o -I -N '(\.\.)?/\.\.(/\.\.)+' $filename | uniq)
    if [ -n "$replace_str" ]; then
        sed -i '/^cmake.* -DCMAKE_TOOLCHAIN_FILE.*/i\
	    if [ -n ${MCUX_SDK_PATH} ]; then\
		sdk_dir=${MCUX_SDK_PATH};\
            else sdk_dir='${replace_str}'\
            fi' ${filename}

        replace_str=$(echo $replace_str | sed 's/\//\\\//g')

        sed -i 's/CMAKE_TOOLCHAIN_FILE="'${replace_str}'/CMAKE_TOOLCHAIN_FILE="${sdk_dir}/' ${filename}
    fi
done

files=$(find . -path "*/CMakeLists.txt")

for filename in ${files}; do
    replace_str=$(rg -o -I -N '(\.\.)?/\.\.(/\.\.)+' $filename | uniq | sed 's/\//\\\//g')
    if [ -n "$replace_str" ]; then

    sed -i 's/SET(SdkRootDirPath ${ProjDirPath}'${replace_str}')/if (DEFINED ENV{MCUX_SDK_PATH})\
        SET(SdkRootDirPath $ENV{MCUX_SDK_PATH})\
    else()\
        SET(SdkRootDirPath ${ProjDirPath}'${replace_str}')\
        endif()/' ${filename}

fi
done
