#!/bin/bash

cmake -S . -B build/ -DCMAKE_EXPORT_COMPILE_COMMANDS=On

if [ ! -L "compile_commands.json" ]; then
    ln -s build/compile_commands.json .
fi

cmake --build build/ --config Debug -j20
