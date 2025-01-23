#!/usr/bin/env bash

# TODO test scripts with proper data from watchman

# `run` parameter
run_function() {
  path_to_main=$1
  options=$2

  filename=$(basename -- "$path_to_main")
  dirname="$(dirname "$path_to_main")"
  cd $dirname
  
  timeout 10s python $filename $options
  if [ "$?" -ne 0 ]; then
    echo user_solution_error_f936a25e
    exit
  fi

  echo user_code_ok_f936a25e # TODO crutch, make it better
  echo user_solution_ok_f936a25e
}

# `test` parameter
test_function() {
  project_directory=$1

  echo user_code_ok_f936a25e # TODO crutch, make it better

  cd /home/code_runner/practice/$project_directory
  timeout 10s python -m unittest
  if [ "$?" -ne 0 ]; then
   echo tests_cases_error_f936a25e
   exit
  fi

  echo user_solution_ok_f936a25e
}

# main goes here

# column after option means required argument
while getopts f:p:o:rt opt
do
    case "${opt}" in
        f) project=${OPTARG}
          ;;
        o) user_options="${OPTARG}"
          ;;
        p) main_path=${OPTARG}
          ;;
        r) run=${OPTARG}
          ;;
        t) test=${OPTARG}
          ;;
        \?) echo "Invalid option" >&2
          exit
          ;;
    esac
done

# e.g: sh run.sh -p path_to_main.py -o user_options -r
if [ ${run+x} ]; then
  run_function $main_path "$user_options";
fi

# e.g: sh run.sh -f dir_name -t
if [ ${test+x} ]; then
  test_function $project;
fi
