#!/usr/bin/env bash
export LANG=C
export LC_ALL=C
[ -n "$TOPDIR" ] && cd $TOPDIR

try_version() {
	[ -f version.date ] || return 1
	SOURCE_DATE_EPOCH="$(cat version.date)"
	[ -n "$SOURCE_DATE_EPOCH" ]
}

try_git() {
	[ -e .git ] || return 1
	SOURCE_DATE_EPOCH="$(git log -1 --format=format:%ct)"
	[ -n "$SOURCE_DATE_EPOCH" ]
}

try_hg() {
	[ -d .hg ] || return 1
	SOURCE_DATE_EPOCH="$(hg log --template '{date}' -l 1 | cut -d. -f1)"
	[ -n "$SOURCE_DATE_EPOCH" ]
}

try_mtime() {
	perl -e 'print((stat $ARGV[0])[9])' "$0"
	[ -n "$SOURCE_DATE_EPOCH" ]
}

try_version || try_git || try_hg || try_mtime || SOURCE_DATE_EPOCH=""
echo "$SOURCE_DATE_EPOCH"
