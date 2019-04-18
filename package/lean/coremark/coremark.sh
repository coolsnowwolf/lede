#!/bin/sh

/bin/coremark > /tmp/coremark.log

cat /tmp/coremark.log | grep "CoreMark 1.0" | cut -d  "/"  -f 1 > /tmp/bench.log
sed -i 's/CoreMark 1.0/(CpuMark/g'  /tmp/bench.log
echo " Scores)" >> /tmp/bench.log