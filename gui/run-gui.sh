#!/bin/sh
cd ..
export LD_LIBRARY_PATH=.

#valgrind --error-limit=no --num-callers=20 ./gui/gui $*
./gui/gui $*
