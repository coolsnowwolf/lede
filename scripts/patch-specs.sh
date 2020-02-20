#!/usr/bin/env bash

DIR="$1"

if [ -d "$DIR" ]; then
	DIR="$(cd "$DIR"; pwd)"
else
	echo "Usage: $0 toolchain-dir"
	exit 1
fi

echo -n "Locating cpp ... "
for bin in bin usr/bin usr/local/bin; do
	for cmd in "$DIR/$bin/"*-cpp; do
		if [ -x "$cmd" ]; then
			echo "$cmd"
			CPP="$cmd"
			break
		fi
	done
done

if [ ! -x "$CPP" ]; then
	echo "Can't locate a cpp executable in '$DIR' !"
	exit 1
fi

patch_specs() {
	local found=0

	for lib in $(STAGING_DIR="$DIR" "$CPP" -x c -v /dev/null 2>&1 | sed -ne 's#:# #g; s#^LIBRARY_PATH=##p'); do
		if [ -d "$lib" ]; then
			grep -qs "STAGING_DIR" "$lib/specs" && rm -f "$lib/specs"
			if [ $found -lt 1 ]; then
				echo -n "Patching specs ... "
				STAGING_DIR="$DIR" "$CPP" -dumpspecs | awk '
					mode ~ "link" {
						sub(/(%@?\{L.\})/, "& -L %:getenv(STAGING_DIR /usr/lib) -rpath-link %:getenv(STAGING_DIR /usr/lib)")
					}
					mode ~ "cpp" {
						$0 = $0 " -idirafter %:getenv(STAGING_DIR /usr/include)"
					}
					{
						print $0
						mode = ""
					}
					/^\*cpp:/ {
						mode = "cpp"
					}
					/^\*link.*:/ {
						mode = "link"
					}
				' > "$lib/specs"
				echo "ok"
				found=1
			fi
		fi
	done

	[ $found -gt 0 ]
	return $?
}


VERSION="$(STAGING_DIR="$DIR" "$CPP" --version | sed -ne 's/^.* (.*) //; s/ .*$//; 1p')"
VERSION="${VERSION:-unknown}"

case "${VERSION##* }" in
	2.*|3.*|4.0.*|4.1.*|4.2.*)
		echo "The compiler version does not support getenv() in spec files."
		echo -n "Wrapping binaries instead ... "

		if "${0%/*}/ext-toolchain.sh" --toolchain "$DIR" --wrap "${CPP%/*}"; then
			echo "ok"
			exit 0
		else
			echo "failed"
			exit $?
		fi
	;;
	*)
		if patch_specs; then
			echo "Toolchain successfully patched."
			exit 0
		else
			echo "Failed to locate library directory!"
			exit 1
		fi
	;;
esac
