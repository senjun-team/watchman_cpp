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
cp $f main.cpp # otherwise impossible to compile

cd /home/code_runner

executable="main"
timeout 10s g++ -o $executable main.cpp

if ! ( timeout 10s "./$executable" ); then
   echo user_solution_error_f936a25e
   exit
fi
echo user_code_ok_f936a25e

f="$(basename -- ${file}_tests)"
cp $f main.cpp # otherwise impossible to compile
timeout 10s g++ -o $executable main.cpp

if ! ( timeout 10s "./$executable" ); then
   echo tests_cases_error_f936a25e
   exit
fi

echo user_solution_ok_f936a25e
