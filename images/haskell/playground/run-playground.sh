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

f="$(basename -- $file)"

cp -r /home/code_runner/playground/$f/app /home/code_runner/user-code/
cp -r /home/code_runner/playground/$f/src /home/code_runner/user-code/
cp -r /home/code_runner/playground/$f/test /home/code_runner/user-code/

cd /home/code_runner/user-code/

f_capture="/tmp/capture.txt"

if ! ( timeout 10s stack build --verbosity warn  --allow-different-user  --ghc-options '-fno-warn-missing-export-lists -Wno-type-defaults' && stack exec user-code-exe  | tee $f_capture ); then
   echo user_solution_error_f936a25e
   exit
fi
echo user_solution_ok_f936a25e
