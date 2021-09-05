#!/bin/sh

"$1" --version 2>&1 | grep "$2"
