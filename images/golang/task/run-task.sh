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

cp /home/code_runner/task/$f /home/code_runner/task/user-code/main.go

cd /home/code_runner/task/user-code

# we call gofmt to prevent compiler errors:
# go compiler treats formatting errors as compilation errors!

# TODO: format go code in online IDE to show user the right way
timeout 5s gofmt -w main.go

if ! ( timeout 10s go run . ); then
   echo user_solution_error_f936a25e
   exit
fi
echo user_code_ok_f936a25e

f="$(basename -- ${file}_tests)"
# don't clear previous container use because
# we don't want to erase main.go
cp /home/code_runner/task/$f /home/code_runner/task/user-code/main_test.go
# here we don't call gofmt because we rely on the fact that
# the tests are already formatted
if ! ( timeout 10s go test -v); then
   echo tests_cases_error_f936a25e
   exit
fi

echo user_solution_ok_f936a25e
