#!/bin/sh

# shellcheck shell=busybox

# libdrm-tests ships DRM tools (modetest, ...) with no --version flag; the other
# subpackages ship no executables. Skip the version probe for the whole family.
case "$PKG_NAME" in
libdrm*)
	exit 0
	;;

*)
	echo "Untested package: $PKG_NAME" >&2
	exit 1
	;;
esac
