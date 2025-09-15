#!/usr/bin/env bash
#
# Automated OpenWrt package dependency checker
#
# Copyright (C) 2009-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

SCRIPTDIR="$(dirname "$0")"
[ "${SCRIPTDIR:0:1}" = "/" ] || SCRIPTDIR="$PWD/$SCRIPTDIR"
BASEDIR="$SCRIPTDIR/.."

DIR="$BASEDIR/tmp/deptest"
STAMP_DIR_SUCCESS="$DIR/stamp-success"
STAMP_DIR_FAILED="$DIR/stamp-failed"
STAMP_DIR_BLACKLIST="$DIR/stamp-blacklist"
BUILD_DIR="$DIR/build_dir/target"
BUILD_DIR_HOST="$DIR/build_dir/host"
KERNEL_BUILD_DIR="$DIR/build_dir/linux"
STAGING_DIR="$DIR/staging_dir/target"
STAGING_DIR_HOST="$DIR/staging_dir/host"
STAGING_DIR_HOST_TMPL="$DIR/staging_dir_host_tmpl"
BIN_DIR="$DIR/staging_dir/bin_dir"
LOG_DIR_NAME="logs"
LOG_DIR="$DIR/$LOG_DIR_NAME"

die()
{
	echo "$@"
	exit 1
}

usage()
{
	echo "deptest.sh [OPTIONS] [PACKAGES]"
	echo
	echo "OPTIONS:"
	echo "  --lean       Run a lean test. Do not clean the build directory for each"
	echo "               package test."
	echo "  --force      Force a test, even if a success/blacklist stamp is available"
	echo "  -j X         Number of make jobs"
	echo
	echo "PACKAGES are packages to test. If not specified, all installed packages"
	echo "will be tested."
}

deptest_make()
{
	local target="$1"
	shift
	local logfile="$1"
	shift
	make -j$nrjobs "$target" \
		BUILD_DIR="$BUILD_DIR" \
		BUILD_DIR_HOST="$BUILD_DIR_HOST" \
		KERNEL_BUILD_DIR="$KERNEL_BUILD_DIR" \
		BIN_DIR="$BIN_DIR" \
		STAGING_DIR="$STAGING_DIR" \
		STAGING_DIR_HOST="$STAGING_DIR_HOST" \
		FORCE_HOST_INSTALL=1 \
		V=99 "$@" >"$LOG_DIR/$logfile" 2>&1
}

clean_kernel_build_dir()
{
	# delete everything, except the kernel build dir "linux-X.X.X"
	(
		cd "$KERNEL_BUILD_DIR" || die "Failed to enter kernel build dir"
		for entry in *; do
			[ -z "$(echo "$entry" | egrep -e '^linux-*.*.*$')" ] || continue
			rm -rf "$entry" || die "Failed to clean kernel build dir"
		done
	)
}

stamp_exists() # $1=stamp
{
	[ -e "$1" -o -L "$1" ]
}

test_package() # $1=pkgname
{
	local pkg="$1"
	[ -n "$pkg" -a -z "$(echo "$pkg" | grep -e '/')" -a "$pkg" != "." -a "$pkg" != ".." ] || \
		die "Package name \"$pkg\" contains illegal characters"
	local SELECTED=
	for conf in `grep CONFIG_PACKAGE tmp/.packagedeps | grep -E "[ /]$pkg\$" | sed -e 's,package-$(\(CONFIG_PACKAGE_.*\)).*,\1,'`; do
		grep "$conf=" .config > /dev/null && SELECTED=1 && break
	done
	local STAMP_SUCCESS="$STAMP_DIR_SUCCESS/$pkg"
	local STAMP_FAILED="$STAMP_DIR_FAILED/$pkg"
	local STAMP_BLACKLIST="$STAMP_DIR_BLACKLIST/$pkg"
	rm -f "$STAMP_FAILED"
	stamp_exists "$STAMP_SUCCESS" && [ $force -eq 0 ] && return
	rm -f "$STAMP_SUCCESS"
	[ -n "$SELECTED" ] || {
		echo "Package $pkg is not selected"
		return
	}
	stamp_exists "$STAMP_BLACKLIST" && [ $force -eq 0 ] && {
		echo "Package $pkg is blacklisted"
		return
	}
	echo "Testing package $pkg..."
	rm -rf "$STAGING_DIR" "$STAGING_DIR_HOST"
	mkdir -p "$STAGING_DIR"
	cp -al "$STAGING_DIR_HOST_TMPL" "$STAGING_DIR_HOST"
	[ $lean_test -eq 0 ] && {
		rm -rf "$BUILD_DIR" "$BUILD_DIR_HOST"
		clean_kernel_build_dir
	}
	mkdir -p "$BUILD_DIR" "$BUILD_DIR_HOST"
	local logfile="$(basename $pkg).log"
	deptest_make "package/$pkg/compile" "$logfile"
	if [ $? -eq 0 ]; then
		( cd "$STAMP_DIR_SUCCESS"; ln -s "../$LOG_DIR_NAME/$logfile" "./$pkg" )
	else
		( cd "$STAMP_DIR_FAILED"; ln -s "../$LOG_DIR_NAME/$logfile" "./$pkg" )
		echo "Building package $pkg FAILED"
	fi
}

# parse commandline options
packages=
lean_test=0
force=0
nrjobs=1
while [ $# -ne 0 ]; do
	case "$1" in
	--help|-h)
		usage
		exit 0
		;;
	--lean)
		lean_test=1
		;;
	--force)
		force=1
		;;
	-j*)
		if [ -n "${1:2}" ]; then
			nrjobs="${1:2}"
		else
			shift
			nrjobs="$1"
		fi
		;;
	*)
		packages="$packages $1"
		;;
	esac
	shift
done

[ -f "$BASEDIR/include/toplevel.mk" ] || \
	die "Error: Could not find buildsystem base directory"
[ -f "$BASEDIR/.config" ] || \
	die "The buildsystem is not configured. Please run make menuconfig."
cd "$BASEDIR" || die "Failed to enter base directory"

mkdir -p "$STAMP_DIR_SUCCESS" "$STAMP_DIR_FAILED" "$STAMP_DIR_BLACKLIST" \
	"$BIN_DIR" "$LOG_DIR"

bootstrap_deptest_make()
{
	local target="$1"
	shift
	local logfile="bootstrap-deptest-$(echo "$target" | tr / -).log"
	echo "deptest-make $target"
	deptest_make "$target" "$logfile" "$@" || \
		die "make $target failed, please check $logfile"
}

bootstrap_native_make()
{
	local target="$1"
	shift
	local logfile="bootstrap-native-$(echo "$target" | tr / -).log"
	echo "make $target"
	make -j$nrjobs "$target" \
		V=99 "$@" >"$LOG_DIR/$logfile" 2>&1 || \
		die "make $target failed, please check $logfile"
}

[ -d "$STAGING_DIR_HOST_TMPL" ] || {
	echo "Bootstrapping build environment..."
	rm -rf "$STAGING_DIR" "$STAGING_DIR_HOST" "$BUILD_DIR" "$BUILD_DIR_HOST" "$KERNEL_BUILD_DIR"
	mkdir -p "$STAGING_DIR" "$STAGING_DIR_HOST" \
		"$BUILD_DIR" "$BUILD_DIR_HOST" "$KERNEL_BUILD_DIR"
	bootstrap_native_make tools/install
	bootstrap_native_make toolchain/install
	bootstrap_deptest_make tools/install
	bootstrap_deptest_make target/linux/install
	cp -al "$STAGING_DIR_HOST" "$STAGING_DIR_HOST_TMPL"
	rm -rf "$STAGING_DIR" "$STAGING_DIR_HOST" "$BUILD_DIR" "$BUILD_DIR_HOST"
	echo "Build environment OK."
}

if [ -z "$packages" ]; then
	# iterate over all packages
	for pkg in `cat tmp/.packagedeps  | grep CONFIG_PACKAGE | grep -v curdir | sed -e 's,.*[/=]\s*,,' | sort -u`; do
		test_package "$pkg"
	done
else
	# only check the specified packages
	for pkg in $packages; do
		test_package "$pkg"
	done
fi
