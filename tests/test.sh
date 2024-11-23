#!/usr/bin/env bash

S3C=../build/s3c

verify_s3c() {
    if [ ! -f $S3C ]; then
        echo "error: s3c not present."
        exit 1
    fi
}

clean_test() {
    rm -f ./__main_pp.prog__
    rm -f ./a.out
}

test_hello_world() {
    echo "running test on hello_world"
    $S3C ./syntax/hello_world.3
    ./a.out
    clean_test
}

test_comments() {
    echo "running test on comments"
    $S3C ./syntax/comments.3
    ./a.out
    clean_test
}

# note: wont compile because all the types and symbols are wrong
test_syntax() {
    echo "running test on syntax"
    $S3C ./syntax/syntax.3
    clean_test
}

test_fib() {
    echo "running test on fib"
    $S3C ./fib.3
    echo 5 | ./a.out
    echo 7 | ./a.out
    clean_test
}

test_arrays() {
    echo "running test on arrays"
    $S3C ./data/arrays.3
    ./a.out
    clean_test
}

test_set() {
    echo "running test on set"
    $S3C ./builtins/set.3
    ./a.out
    clean_test
}

test_operations() {
    echo "running test on operations"
    $S3C ./builtins/operations.3
    ./a.out
    clean_test
}

test_for() {
    echo "running test on for"
    $S3C ./builtins/for.3
    clean_test
}

test_errors() {
    echo "running test on errors"
    $S3C ./errors/errors.3
    clean_test
}

test_definitions() {
    echo "running test on definitions"
    $S3C ./symbols/definitions.3
    ./a.out
    clean_test
}

test_no_function_definition_order() {
    echo "running test on no_function_definition_order"
    $S3C ./symbols/no_function_definition_order.3
    ./a.out
    clean_test
}

test_multiple_definitions1() {
    echo "running test on multiple_definitions1"
    $S3C ./symbols/multiple_definitions1.3
    clean_test
}

test_multiple_definitions2() {
    echo "running test on multiple_definitions1"
    $S3C ./symbols/multiple_definitions2.3
    clean_test
}

test_recursion() {
    echo "running test on recursion"
    $S3C ./symbols/recursion.3
    ./a.out
    clean_test
}

verify_s3c

# syntax

# test_hello_world
# test_comments
# test_syntax # error !!!

# builtins

# test_set # python error !!!
# test_operations # error !!!
# test_for # todo: save the errors

# symbols

# test_definitions
# test_no_function_definition_order
# test_multiple_definitions1 # todo: save the error
# test_multiple_definitions2 # todo: save the error
# test_recursion # error !!!

# errors

# test_errors # todo: save the errors

# programs

# test_fib

# test_arrays # error !!!
