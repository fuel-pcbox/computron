#!/bin/bash

if [ "$1" = "" ] ; then
	echo "usage: $0 <testfile>"
	exit 1
fi

PROGRAM="../vomit --no-gui --no-vlog --run"
TEST=$1
EXPECTATION=$(echo $TEST | sed s/.asm/.expected/)
COMPILED=tmp.bin
RESULT=`mktemp /tmp/tmp.XXXXXX || exit 1`

nasm -f bin -o $COMPILED $TEST || \
	{ rm -f $COMPILED
	  exit 1
	}

$PROGRAM $COMPILED > $RESULT
if [ -e $EXPECTATION ]; then
    if diff -q $EXPECTATION $RESULT >/dev/null; then
        echo -ne "\033[32;1mPASS\033[0m: "
    else
        echo -ne "\033[31;1mFAIL\033[0m: "
        diff -u $EXPECTATION $RESULT | less -R
    fi
else
    cat $RESULT > $EXPECTATION
    echo -ne "\033[33;1mNEW\033[0m: "
fi
echo $TEST

rm -f $COMPILED $RESULT
