#!/bin/sh

tail -n $1 "$2" > /var/run/tailtmp
cat /var/run/tailtmp > "$2"
rm /var/run/tailtmp
