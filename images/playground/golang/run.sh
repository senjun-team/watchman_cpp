#!/usr/bin/env bash

# parse flags - single letters prefixed fith hyphen before each argument
# example: sh run.sh -f task
while getopts c:j:f:t:v: flag
do
    case "${flag}" in
        c) color=${OPTARG};;
        j) jobs=${OPTARG};;
        f) file=${OPTARG};;
        t) type_check=${OPTARG};;
        v) task_type=${OPTARG};;
    esac
done

f="$(basename -- $file)"

# clear previous container use
rm -rf /home/code_runner > /dev/null 2>&1

cp /home/code_runner/$f /home/code_runner/user-code/main.go

cd /home/code_runner/user-code

# we call gofmt to prevent compiler errors:
# go compiler treats formatting errors as compilation errors!

# TODO: format go code in online IDE to show user the right way
timeout 5s gofmt -w main.go

if ! ( timeout 10s go run . ); then
   echo user_solution_error_f936a25e
   exit
fi
echo user_solution_ok_f936a25e
