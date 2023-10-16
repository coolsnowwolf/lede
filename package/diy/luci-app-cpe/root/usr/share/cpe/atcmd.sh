#!/bin/sh
rec=$(sendat $1 $2)
echo $rec  >>  /tmp/result.at