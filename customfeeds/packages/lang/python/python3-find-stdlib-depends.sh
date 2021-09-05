#!/bin/sh

# Packages data
#
# Notes:
# * python3-codecs: Also contains codecs for CJK encodings but we don't
#   have a good way to check for uses
# * python3-openssl: Don't include hashlib as it supports several
#   standard algorithms without requiring OpenSSL

packages="
python3-asyncio: asyncio
python3-cgi: cgi
python3-cgitb: cgitb
python3-codecs: unicodedata
python3-ctypes: ctypes
python3-dbm: dbm
python3-decimal: decimal
python3-distutils: distutils
python3-email: email
python3-gdbm: dbm.gnu
python3-logging: logging
python3-lzma: lzma
python3-multiprocessing: multiprocessing
python3-ncurses: ncurses
python3-openssl: ssl
python3-pydoc: doctest pydoc
python3-sqlite3: sqlite3
python3-unittest: unittest
python3-urllib: urllib
python3-xml: xml xmlrpc
"


# Constants

stdin_name="<stdin>"
grep_dir_filters="
-Ir
--include=*.py
--exclude=setup.py
--exclude=test_*.py
--exclude=*_test.py
--exclude-dir=test
--exclude-dir=tests
--exclude-dir=ipkg-*
--exclude-dir=.pkgdir
"

log_level_notice=5
log_level_info=6
log_level_debug=7

# /usr/include/sysexits.h
ex_usage=64
ex_noinput=66
ex_unavailable=69
ex_software=70

newline="
"
oifs="$IFS"


# Defaults

grep_output_default_max_count=3
grep_output_default_color_when="auto"
grep_output_default_line_prefix="-HnT --label=$stdin_name"
grep_output_default_context_num=1

log_level_default="$log_level_info"


# Global variables

log_level=
grep=
grep_output_options=
grep_output_description=
stdin=
output_name=
is_first_search=


# Logging

log() {
	printf '%s\n' "$*"
}

can_log_notice() {
	[ "$log_level" -ge "$log_level_notice" ]
}

can_log_info() {
	[ "$log_level" -ge "$log_level_info" ]
}

can_log_debug() {
	[ "$log_level" -ge "$log_level_debug" ]
}

log_notice() {
	if can_log_notice; then
		log "$@"
	fi
}

log_info() {
	if can_log_info; then
		log "$@"
	fi
}

log_debug() {
	if can_log_debug; then
		log "$@"
	fi
}

log_error() {
	printf '%s\n' "Error: $*" >&2
}


# Usage

usage() {
	cat <<- EOF
	Usage: ${0##*/} [OPTION]... [FILE]...
	Search for imports of certain Python standard libraries in each FILE,
	generate a list of suggested package dependencies.

	With no FILE, or when FILE is -, read standard input.

	Options:
	  -c WHEN   use color in output;
	            WHEN is 'always', 'never', or 'auto' (default: '$grep_output_default_color_when')
	  -h        display this help text and exit
	  -m NUM    show max NUM matches per package per file (default: $grep_output_default_max_count);
	            use 0 to show all matches
	  -n NAME   when one or no FILE is given, use NAME instead of FILE in
	            displayed information
	  -q        show suggested dependencies only
	  -v        show verbose output (also show all matches)
	  -x NUM    show NUM lines of context (default: $grep_output_default_context_num)

	EOF
}


# Imports search

get_package_modules() {
	local line="$1"
	local field_num=0
	local IFS=:

	for field in $line; do
		# strip leading and trailing whitespace
		field="${field#"${field%%[! 	]*}"}"
		field="${field%"${field##*[! 	]}"}"

		# set variables in search_path()
		if [ "$field_num" -eq 0 ]; then
			package="$field"
			field_num=1
		elif [ "$field_num" -eq 1 ]; then
			modules="$field"
			field_num=2
		else
			field_num=3
		fi
	done

	if [ "$field_num" -ne 2 ] || [ -z "$package" ] || [ -z "$modules" ]; then
		log_error "invalid package data \"$line\""
		exit "$ex_software"
	fi
}

search_path_for_modules() {
	local path="$1"
	local path_is_dir="$2"
	local path_is_stdin="$3"
	local package="$4"
	local modules="$5"
	local modules_regex
	local regex
	local remove_dir_prefix
	local grep_results
	local IFS="$oifs"

	log_debug "    Looking for modules in $package ($modules)"

	modules_regex=$(printf '%s' "$modules" | sed -e 's/\./\\./g' -e 's/\s\+/|/g')
	regex="\b(import\s+($modules_regex)|from\s+($modules_regex)\S*\s+import)\b"

	if [ -n "$path_is_dir" ]; then
		remove_dir_prefix="s|^\(\(\x1b[[0-9;]*[mK]\)*\)$path|\1|"
		grep_results=$($grep $grep_output_options $grep_dir_filters -E "$regex" "$path")

	elif [ -n "$path_is_stdin" ]; then
		grep_results=$(printf '%s\n' "$stdin" | $grep $grep_output_options -E "$regex")

	else
		grep_results=$($grep $grep_output_options -E "$regex" "$path")
	fi

	if [ "$?" -ne 0 ]; then
		log_debug "    No imports found"
		log_debug ""
		return 0
	fi

	log_info "    Found imports for: $modules"

	if can_log_info; then
		printf '%s\n' "$grep_results" | sed -e "$remove_dir_prefix" -e "s/^/        /"
	fi

	log_info ""

	# set variable in search_path()
	suggested="$suggested +$package"
}

search_path() {
	local path="$1"
	local name="$2"
	local path_is_dir
	local path_is_stdin
	local package
	local modules
	local suggested
	local IFS="$newline"

	if [ "$path" = "-" ]; then
		path_is_stdin=1

	else
		if ! [ -e "$path" ]; then
			log_error "$path does not exist"
			exit "$ex_noinput"
		fi

		if [ -d "$path" ]; then
			path="${path%/}/"
			path_is_dir=1
		fi
	fi

	log_info ""
	log_info "Searching $name (showing $grep_output_description):"
	log_info ""

	if [ -n "$path_is_stdin" ]; then
		stdin="$(cat)"
	fi

	for line in $packages; do
		# strip leading whitespace
		line="${line#"${line%%[! 	]*}"}"

		# skip blank lines or comments (beginning with #)
		if [ -z "$line" ] ||  [ "$line" != "${line###}" ]; then
			continue
		fi

		package=
		modules=

		get_package_modules "$line"
		search_path_for_modules "$path" "$path_is_dir" "$path_is_stdin" "$package" "$modules"
	done

	log_notice "Suggested dependencies for $name:"

	if [ -z "$suggested" ]; then
		suggested="(none)"
	fi
	IFS="$oifs"
	for package in $suggested; do
		log_notice "    $package"
	done

	log_notice ""
}


# Find GNU grep

if ggrep --version 2>&1 | grep -q GNU; then
	grep="ggrep"
elif grep --version 2>&1 | grep -q GNU; then
	grep="grep"
else
	log_error "cannot find GNU grep"
	exit "$ex_unavailable"
fi


# Process environment variables

case $PYTHON3_FIND_STDLIB_DEPENDS_LOG_LEVEL in
	notice)
		log_level="$log_level_notice"
		;;
	info)
		log_level="$log_level_info"
		;;
	debug)
		log_level="$log_level_debug"
		;;
	*)
		log_level="$log_level_default"
		;;
esac

grep_output_max_count="${PYTHON3_FIND_STDLIB_DEPENDS_MAX_COUNT:-$grep_output_default_max_count}"
grep_output_color_when="${PYTHON3_FIND_STDLIB_DEPENDS_COLOR_WHEN:-$grep_output_default_color_when}"
grep_output_line_prefix="${PYTHON3_FIND_STDLIB_DEPENDS_LINE_PREFIX:-$grep_output_default_line_prefix}"
grep_output_context_num="${PYTHON3_FIND_STDLIB_DEPENDS_CONTEXT_NUM:-$grep_output_default_context_num}"


# Process command line options

while getopts c:hm:n:qvx: OPT; do
	case $OPT in
		c)
			grep_output_color_when="$OPTARG"
			;;
		h)
			usage
			exit 0
			;;
		m)
			grep_output_max_count="$OPTARG"
			;;
		n)
			output_name="$OPTARG"
			;;
		q)
			log_level="$log_level_notice"
			;;
		v)
			log_level="$log_level_debug"
			;;
		x)
			grep_output_context_num="$OPTARG"
			;;
		\?)
			usage
			exit "$ex_usage"
			;;
	esac
done

shift $((OPTIND - 1))


# Set up grep output options

if can_log_info; then
	if [ "$grep_output_color_when" = "auto" ]; then
		if [ -t 1 ]; then
			grep_output_color_when="always"
		else
			grep_output_color_when="never"
		fi
	fi

	if ! can_log_debug && [ "$grep_output_max_count" -gt 0 ]; then
		grep_output_options="-m $grep_output_max_count"

		if [ "$grep_output_max_count" -eq 1 ]; then
			grep_output_description="max 1 match per file"
		else
			grep_output_description="max $grep_output_max_count matches per file"
		fi

	else
		grep_output_description="all matches"
	fi

	if [ "$grep_output_context_num" -gt 0 ]; then
		grep_output_options="$grep_output_options -C $grep_output_context_num"

		if [ "$grep_output_context_num" -eq 1 ]; then
			grep_output_description="$grep_output_description, 1 line of context"
		else
			grep_output_description="$grep_output_description, $grep_output_context_num lines of context"
		fi
	fi

	grep_output_options="$grep_output_options --color=$grep_output_color_when $grep_output_line_prefix"

else
	grep_output_options="-q"
fi


# Main

if [ "$#" -gt 0 ]; then
	is_first_search=1

	if [ "$#" -gt 1 ]; then
		output_name=
	fi

	for path; do
		if [ -z "$is_first_search" ]; then
			log_info "===="
		fi

		if [ -z "$output_name" ]; then
			if [ "$path" = "-" ]; then
				output_name="$stdin_name"
			else
				output_name="$path"
			fi
		fi

		search_path "$path" "$output_name"

		is_first_search=
		output_name=
	done

else
	search_path "-" "${output_name:-$stdin_name}"
fi
