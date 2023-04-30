#!/bin/bash

mv $1 /home/code_runner/user-code/src
cd /home/code_runner/user-code
f="$(basename -- $1)"

mv -f src/"$f" src/main.rs

timeout 10s cargo clean && cargo run -r -q --offline
echo $?
