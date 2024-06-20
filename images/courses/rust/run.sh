#!/bin/bash

# parse flags - single letters prefixed fith hyphen before each argument
# example: sh run.sh -c never -j 4
while getopts c:j:f:v: flag
do
    case "${flag}" in
        c) color=${OPTARG};;
        j) jobs=${OPTARG};;
        f) file=${OPTARG};;
        v) task_type=${OPTARG};;
    esac
done


# build 'cargo_run' command options
# example: --color never --jobs 5
cargo_run_opts=""

if [ ! -z "$color" ];
then
    cargo_run_opts="${cargo_run_opts} --color ${color}"
fi

if [ ! -z "$jobs" ]
then
    cargo_run_opts="${cargo_run_opts} --jobs ${jobs}"
fi

# clear previous container use
rm -rf /home/code_runner > /dev/null 2>&1

# run user code
f="$(basename -- $file)"

# TODO think about cp, should be mv! Now it is forbidden to modify files in /home/code_runner
cp /home/code_runner/$f /home/code_runner/user-code/src/main.rs

# go to /home/code_runner/user_code for cargo compiling and running
cd /home/code_runner/user-code

if ! ( timeout 10s cargo clean --quiet  && cargo run --release --quiet --offline $cargo_run_opts ); then
   echo user_solution_error_f936a25e
   exit
fi
echo user_code_ok_f936a25e

# run tests
f="$(basename -- ${file}_tests)"

# TODO think about cp, should be mv! Now it is forbidden to modify files in /home/code_runner
cp /home/code_runner/$f /home/code_runner/user-code/src/main.rs

if ! ( timeout 10s cargo clean --quiet && cargo run --release --quiet --offline $cargo_run_opts ); then
   echo tests_cases_error_f936a25e
   exit
fi

echo user_solution_ok_f936a25e
