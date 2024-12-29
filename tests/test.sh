#!/usr/bin/env bash

S3C=../build/s3c
RESULT=0

################################################################################
#                              utility functions                               #
################################################################################

verify_s3c() {
    if [ ! -f $S3C ]; then
        echo "error: s3c not present."
        exit 1
    fi
}

clean_test() {
    rm -f ./__main_pp.prog__
    rm -f ./a.out
    rm -f *.out
    rm -f *.err
}

assert_no_diff() {
    out_file=$1
    ground_truth=$2
    error_message=$3

    diffs=$(diff $out_file $ground_truth)

    if [ ! -z "$diffs" ]; then
        echo $error_message
        echo $diffs
        RESULT=1
    fi
}

assert_compiled() {
    test_name=$1

    if [ ! $? -eq 0 ]; then
        echo "error: compile result of $test_name is $? instead of 0."
        RESULT=1
    fi

    if [ ! -f ./a.out ]; then
        echo "error: a.out file not present for $test_name."
        RESULT=1
    fi
}

################################################################################
#                                 syntax tests                                 #
################################################################################

test_hello_world() {
    echo "running test on hello_world"
    $S3C ./syntax/hello_world.3
    assert_compiled "hello_world"

    ./a.out > hello_world.out
    assert_no_diff hello_world.out ./syntax/out/hello_world.out "error: hello_world."

    clean_test
}

test_comments() {
    echo "running test on comments"
    $S3C ./syntax/comments.3
    assert_compiled "comments"

    ./a.out > comments.out
    assert_no_diff comments.out ./syntax/out/comments.out "error: comments."

    clean_test
}

# note: wont compile because all the types and symbols are wrong
test_syntax() {
    echo "running test on syntax"

    $S3C ./syntax/syntax.3
    assert_compiled "syntax"

    clean_test
}

################################################################################
#                                     data                                     #
################################################################################

test_arrays() {
    echo "running test on arrays"

    $S3C ./data/arrays.3 2> arrays.err
    assert_compiled "arrays"
    assert_no_diff arrays.err ./data/out/arrays.err "error: arrays (errors)."

    ./a.out > arrays.out
    assert_no_diff arrays.out ./data/out/arrays.out "error: arrays (output)."

    clean_test
}

################################################################################
#                                   builtins                                   #
################################################################################

test_set() {
    echo "running test on set"

    $S3C ./builtins/set.3
    assert_compiled "set"

    ./a.out

    clean_test
}

test_operators() {
    echo "running test on operators"

    $S3C ./builtins/operators.3 2> operators.err
    assert_compiled "operators"
    assert_no_diff operators.err ./builtins/out/operators.err "error: operators."

    ./a.out > operators.out
    assert_no_diff operators.out ./builtins/out/operators.out "error: operators."

    clean_test
}

test_for() {
    echo "running test on for"
    $S3C ./builtins/for.3 2> for.err
    assert_no_diff for.err ./builtins/out/for.err "error: for."
    clean_test
}

################################################################################
#                                symbols tests                                 #
################################################################################

test_definitions() {
    echo "running test on definitions"

    $S3C ./symbols/definitions.3
    assert_compiled "definitions"

    ./a.out > definitions.out
    assert_no_diff definitions.out ./symbols/out/definitions.out "error: definitions."

    clean_test
}

test_no_function_definition_order() {
    echo "running test on no_function_definition_order"

    $S3C ./symbols/no_function_definition_order.3
    assert_compiled "no_function_definition_order"

    ./a.out > no_function_definition_order.out
    assert_no_diff \
        no_function_definition_order.out \
        ./symbols/out/no_function_definition_order.out \
        "error: no_function_definition_order."

    clean_test
}

test_multiple_definitions1() {
    echo "running test on multiple_definitions1"
    $S3C ./symbols/multiple_definitions1.3 2> multiple_definitions1.err
    assert_no_diff \
        multiple_definitions1.err \
        ./symbols/out/multiple_definitions1.err \
        "error: multiple_definitions1."
    clean_test
}

test_multiple_definitions2() {
    echo "running test on multiple_definitions1"
    $S3C ./symbols/multiple_definitions2.3 2> multiple_definitions2.err
    assert_no_diff \
        multiple_definitions2.err \
        ./symbols/out/multiple_definitions2.err \
        "error: multiple_definitions2."
    clean_test
}

test_recursion() {
    echo "running test on recursion"
    $S3C ./symbols/recursion.3
    assert_compiled "recursion"
    # todo: fix table of symbols
    ./a.out
    # todo: test output
    clean_test
}

################################################################################
#                                   errors                                     #
################################################################################

test_errors() {
    echo "running test on errors"
    $S3C ./errors/errors.3 2> errors.err
    assert_no_diff errors.err ./errors/out/errors.err "error: errors."
    clean_test
}

################################################################################
#                                   programs                                   #
################################################################################

test_fib() {
    echo "running test on fib"

    $S3C ./programs/fib.3
    assert_compiled "hello_world"

    echo 5 | ./a.out >> fib.out
    echo 7 | ./a.out >> fib.out

    assert_no_diff fib.out ./programs/out/fib.out "error: fib."

    clean_test
}

################################################################################
#                                    tests                                     #
################################################################################

verify_s3c

# syntax

test_hello_world
test_comments
# test_syntax # error !!!

# builtins

# test_set # python error !!!
test_operators # error !!!
test_for

# symbols

test_definitions
test_no_function_definition_order
test_multiple_definitions1
test_multiple_definitions2
# test_recursion # error !!!

# errors

test_errors

# programs

test_fib

test_arrays # error !!!

################################################################################
#                                  test exit                                   #
################################################################################

exit $RESULT
