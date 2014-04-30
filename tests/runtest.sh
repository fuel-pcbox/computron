#!/bin/bash

if [ "$1" = "" ] ; then
	echo "usage: $0 <testfile>"
	exit 1
fi

PROGRAM="../vomit --run"
TEST=$1
EXPECTATION=$(echo $TEST | sed s/.asm/.expected/)
COMPILED=tmp.bin
RESULT=`mktemp /tmp/tmp.XXXXXX || exit 1`

echo ": $TEST"

nasm -f bin -o $COMPILED $TEST || \
	{ rm -f $COMPILED
	  exit 1
	}

$PROGRAM $COMPILED > $RESULT
if [ -e $EXPECTATION ]; then
    diff -u $RESULT $EXPECTATION
else
    cat $RESULT > $EXPECTATION
    echo "Wrote new expectation: $EXPECTATION"
fi

rm -f $COMPILED $RESULT
