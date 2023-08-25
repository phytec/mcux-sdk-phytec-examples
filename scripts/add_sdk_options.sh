#!/usr/bin/bash
OPTIONS="hgv"

display_help() {
	echo "Usage: $0 [-g] [-v ]"
	echo ""
	echo "-g: Adopt for using Github SDK"
	echo "-V: Verbose output"
}

#TODO add option for SDK path/variable?
PREP_GITHUB_SDK=false
MCUX_PATH_ENV_VAR="MCUX_SDK_PATH"
VERBOSE=false

print_verb() {
	if ${VERBOSE}; then
		echo $1
	else
		echo -n "."
	fi
}

replace_mcu_id() {

	filename=$1
	append_mcux_dev=$2

	if [[ "$filename" == *"phyboard-pollux"* ]]; then
		print_verb "$filename is Pollux"
		mcu_id="MIMX8ML8"
	elif [[ "$filename" == *"phyboard-polis"* ]]; then
		print_verb "$filename is Polis"
		mcu_id="MIMX8MM6"
	fi
	# remove mcu_id string (incl leading _, there may be occurences of mcu_id
	# in path as well)
	sed -i "s/_${mcu_id}//g" ${filename}

	# re-replace some occurences (as negative lookbehind seems not to work in sed,
	# i.e. (?<!misc_utilities)_MIMX8ML8 )
	sed -i "s/middleware_freertos-kernel)/middleware_freertos-kernel_${mcu_id})/g" ${filename}
	sed -i "s/misc_utilities)/misc_utilities_${mcu_id})/g" ${filename}


	if $2 ; then
		echo "set(MCUX_DEVICE \"$mcu_id\")" >> $filename
	fi
}

while getopts ${OPTIONS} option
do
	case ${option} in
		g) PREP_GITHUB_SDK=true;;
		v) VERBOSE=true;;
		h) display_help; exit;;
		\?) echo "Unknown option: -$OPTARG" >&2; exit 1;;
		:) echo "Missing option argument for -$OPTARG" >&2; exit 1;;
		*) echo "Unimplemented option: -$OPTARG" >&2; exit 1;;
	esac
done


echo "===== Add options to build_*.sh ====="

files=$(find . -path "*armgcc/build_*.sh") # exclude scripts folder
for filename in ${files}; do
	if ! grep -m 1 -q "${MCUX_PATH_ENV_VAR}" "${filename}"; then
		replace_str=$(rg -o -I -N '(\.\.)?/\.\.(/\.\.)+' ${filename} | uniq)
		if [ -n "${replace_str}" ]; then
			print_verb "Replace SDK path in ${filename}"
			sed -i '/^cmake.* -DCMAKE_TOOLCHAIN_FILE.*/i\
if [ ! -z ${'${MCUX_PATH_ENV_VAR}'} ]; then\
    sdk_dir=\${'${MCUX_PATH_ENV_VAR}'}\
else\
    sdk_dir='${replace_str}'\
fi' ${filename}

			replace_str=$(echo ${replace_str} | sed 's/\//\\\//g')

			sed -i 's/CMAKE_TOOLCHAIN_FILE="'${replace_str}'/CMAKE_TOOLCHAIN_FILE="${sdk_dir}\/core/' ${filename}
		fi
	else
		! ${VERBOSE} && echo ""
		echo -n "${filename} already w replaced SDK path"
	fi
done
! ${VERBOSE} && echo ""

echo "===== Add options to CMakeLists ====="

files=$(find . -path "*/CMakeLists.txt")

for filename in ${files}; do
	if ! grep -m 1 -q "${MCUX_PATH_ENV_VAR}" "${filename}"; then
	replace_str=$(rg -o -I -N '(\.\.)?/\.\.(/\.\.)+' ${filename} | uniq | sed 's/\//\\\//g')
		if [ -n "${replace_str}" ]; then
			print_verb "Replace SDK path in ${filename}"
			sed -i 's/SET(SdkRootDirPath ${ProjDirPath}'${replace_str}')/if ((DEFINED ENV{'${MCUX_PATH_ENV_VAR}'}) AND (NOT ("$ENV{'${MCUX_PATH_ENV_VAR}'}" STREQUAL "")))\
        SET(SdkRootDirPath $ENV{'${MCUX_PATH_ENV_VAR}'})\
    else()\
        SET(SdkRootDirPath ${ProjDirPath}'${replace_str}')\
    endif()/' ${filename}

		fi

		if ${PREP_GITHUB_SDK}; then
			# this part is only relevant for github SDK
			replace_mcu_id ${filename} false


			# Remove some entries from CMAKE_MODULE_PATH (without replacement), ...
			sed -i '/\${SdkRootDirPath}\/devices\/M.*\/utilities\/debug_console_lite/d' ${filename}

			# ... add various entries to CMAKE_MODULE_PATH, some specifically replacing old ones,...

			sed -i 's/\${SdkRootDirPath}\/devices\/M.*\/utilities$/${SdkRootDirPath}\/core\/utilities\
    ${SdkRootDirPath}\/core\/utilities\/misc_utilities/' ${filename}

			# just add /core
			sed -i 's/\${SdkRootDirPath}\/devices/${SdkRootDirPath}\/core\/devices/g' ${filename}

			sed -i 's/\${SdkRootDirPath}\/components/\${SdkRootDirPath}\/core\/components/g' ${filename}

			sed -i 's/\${SdkRootDirPath}\/CMSIS/\${SdkRootDirPath}\/core\/CMSIS/g' ${filename}

			# ... and add some common ones unconditionally TODO do this more conditionally -> related to include(x)?! (and/or append, not prepend?)
			sed -i '/set(CMAKE_MODULE_PATH/a\
    ${SdkRootDirPath}\/core\/drivers\/common\
    ${SdkRootDirPath}\/core\/drivers\/ecspi\
    ${SdkRootDirPath}\/core\/drivers\/flexcan\
    ${SdkRootDirPath}\/core\/drivers\/gpt\
    ${SdkRootDirPath}\/core\/drivers\/igpio\
    ${SdkRootDirPath}\/core\/drivers\/ii2c\
    ${SdkRootDirPath}\/core\/drivers\/ipwm\
    ${SdkRootDirPath}\/core\/drivers\/iuart\
    ${SdkRootDirPath}\/core\/drivers\/mu\
    ${SdkRootDirPath}\/core\/drivers\/rdc\
    ${SdkRootDirPath}\/core\/drivers\/sdma\
    ${SdkRootDirPath}\/core\/drivers\/tmu_1\
    ${SdkRootDirPath}\/core\/drivers\/tmu_2\
    ${SdkRootDirPath}\/core\/drivers\/wdog01\
    ${SdkRootDirPath}\/core\/utilities\/assert' ${filename}

			#last but not least remove trailing whitespaces
			sed -i 's/[ \t]*$//' ${filename}
		fi

	else
		! ${VERBOSE} && echo ""
		echo -n "${filename} already w replaced SDK path"
	fi
done
! ${VERBOSE} && echo ""

# this part is only relevant for github SDK (as some parts in CMakeList)
if ${PREP_GITHUB_SDK}; then
	! ${VERBOSE} && echo ""
	echo "===== Add option to config.cmake ====="

	files=$(find . -path "*/config.cmake")

	for filename in ${files}; do
		if ! grep -m 1 -q MCUX_DEVICE "${filename}"; then
			replace_mcu_id ${filename} true
		else
			! ${VERBOSE} && echo ""
			echo -n "${filename} already github SDK compatible"
		fi
	done
	! ${VERBOSE} && echo ""
fi
