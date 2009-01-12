#!/bin/sh
cd ..
export LD_LIBRARY_PATH=.

#valgrind --tool=callgrind --error-limit=no --num-callers=20 ./gui/gui $*
valgrind --error-limit=no --num-callers=20 ./gui/gui $*
#./gui/gui $*
