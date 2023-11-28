#!/bin/sh

set -e;

sed -n -e 's/.*define *YAJL_MAJOR *//p' \
       -e 's/.*define *YAJL_MINOR *//p'  \
       -e 's/.*define *YAJL_MICRO *//p'  "./include/yajl/yajl_version.h" | tr -d '\r' | paste -s -d '.' - - -