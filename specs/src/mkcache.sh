#!/bin/sh
find -s . | grep \.d$ | specs cat 1 w1 nw | $SHELL > Makefile.cached_depends
cat Makefile.cached_depends | sed s/\.o:\ /\.obj:\ / > Makefile.cached_depends_vs

