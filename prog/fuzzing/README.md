# Leptonica fuzzing

This directory contains fuzzing tests for Leptonica.
Each test is in a separate source file *_fuzzer.cc.

Normally these fuzzing tests are run by [OSS-Fuzz](https://oss-fuzz.com/),
but can also be run locally.

## Local build instructions

Local builds require the clang compiler.
The example was tested with clang-6.0 on Debian GNU Linux.

    ./configure CC=clang-6.0 CXX=clang++-6.0
    make fuzzers CXX=clang++-6.0

## Running local fuzzers

Each local fuzzer can be run like in the following example.

    # Show command line syntax.
    ./barcode_fuzzer -help=1

    # Run the fuzzer.
    ./barcode_fuzzer
