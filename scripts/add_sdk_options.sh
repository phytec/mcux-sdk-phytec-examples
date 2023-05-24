#!/usr/bin/bash

find . -path "*/CMakeLists.txt" -o -path "*/*.sh" | xargs sed -i -f scripts/rm_hardcoded_sdk.sed
