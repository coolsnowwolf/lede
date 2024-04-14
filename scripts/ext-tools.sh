#!/usr/bin/env bash

TOOLS_TAR=""
HOST_BUILD_DIR=$(pwd)/"build_dir/host"
HOST_STAGING_DIR_STAMP=$(pwd)/"staging_dir/host/stamp"

refresh_timestamps() {
	find -H "$1" -not -type l -print0 | xargs -0 touch
}

extract_prebuilt_tar() {
	tar -xf "$1"
}

refresh_prebuilt_tools() {
	if [ ! -d "$HOST_BUILD_DIR" ]; then
		echo "Can't find Host Build Dir "$HOST_BUILD_DIR"" >&2
		exit 1
	fi

	refresh_timestamps "$HOST_BUILD_DIR"
	sleep 1

	if [ ! -d "$HOST_STAGING_DIR_STAMP" ]; then
		echo "Can't find Host Staging Dir Stamp "$HOST_STAGING_DIR_STAMP"" >&2
		exit 1
	fi

	refresh_timestamps "$HOST_STAGING_DIR_STAMP"

	return 0
}

install_prebuilt_tools() {
	extract_prebuilt_tar "$TOOLS_TAR"

	refresh_prebuilt_tools

	return 0
}

while [ -n "$1" ]; do
	arg="$1"; shift
	case "$arg" in
		--host-build-dir)
			[ -d "$1" ] || {
				echo "Directory '$1' does not exist." >&2
				exit 1
			}
			HOST_BUILD_DIR="$(cd "$1"; pwd)"; shift
		;;

		--host-staging-dir-stamp)
			[ -d "$1" ] || {
				echo "Directory '$1' does not exist." >&2
				exit 1
			}
			HOST_STAGING_DIR_STAMP="$(cd "$1"; pwd)"; shift
		;;

		--tools)
			[ -f "$1" ] || {
				echo "Tools tar file '$1' does not exist." >&2
				exit 1
			}
			TOOLS_TAR="$1"; shift
			install_prebuilt_tools

			exit $?
		;;

		--refresh)
			refresh_prebuilt_tools

			exit $?
		;;

		-h|--help)
			me="$(basename "$0")"
			echo -e "\nUsage:\n"                                            >&2
			echo -e "  $me --host-build-dir {directory}"                    >&2
			echo -e "    Set to refresh timestamp of this build directory"  >&2
			echo -e "    with --tools."                                     >&2
			echo -e "    THIS OPTION MUST BE SET BEFORE --tools."           >&2
			echo -e "    If not provided the default directory is:"         >&2
			echo -e "    $(pwd)/build_dir/host\n"                           >&2
			echo -e "  $me --host-staging-dir-stamp {directory}"            >&2
			echo -e "    Set to refresh staging timestamp present in this"  >&2
			echo -e "    directory with --tools."                           >&2
			echo -e "    THIS OPTION MUST BE SET BEFORE --tools."           >&2
			echo -e "    If not provided the default directory is:"         >&2
			echo -e "    $(pwd)/staging_dir/host/stamp\n"                   >&2
			echo -e "  $me --tools {tar}"                                   >&2
			echo -e "    Install the prebuilt tools present in the passed"  >&2
			echo -e "    tar and prepare them."                             >&2
			echo -e "    To correctly use them it's needed to update the"   >&2
			echo -e "    timestamp of each tools to skip recompilation.\n"  >&2
			echo -e "  $me --refresh"                                       >&2
			echo -e "    Refresh timestamps of already extracted prebuilt"  >&2
			echo -e "    tools to correctly use them and skip"              >&2
			echo -e "    recompilation.\n"                                  >&2
			echo -e "  $me --help"                                          >&2
			echo -e "    Display this help text and exit.\n\n"              >&2
			exit 1
		;;

		*)
			echo "Unknown argument '$arg'" >&2
			exec $0 --help
		;;
	esac
done

exec $0 --help
