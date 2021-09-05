#!/bin/sh

prometheus --version 2>&1 | grep "$2"
