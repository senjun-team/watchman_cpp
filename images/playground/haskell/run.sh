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


# clear previous container use
# rm -rf /home/code_runner > /dev/null 2>&1

# run user code
f="$(basename -- $file)"

cp /home/code_runner/$f /home/code_runner/user-code/app/Main.hs

# go to /home/code_runner/user_code for stack compiling and running
cd /home/code_runner/user-code

f_capture="/tmp/capture.txt"

if ! ( timeout 10s stack build --verbosity warn --ghc-options '-fno-warn-missing-export-lists -Wno-type-defaults' && stack exec user-code-exe  | tee $f_capture ); then
   echo user_solution_error_f936a25e
   exit
fi
echo user_code_ok_f936a25e
echo user_solution_ok_f936a25e
