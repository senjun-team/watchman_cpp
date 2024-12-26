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

# if exists file with user code
if [ $task_type = "code" ]; then
   touch main.cpp
   cp "$(basename -- $file)" main.cpp
fi

cp "$(basename -- ${file}_tests)" tests.cpp

if ! ( timeout 10s cmake -Wno-dev -Bbuild -GNinja > /tmp/configure.txt ); then
   cat /tmp/configure.txt
   echo user_solution_error_f936a25e
   exit
fi

if ! ( timeout 20s cmake --build build/ -- -j4  > /tmp/build.txt ); then
   cat /tmp/build.txt
   echo user_solution_error_f936a25e
   exit
fi

if [ $task_type = "code" ]; then
   if ! ( timeout 4s ./build/main ); then
      echo user_solution_error_f936a25e
      exit
   fi
fi

echo user_code_ok_f936a25e

if ! ( timeout 4s ./build/tests ); then
   echo tests_cases_error_f936a25e
   exit
fi

echo user_solution_ok_f936a25e