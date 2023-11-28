#!/bin/sh

set -e;

sed -n -e 's/.*cloud_build_number *= *//' \
	   -e 's/ *;.*//p' "./version.h"