#!/usr/bin/sed

# modify .sh files
s/cmake -DCMAKE_TOOLCHAIN_FILE/\
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE/

/^make -j.*/a\
mv compile_commands.json ..
