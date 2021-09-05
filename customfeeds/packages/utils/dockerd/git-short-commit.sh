#!/bin/sh
#
#	USAGE: git-short-commit.sh <GIT_URL> <GIT_REF> <GIT_DIR>
#

set -e

error() {
	echo "ERROR: ${*}" >&2
	exit 1
}

GIT_URL="${1}"
if [ -z "${GIT_URL}" ]; then
	error "Git URL not specified"
fi

GIT_REF="${2}"
if [ -z "${GIT_REF}" ]; then
	error "Git reference not specified"
fi

GIT_DIR="${3}"
if [ -z "${GIT_DIR}" ]; then
	error "Git clone directory not specified"
fi

clean_up() {
	rm --force --recursive "${GIT_DIR}"
}
trap clean_up EXIT

git init --quiet "${GIT_DIR}"
(
	cd "${GIT_DIR}"
	for PREFIX in "" "https://" "http://" "git@"; do
		echo "Trying remote '${PREFIX}${GIT_URL}'" >&2
		git remote add origin "${PREFIX}${GIT_URL}"

		if git fetch --depth 1 origin "${GIT_REF}"; then
			git checkout --detach FETCH_HEAD --
			git rev-parse --short HEAD
			break
		fi

		git remote remove origin
	done
)
