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

# clear previous container use
rm -rf /home/code_runner > /dev/null 2>&1

# check script correctness
timeout 10s python $file
if [ "$?" -ne 0 ]; then
    echo user_solution_error_f936a25e
    exit
fi

echo user_code_ok_f936a25e

timeout 10s python ${file}_tests
if [ "$?" -ne 0 ]; then
    echo tests_cases_error_f936a25e
    exit
fi

echo user_solution_ok_f936a25e
