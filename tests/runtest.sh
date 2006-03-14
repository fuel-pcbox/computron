#!/bin/sh

assembleonly=false

if [ "$1" = "" ] ; then
	echo "usage: $0 <testfile>"
	exit 1
fi

if [ "$1" = "-s" ] ; then
	assembleonly=true
	shift
fi

PROGRAM="./vmachine --try"
TEST=$1
TMP_IN=`mktemp /tmp/tmp.XXXXXX || exit 1`
TMP_OUT=`mktemp /tmp/tmp.XXXXXX || exit 1`
TMP_RES=`mktemp /tmp/tmp.XXXXXX || exit 1`
TMP_RUN=`mktemp /tmp/tmp.XXXXXX || exit 1`

echo "Running test: $1"

sed -n '/^asm/,/^_asm/p' $TEST | grep '^[ 	]' > $TMP_IN
sed -n '/^result/,/^_result/p' $TEST | grep '^[ 	]' | sed 's/^[	 ]//' > $TMP_RES
echo "HLT" >> $TMP_IN
nasm -f bin -o $TMP_OUT $TMP_IN || \
	{ cp $TMP_IN __fail.asm
	  rm -f $TMP_IN $TMP_OUT $TMP_RES $TMP_RUN
	  exit 1
	}

$assembleonly && \
	{
	  cp $TMP_OUT prog.bin
	  rm -f $TMP_IN $TMP_OUT $TMP_RES $TMP_RUN
	  exit
	}

OWD=`pwd`
cd ..
$PROGRAM $TMP_OUT > $TMP_RUN
cd $OWD

for expression in `cat $TMP_RES` ; do
	grep $expression $TMP_RUN > /dev/null
	if [ "$?" != "0" ] ; then
		echo "FAIL: $expression"
	fi
done

rm -f $TMP_IN $TMP_OUT $TMP_RES $TMP_RUN
