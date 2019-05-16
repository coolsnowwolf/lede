mkdir -pv m4
libtoolize
aclocal
aclocal -I m4
automake --add-missing --copy
autoconf


