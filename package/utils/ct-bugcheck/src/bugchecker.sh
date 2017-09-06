#!/bin/sh

# Periodically call bugcheck.sh script

CHECKER=bugcheck.sh
SLEEPFOR=60

DO_BUGCHECK=0

# So, to enable this, you create an /etc/config/bugcheck file
# with contents like:
#  DO_BUGCHECK=1
#  export DO_BUGCHECK

if [ -f /etc/config/bugcheck ]
    then
    . /etc/config/bugcheck
fi

if [ $DO_BUGCHECK == 0 ]
then
    exit 0
fi

while true
  do
  $CHECKER
  sleep $SLEEPFOR
done
