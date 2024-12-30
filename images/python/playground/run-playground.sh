#!/usr/bin/env bash

while getopts f: flag
do
    case "${flag}" in
        f) project=${OPTARG};;
    esac
done

f="$(basename -- $project)"

required_file="main.py"

timeout 10s python /home/code_runner/playground/$f/$required_file
if [ "$?" -ne 0 ]; then
  echo user_solution_error_f936a25e
  exit
fi

echo user_solution_ok_f936a25e
