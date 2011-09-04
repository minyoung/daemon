#!/bin/bash

FAILED_TESTS=""
pushd bin
for t in check_*; do
    echo "== Running $t =="
    ./$t || FAILED_TESTS="$FAILED_TESTS $t"
    echo
done
popd

if [[ $FAILED_TESTS != "" ]]; then
    echo
    echo "The following tests failed:"
    for t in $FAILED_TESTS; do
        echo "* $t"
    done
fi
