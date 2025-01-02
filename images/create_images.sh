#!/bin/bash
set -e

create_image () {
    local lang=$1
    cd $lang
    sh build_image.sh
    cd ..
}

create_image cpp
create_image golang
create_image python
create_image rust

# Uncomment haskell if needed
# create_image haskell
