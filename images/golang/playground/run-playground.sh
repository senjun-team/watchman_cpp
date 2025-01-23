#!/usr/bin/env bash

# parse flags - single letters prefixed fith hyphen before each argument
# example: sh run.sh -f task
while getopts f: flag
do
    case "${flag}" in
        f) file=${OPTARG};;
    esac
done

# prepare project
f="$(basename -- $file)"
cd /home/code_runner/playground/$f

# we call gofmt to prevent compiler errors:
# go compiler treats formatting errors as compilation errors!

# TODO: format go code in online IDE to show user the right way
timeout 5s gofmt -s -w /home/code_runner/playground/$f

# build and run user code
if ! ( timeout 10s go run /home/code_runner/playground/$f/cmd); then
   echo user_solution_error_f936a25e
   exit
fi
echo user_solution_ok_f936a25e
