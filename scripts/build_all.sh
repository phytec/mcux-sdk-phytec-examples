#!/bin/bash
OPTIONS="c"
DO_CLEAN=false
while getopts ${OPTIONS} option
do
	case ${option} in
		c) DO_CLEAN=true;;
		\?) echo "Unknown option: -$OPTARG" >&2; exit 1;;
		:) echo "Missing option argument for -$OPTARG" >&2; exit 1;;
		*) echo "Unimplemented option: -$OPTARG" >&2; exit 1;;
	esac
done

#check ENV set
if [ -z ${MCUX_SDK_PATH} ]; then
		echo "Define MCUX_SDK_PATH if not build from west setup (out-of-tree)"
fi
if [ -z ${ARMGCC_DIR} ]; then
	echo "Define ARMGCC_DIR"
	exit 0
fi
#check to call it from example root (works also for symlinks)
base_folder=$(dirname "$(readlink -f "$0")")/..
echo ${base_folder}
cd ${base_folder}

build_folders=$(find . -path "*/armgcc")
command="./build_all.sh"
if ${DO_CLEAN}; then
	command="./clean.sh"
fi

for bf in ${build_folders}; do
	echo "======= Run ${command} on ${bf} ========"
	cd ${bf}
	$command 2>&1 > /dev/null
	cd - >/dev/null
done
