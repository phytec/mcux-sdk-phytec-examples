#!/usr/bin/bash

find . -path "*/*.sh" | xargs sed -i -f scripts/autogen_compile_commands.sed
