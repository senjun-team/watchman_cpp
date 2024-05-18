#!/usr/bin/env bash

while getopts f: flag
do
    case "${flag}" in
        f) file=${OPTARG};;
    esac
done
rm -rf /home/code_runner > /dev/null 2>&1

timeout 10s python $file
if [ "$?" -ne 0 ]; then
  echo user_solution_error_f936a25e
  exit
fi

echo user_code_ok_f936a25e
