#!/usr/bin/env bash

# clear previous container use
rm -rf /home/code_runner > /dev/null 2>&1

timeout 10s python $1
echo $?
