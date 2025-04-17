#!/usr/bin/env bash
if [ ! $# -eq 1 ]
then
    echo >&2 usage: $0 '[program]'
    exit 1
fi
while read line
do
    $1 $line 0 -1
done <test/inputs
