#!/bin/sh
cd ..
export LD_LIBRARY_PATH=.

if [ "$1" = "--valgrind" ]; then 
	shift
	valgrind --error-limit=no --num-callers=20 ./gui/gui $*
elif [ "$1" = "--callgrind" ]; then 
	shift
	valgrind --tool=callgrind --error-limit=no --num-callers=20 ./gui/gui $*
elif [ "$1" = "--memleak" ]; then 
	shift
	valgrind --leak-check=full --show-reachable=yes --error-limit=no --num-callers=20 ./gui/gui $*
else
	./gui/gui $*
fi
