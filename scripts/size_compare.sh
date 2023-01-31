# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2020 Paul Spooren <mail@aparcar.org>
#
###
### size_compare - compare size of OpenWrt packages against upstream
###
### The script compares locally compiled package with the package indexes
### available upstream. This way the storage impact of optimizations or
### feature modifications is easy to see.
###
### If no environmental variables are set the script reads the current
### .config file. The evaluated env variables are the following:
###
###   TARGET SUBTARGET ARCH PACKAGES BIN_DIR BASE_URL CHECK_INSTALLED
###
### Usage:
###   ./scripts/size_compare.sh
###
### Options:
###   -p --package-size 	Check IPK package size and not installed size
###   -h --help 		This message

eval "$(grep \
	-e ^CONFIG_TARGET_BOARD= \
	-e ^CONFIG_TARGET_SUBTARGET= \
	-e ^CONFIG_TARGET_ARCH_PACKAGES= \
	-e ^CONFIG_BINARY_FOLDER= \
	.config 2>/dev/null \
)"
CONFIG_PACKAGES=$(sed -n 's/^CONFIG_PACKAGE_\(.*\)=y$/\1/p' .config | tr '\n' ' ')

TARGET=${TARGET:-$CONFIG_TARGET_BOARD}
SUBTARGET=${SUBTARGET:-$CONFIG_TARGET_SUBTARGET}
ARCH=${ARCH:-$CONFIG_TARGET_ARCH_PACKAGES}
PACKAGES=${PACKAGES:-$CONFIG_PACKAGES}
BIN_DIR=${CONFIG_BINARY_FOLDER:-./bin}
BASE_URL="${BASE_URL:-https://downloads.openwrt.org/snapshots}"
CHECK_INSTALLED="${CHECK_INSTALLED:-y}"

TARGET_URL="$BASE_URL/targets/$TARGET/$SUBTARGET/packages/Packages.gz"
CONFIG_URL="$BASE_URL/targets/$TARGET/$SUBTARGET/config.buildinfo"
PACKAGES_URL="$BASE_URL/packages/$ARCH/base/Packages.gz"

if command -v curl > /dev/null; then
	DOWNLOAD_METHOD="curl"
else
	DOWNLOAD_METHOD="wget --output-document=-"
fi

help() {
    sed -rn 's/^### ?//;T;p' "$0"
}

package_size () {
	FOUND_PACKAGE=
	if [ -z "$CHECK_INSTALLED" ]; then
		SEARCH_PATTERN="Size"
	else
		SEARCH_PATTERN="Installed-Size"
	fi
	while IFS= read -r line; do
		if [ "$line" = "Package: $2" ]; then
			FOUND_PACKAGE=y
		fi
		if [ -n "$FOUND_PACKAGE" ]; then
			case $line in
				"$SEARCH_PATTERN"*)
					echo "$line" | cut -d ' ' -f 2
					break
					;;
			esac
		fi
	done < "$1"
}

compare_sizes () {
	TOTAL_DIFF="0"
	for PACKAGE in $PACKAGES; do
		if [ "$PACKAGE" = "libc" ]; then
			continue
		fi
		PACKAGE_FILE=$(find "$BIN_DIR/packages/$ARCH/" \
			"$BIN_DIR/targets/$TARGET/$SUBTARGET/" \
			-name "${PACKAGE}_*.ipk" 2>/dev/null | head -n1)

		if [ -z "$PACKAGE_FILE" ]; then
			continue
		fi
		if [ -z "$CHECK_INSTALLED" ]; then
			SIZE_LOCAL=$(stat -c '%s' "$PACKAGE_FILE")
		else
			SIZE_LOCAL=$(tar tzvf "$PACKAGE_FILE" ./data.tar.gz | awk '{ print $3 }')
		fi
		SIZE_UPSTREAM=$(package_size "$TMP_INDEX" "$PACKAGE")
		SIZE_DIFF="$((SIZE_LOCAL - SIZE_UPSTREAM))"
		TOTAL_DIFF="$((TOTAL_DIFF + SIZE_DIFF))"
		if [ "$SIZE_DIFF" -gt 0 ]; then
			SIZE_DIFF="+$SIZE_DIFF"
		fi
		if [ -n "$SIZE_UPSTREAM" ]; then
			echo "${SIZE_DIFF}	${SIZE_LOCAL}	${SIZE_UPSTREAM}	$PACKAGE"
		else
			echo "$PACKAGE is missing upstream"
		fi
	done
	echo "~~~~~~~	total change	${TOTAL_DIFF}"
}

if [ "$1" = "-h" ]; then
    help
    exit 0
fi

if [ "$1" = "-p" ]; then
    CHECK_INSTALLED=
fi

echo "Compare packages of $TARGET/$SUBTARGET/$ARCH":
echo "$PACKAGES"
echo

echo "Checking configuration difference"
TMP_CONFIG=$(mktemp /tmp/config.XXXXXX)
sed -n 's/^	\+config \(.*\)/\1/p' config/Config-build.in config/Config-devel.in > "${TMP_CONFIG}-FOCUS"
sort .config | grep -f "${TMP_CONFIG}-FOCUS" | grep -v "^#" | sort > "${TMP_CONFIG}-LOCAL"
mv .config .config.bak
"$DOWNLOAD_METHOD" "$CONFIG_URL" > .config
make defconfig > /dev/null 2> /dev/null
grep -f "${TMP_CONFIG}-FOCUS" .config | grep -v "^#" | sort > "${TMP_CONFIG}-UPSTREAM"
mv .config.bak .config

echo
echo " --- start config diff ---"
diff -u "${TMP_CONFIG}-LOCAL" "${TMP_CONFIG}-UPSTREAM"
echo " --- end config diff ---"
rm "${TMP_CONFIG}-FOCUS" "${TMP_CONFIG}-UPSTREAM" "${TMP_CONFIG}-LOCAL"
echo

if [ -z "$CHECK_INSTALLED" ]; then
	echo "Checking IPK package size"
else
	echo "Checking installed size"
fi
echo

echo "Fetching latest package indexes..."
TMP_INDEX=$(mktemp /tmp/size_compare_package_index.XXXXXX)
"$DOWNLOAD_METHOD" "$TARGET_URL" | gzip -d > "$TMP_INDEX" || exit 1
"$DOWNLOAD_METHOD" "$PACKAGES_URL" | gzip -d >> "$TMP_INDEX" || exit 1
echo

echo "Comparing package sizes..."
echo "Change 	Local	Remote 	Package"
compare_sizes | sort -g -r

rm "$TMP_INDEX"
