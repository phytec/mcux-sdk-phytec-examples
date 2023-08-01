#!/usr/bin/bash

echo "===== Add options to build_*.sh ====="

files=$(find . -path "*armgcc/build_*.sh") # exclude scripts folder
for filename in ${files}; do
	if ! grep -m 1 -q "MCUX_SDK_PATH" "$filename"; then
		replace_str=$(rg -o -I -N '(\.\.)?/\.\.(/\.\.)+' $filename | uniq)
		if [ -n "$replace_str" ]; then
			echo "Replace SDK path in $filename"
			sed -i '/^cmake.* -DCMAKE_TOOLCHAIN_FILE.*/i\
if [ -n ${MCUX_SDK_PATH} ]; then\
    sdk_dir=${MCUX_SDK_PATH}\
else\
    sdk_dir='${replace_str}'\
fi' ${filename}

			replace_str=$(echo $replace_str | sed 's/\//\\\//g')

			sed -i 's/CMAKE_TOOLCHAIN_FILE="'${replace_str}'/CMAKE_TOOLCHAIN_FILE="${sdk_dir}/' ${filename}
		fi
	else
		echo "$filename already w replaced SDK path"
	fi
done

echo "===== Add options to CMakeLists ====="

files=$(find . -path "*/CMakeLists.txt")

for filename in ${files}; do
	if ! grep -m 1 -q "MCUX_SDK_PATH" "$filename"; then
	replace_str=$(rg -o -I -N '(\.\.)?/\.\.(/\.\.)+' $filename | uniq | sed 's/\//\\\//g')
		if [ -n "$replace_str" ]; then
			echo "Replace SDK path in $filename"
			sed -i 's/SET(SdkRootDirPath ${ProjDirPath}'${replace_str}')/if (DEFINED ENV{MCUX_SDK_PATH})\
        SET(SdkRootDirPath $ENV{MCUX_SDK_PATH})\
    else()\
        SET(SdkRootDirPath ${ProjDirPath}'${replace_str}')\
    endif()/' ${filename}

		fi
	else
		echo "$filename already w replaced SDK path"
	fi
done
