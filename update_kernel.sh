#!/bin/sh
#
# update_kernel.sh: (c) 2017 Jonas Gorski <jonas.gorski@gmail.com>
# Licensed under the terms of the GNU GPL License version 2

BUILD=0
BUILD_ARGS=
VERBOSE=w
TEST=0
UPDATE=0

KERNEL=
PATCHVER=

while [ $# -gt 0 ]; do
	case $1 in
		-b|--build)
			BUILD=1
			shift
			BUILD_ARGS=$1
			;;
		-q|--quiet)
			VERBOSE=
			;;
		-t|--test)
			TEST=1
			;;
		-u|--update)
			UPDATE=1
			;;
		-v|--verbose)
			VERBOSE=ws
			;;
		[1-9]*)
			if [ -z "$KERNEL" ]; then
				KERNEL=$1
			elif [ -z "$PATCHVER" ]; then
				PATCHVER=$1
			else
				exit 1
			fi
			;;
		*)
			break
			;;

	esac

	shift
done

if [ -z "$KERNEL" ]; then
	echo "usage: $0 [<options>...] <patchver> [<version>]"
	echo "example: $0 3.18 3.18.30"
	echo "If <version> is not given, it will try to find out the latest from kernel.org"
	echo ""
	echo "valid options:"
	echo "-b|--build <args> also do a test build with <args> as extra arguments (e.g. -j 3)"
	echo "-q|--quiet        less output"
	echo "-t|--test         don't do anything, just print what it would do"
	echo "-u|--update       update include/kernel-version.mk after a successful run"
	echo "-v|--verbose      more output (pass V=ws to all commands)"
	exit 1
fi

if [ -z "$PATCHVER" ]; then
	if [ -n "$(which curl)" ]; then
		DL_CMD="curl -s "
	fi

	if [ -n "$(which wget)" ]; then
		DL_CMD="wget -O - -q "
	fi

	if [ -z "$DL_CMD" ]; then
		echo "Failed to find a suitable download program. Please install either curl or wget." >&2
		exit 1
	fi

	# https://www.kernel.org/feeds/kdist.xml
	# $(curl -s https://www.kernel.org/feeds/kdist.xml | sed -ne 's|^.*"html_url": "\(.*/commit/.*\)",|\1.patch|p')
	# curl -s "https://www.kernel.org/feeds/kdist.xml"
	CURR_VERS=$($DL_CMD "https://www.kernel.org/feeds/kdist.xml" | sed -ne 's|^.*title>\([1-9][^\w]*\): .*|\1|p')

	for ver in $CURR_VERS; do
		case $ver in
			"$KERNEL"*)
				PATCHVER=$ver
				;;
		esac

		if [ -n "$PATCHVER" ]; then
			break
		fi
	done

	if [ -z "$PATCHVER" ]; then
		echo "Failed to find the latest release on kernel.org, please specify the release manually" >&2
		exit 1
	fi
fi

echo "Refreshing Kernel $KERNEL to release $PATCHVER ..."

targets=$(ls -b target/linux)

if [ "$TEST" -eq 1 ]; then
	CMD="echo"
fi

for target in $targets; do
	if [ "$target" = "generic" -o -f "$target" ]; then
		continue
	fi

	grep -q "broken" target/linux/$target/Makefile && { \
		echo "Skipping $target (broken)"
		continue
	}

	if [ -e tmp/${target}_${PATCHVER}_done ]; then
		continue
	fi

	grep -q "${PATCHVER}" target/linux/$target/Makefile || \
	[ -f target/linux/$target/config-${KERNEL} ] || \
	[ -d target/linux/$target/patches-${KERNEL} ] && {
		echo "refreshing $target ..."
		$CMD echo "CONFIG_TARGET_$target=y" > .config || exit 1
		$CMD echo "CONFIG_ALL_KMODS=y" >> .config || exit 1
		$CMD make defconfig KERNEL_PATCHVER=${KERNEL} || exit 1
		if [ ! -f tmp/${target}_${PATCHVER}_refreshed ]; then
			$CMD make target/linux/refresh V=$VERBOSE KERNEL_PATCHVER=${KERNEL} LINUX_VERSION=${PATCHVER} LINUX_KERNEL_HASH=skip || exit 1
			$CMD make target/linux/prepare V=$VERBOSE KERNEL_PATCHVER=${KERNEL} LINUX_VERSION=${PATCHVER} || exit 1
			$CMD touch tmp/${target}_${PATCHVER}_refreshed
		fi
		if [ "$BUILD" = "1" ]; then
			echo "building $target ... "
			$CMD make V=$VERBOSE KERNEL_PATCHVER=${KERNEL} LINUX_VERSION=${PATCHVER} $BUILD_ARGS || exit 1
		fi
		$CMD make target/linux/clean
		$CMD touch tmp/${target}_${PATCHVER}_done
	} || {
		echo "skipping $target (no support for $KERNEL)"
	}
done

if [ "$UPDATE" -eq 1 ]; then
	NEWVER=${PATCHVER#$KERNEL}
	if [ "$TEST" -eq 1 ]; then
		echo ./staging_dir/host/bin/mkhash sha256 dl/linux-$PATCHVER.tar.xz
	fi

	if [ -f dl/linux-$PATCHVER.tar.xz ]; then
		CHECKSUM=$(./staging_dir/host/bin/mkhash sha256 dl/linux-$PATCHVER.tar.xz)
	fi

	if [ -f include/kernel-${KERNEL} ]; then
		# split version files
		KERNEL_VERSION_FILE=include/kernel-${KERNEL}
	else
		# unified version file
		KERNEL_VERSION_FILE=include/kernel-version.mk
	fi

	$CMD ./staging_dir/host/bin/sed -i ${KERNEL_VERSION_FILE} \
		-e "s|LINUX_VERSION-${KERNEL} =.*|LINUX_VERSION-${KERNEL} = ${NEWVER}|" \
		-e "s|LINUX_KERNEL_HASH-${KERNEL}.*|LINUX_KERNEL_HASH-${PATCHVER} = ${CHECKSUM}|"
fi
