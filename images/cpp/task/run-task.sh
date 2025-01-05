#!/usr/bin/env bash

# parse flags - single letters prefixed fith hyphen before each argument
# example: sh run.sh -f task
while getopts f:v: flag
do
    case "${flag}" in
        f) file=${OPTARG};;
        v) task_type=${OPTARG};;
    esac
done

# go to the task dir
cd /home/code_runner/task

# if exists file with user code
if [ $task_type = "code" ]; then
   touch main.cpp
   cp $file main.cpp
fi

cp ${file}_tests tests.cpp

# configure project
if ! ( timeout 10s cmake -Wno-dev -Bbuild -GNinja > /tmp/configure.txt ); then
   cat /tmp/configure.txt
   echo user_solution_error_f936a25e
   exit
fi

# build cpp project
if ! ( timeout 20s cmake --build build/ -- -j4  > /tmp/build.txt ); then
   cat /tmp/build.txt
   echo user_solution_error_f936a25e
   exit
fi

# check task "what will this code output?"
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