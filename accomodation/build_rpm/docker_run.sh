#!/bin/bash

docker run -it  \
  -v $(pwd)/../..:/mnt/watchman_cpp \
  watchman_cpp_rpm