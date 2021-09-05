#!/bin/sh

/usr/bin/"${1}" version 2>/dev/null | grep -F "${2}"
