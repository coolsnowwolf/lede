#!/usr/bin/env bash
set -e

pkg_dir=$1

if [ -z $pkg_dir ] || [ ! -d $pkg_dir ]; then
	echo "Usage: ipkg-make-index <package_directory>" >&2
	exit 1
fi

empty=1

for pkg in `find $pkg_dir -name '*.ipk' | sort`; do
	empty=
	name="${pkg##*/}"
	name="${name%%_*}"
	[[ "$name" = "kernel" ]] && continue
	[[ "$name" = "libc" ]] && continue
	echo "Generating index for package $pkg" >&2
	file_size=$(stat -L -c%s $pkg)
	sha256sum=$($MKHASH sha256 $pkg)
	# Take pains to make variable value sed-safe
	sed_safe_pkg=`echo $pkg | sed -e 's/^\.\///g' -e 's/\\//\\\\\\//g'`
	tar -xzOf $pkg ./control.tar.gz | tar xzOf - ./control | sed -e "s/^Description:/Filename: $sed_safe_pkg\\
Size: $file_size\\
SHA256sum: $sha256sum\\
Description:/"
	echo ""
done
[ -n "$empty" ] && echo
exit 0
