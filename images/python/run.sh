#!/usr/bin/env bash

# remove all regular files except:
# * run.sh
# * task
# * task_tests
me=$(basename "$0")
find . \
    -type f \
    -not -name $me \
    -not -name "task" \
    -not -name "task_tests" -delete \

# remove all directories from current directory
find -mindepth 1 -maxdepth 1 -type d -print0 | xargs -r0 rm -R

timeout 10s python $1
echo $?
