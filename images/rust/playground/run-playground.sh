#!/bin/bash

# parse flags - single letters prefixed fith hyphen before each argument
# example: sh run.sh -c never -j 4
while getopts c:j:f:v: flag
do
    case "${flag}" in
        c) color=${OPTARG};;
        j) jobs=${OPTARG};;
        f) project=${OPTARG};;
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

p="$(basename -- $project)"
cd /home/code_runner/playground/$p

if ! ( timeout 10s cargo clean --quiet && cargo run --release --quiet --offline $cargo_run_opts ); then
   echo user_solution_error_f936a25e
   exit
fi
echo user_solution_ok_f936a25e
