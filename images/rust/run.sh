#!/bin/bash

set -e

# TODO think about cp, should be mv!
cp $1 /home/code_runner/user-code/src
cd /home/code_runner/user-code
f="$(basename -- $1)"

# TODO think about cp, should be mv!
cp -f src/"$f" "src/main.rs"

timeout 10s cargo clean && cargo run -r -q --offline
echo $?
