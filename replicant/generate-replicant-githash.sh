#!/bin/bash

HASH_TEXT='replicant-githash.txt'

git rev-parse --verify HEAD > ${NDK_PROJECT_PATH}/${HASH_TEXT}

# Hmmm, why do these echo's not work, are they being picked up by make?
if [ $? -eq 0 ]; then
	echo "Created '${NDK_PROJECT_PATH}/${HASH_TEXT}'" > /dev/null
else
	echo "Failed to create '${NDK_PROJECT_PATH}/${HASH_TEXT}'" > dev/null
fi
