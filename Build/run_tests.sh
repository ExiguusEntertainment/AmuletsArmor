#!/bin/sh
# Simple build script for running distance tests
set -e
mkdir -p build_tests
cc -IInclude tests/test_distance.c -o build_tests/test_distance
build_tests/test_distance
