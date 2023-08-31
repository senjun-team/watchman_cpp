#!/bin/bash

# parse flags - single letters prefixed fith hyphen before each argument
# example: sh run.sh -c never -j 4
while getopts c:j:f: flag
do
    case "${flag}" in
        c) color=${OPTARG};;
        j) jobs=${OPTARG};;
        f) file=${OPTARG};;
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

set -e

# TODO think about cp, should be mv!
cp $file /home/code_runner/user-code/src
cd /home/code_runner/user-code
f="$(basename -- $file)"

# TODO think about cp, should be mv!
cp -f src/"$f" "src/main.rs"

timeout 10s cargo clean && cargo run --release --quiet --offline $cargo_run_opts
echo $?
