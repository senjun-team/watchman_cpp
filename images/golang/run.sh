#!/usr/bin/env bash

# parse flags - single letters prefixed fith hyphen before each argument
# example: sh run.sh -f task
while getopts c:j:f:t: flag
do
    case "${flag}" in
        c) color=${OPTARG};;
        j) jobs=${OPTARG};;
        f) file=${OPTARG};;
        t) type_check=${OPTARG};;
    esac
done

f="$(basename -- $file)"

if [ "$f" = "task" ]; then
    # clear previous container use
    rm -rf /home/code_runner > /dev/null 2>&1
    set -e
    cp $file /home/code_runner/user-code/main.go
    cd /home/code_runner/user-code
    # we call gofmt to prevent compiler errors:
    # go compiler treats formatting errors as compilation errors!

    # TODO: format go code in online IDE to show user the right way
    timeout 5s gofmt -w main.go
    timeout 10s go run .
else
    # don't clear previous container use because 
    # we don't want to erase main.go
    set -e
    cp $file /home/code_runner/user-code/main_test.go
    cd /home/code_runner/user-code
    # here we don't call gofmt because we rely on the fact that 
    # the tests are already formatted
    timeout 10s go test
fi

echo $?
