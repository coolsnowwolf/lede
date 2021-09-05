#!/bin/sh
ps -ef|grep openssl | /usr/bin/awk '{print $2}' |xargs kill -9
