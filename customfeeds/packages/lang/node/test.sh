#!/bin/sh

node -pe 'process.version' | grep "$2"
