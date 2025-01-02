#!/usr/bin/env bash

# parse flags - single letters prefixed fith hyphen before each argument
# example: sh run.sh -f task -v code

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

cd /home/code_runner/task

# if exists file with user code
if [ $task_type = "code" ]; then
        if [ ! -z "$type_check" ]; then
            timeout 5s mypy --strict $file
            if [ "$?" -ne 0 ]; then
                echo user_solution_error_f936a25e
                exit
            fi
        fi

        # check script correctness
        timeout 10s python $file
        if [ "$?" -ne 0 ]; then
            echo user_solution_error_f936a25e
            exit
        fi

        echo user_code_ok_f936a25e
else
    echo user_code_ok_f936a25e
fi


timeout 10s python ${file}_tests
if [ "$?" -ne 0 ]; then
    echo tests_cases_error_f936a25e
    exit
fi

echo user_solution_ok_f936a25e
