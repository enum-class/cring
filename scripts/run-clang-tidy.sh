#! /bin/bash

run-clang-tidy -p bin -fix -extra-arg=-std=c++20 -extra-arg=-Wno-unknown-warning-option `git diff --name-only origin/main`
