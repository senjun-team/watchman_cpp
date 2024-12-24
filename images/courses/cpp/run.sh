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

cd /home/code_runner

executable="main"
f="$(basename -- $file)"


# if exists file with user code
if [ $task_type = "code" ]; then
    cp $f main.cpp # otherwise impossible to compile

    timeout 10s g++ -o $executable main.cpp -std=c++23

    if ! ( timeout 10s "./$executable" ); then
        echo user_solution_error_f936a25e
        exit
    fi
    echo user_code_ok_f936a25e
else
    echo user_code_ok_f936a25e
fi


f="$(basename -- ${file}_tests)"
cp $f main.cpp # otherwise impossible to compile

rm -rf build > /dev/null

if ! ( cmake -B build -G Ninja -Wno-dev > /dev/null ); then
   echo user_solution_error_f936a25e
   exit
fi

if ! ( cmake --build build -- -j4 > /dev/null ); then
   echo user_solution_error_f936a25e
   exit
fi

if ! ( timeout 10s "build/$executable" ); then
   echo tests_cases_error_f936a25e
   exit
fi

echo user_solution_ok_f936a25e
