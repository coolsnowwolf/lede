#!/bin/sh

yq --version 2>&1 | grep "${2#*v}"
