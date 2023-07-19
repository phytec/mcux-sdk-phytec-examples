#!/usr/bin/bash

search_patterns=("*/board.c" "*/board.h" "*/clock_config.c")

for search in "${search_patterns[@]}"; do

	echo "===== Compare $search ====="

	readarray -d '' DIFF_FILES < <(find $1 -path "$search" -print0)
	FILE1="${DIFF_FILES[0]}"
	#echo "${DIFF_FILES[*]}"

	for file in "${DIFF_FILES[@]}"; do
		echo "diff $FILE1 $file"
		diff "$FILE1" "$file"
	done
done
