#!/usr/bin/env bash

# parse flags - single letters prefixed with hyphen before each argument
# example: sh run.sh -f project_dir
while getopts c:j:f:t:v: flag
do
    case "${flag}" in
        c) color=${OPTARG};;
        j) jobs=${OPTARG};;
        f) project=${OPTARG};;
        t) type_check=${OPTARG};;
        v) task_type=${OPTARG};;
    esac
done

f="$(basename -- $project)"

if ! ( timeout 5s cmake -G Ninja -B /home/code_runner/build /home/code_runner/$f > /tmp/configure.txt ); then
   cat /tmp/configure.txt
   echo user_solution_error_f936a25e
   exit
fi

if ! ( timeout 10s cmake -G Ninja --build /home/code_runner/build > /tmp/build.txt ); then
   cat /tmp/build.txt
   echo user_solution_error_f936a25e
   exit
fi

if ! ( timeout 5s /home/code_runner/build/cpp_experiments ); then
   echo user_solution_error_f936a25e
   exit
fi

echo user_solution_ok_f936a25e
