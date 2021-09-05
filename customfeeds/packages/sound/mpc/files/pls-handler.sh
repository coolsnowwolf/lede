#!/bin/sh
echo parameter: file.pls
mpc clear
grep '^File[0-9]*' "$1" | sed -e 's/^File[0-9]*=//' | mpc add
mpc play
