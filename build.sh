#!/bin/bash
DISABLE_RTTI_EXCEPTIONS="-fno-exceptions -fno-rtti -O0"
DISABLED_WARNINGS="-Wno-write-strings -Wno-sign-compare -Wno-writable-strings -Wno-missing-braces -Wno-unused-function -Wno-format -Wno-switch -Wno-char-subscripts"

INCLUDE_DIR="-I src/irlibs"


clang++ src/main.cpp -o mc -g -Wall -O0 $INCLUDE_DIR $DISABLE_RTTI_EXCEPTIONS $DISABLED_WARNINGS