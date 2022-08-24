#!/bin/bash

function remove() {
    echo -ne "removing \"$1\" ..."
    rm -rf $1
    echo "done"
}

remove "build/"
remove "bin/"
remove "libs/libhermesnet.a"

