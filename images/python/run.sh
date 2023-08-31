#!/usr/bin/env bash

# parse flags - single letters prefixed fith hyphen before each argument
# example: sh run.sh -f task
while getopts c:j:f: flag
do
    case "${flag}" in
        c) color=${OPTARG};;
        j) jobs=${OPTARG};;
        f) file=${OPTARG};;
    esac
done

# clear previous container use
rm -rf /home/code_runner > /dev/null 2>&1

timeout 10s python $file
echo $?
