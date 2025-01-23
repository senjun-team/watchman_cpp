#!/usr/bin/env bash

# parse flags - single letters prefixed with hyphen before each argument
# example: sh run.sh -f project_dir
while getopts f: flag
do
    case "${flag}" in
        f) project=${OPTARG};;
    esac
done

f="$(basename -- $project)"

cd /home/code_runner/playground/$f 

# configure project
if ! ( timeout 5s cmake -Bbuild -Wno-dev -GNinja > /tmp/configure.txt ); then
   cat /tmp/configure.txt
   echo user_solution_error_f936a25e
   exit
fi

# build cpp project
if ! ( timeout 10s cmake --build build/ -- -j4  > /tmp/build.txt ); then
   cat /tmp/build.txt
   echo user_solution_error_f936a25e
   exit
fi

# run cpp project
if ! ( timeout 5s ./build/cpp_experiments ); then
   echo user_solution_error_f936a25e
   exit
fi

echo user_solution_ok_f936a25e
