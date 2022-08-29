#!/bin/sh

TARGETS=$*
READELF="${READELF:-readelf}"
XARGS="${XARGS:-xargs -r}"

find $TARGETS -type f -a -exec file {} \; | \
  sed -n -e 's/^\(.*\):.*ELF.*\(executable\|shared object\).*,.*/\1/p' | \
  $XARGS -n1 $READELF -l | grep 'Requesting' | cut -d':' -f2 | tr -d ' ]' | \
  $XARGS basename

cd `dirname ${0}`
./gen-dependencies.sh ${TARGETS}
