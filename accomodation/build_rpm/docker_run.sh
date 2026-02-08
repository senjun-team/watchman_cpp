#!/bin/bash

docker run -it  \
  -v $(pwd)/../..:/mnt/watchman_cpp:z \
  watchman_cpp_rpm
