#!/bin/sh
# shellcheck disable=SC2039

"/usr/sbin/${1//-full}" -version 2>&1 | grep "$2"
