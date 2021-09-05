#!/bin/sh

cd `dirname "$0"`/tests
../tap/runtests -b /tmp ./*test_*
ret=$?
cd -

return $ret
