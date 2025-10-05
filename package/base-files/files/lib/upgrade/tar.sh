# SPDX-License-Identifier: GPL-2.0-or-later OR MIT

# Example usage:
#
# {
#         tar_print_member "date.txt" "It's $(date +"%Y")"
#         tar_print_trailer
# } > test.tar

__tar_print_padding() {
	dd if=/dev/zero bs=1 count=$1 2>/dev/null
}

tar_print_member() {
	local name="$1"
	local content="$2"
	local mtime="${3:-$(date +%s)}"
	local mode=644
	local uid=0
	local gid=0
	local size=${#content}
	local type=0
	local link=""
	local username="root"
	local groupname="root"

	# 100 byte of padding bytes, using 0x01 since the shell does not tolerate null bytes in strings
	local pad=$'\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1'

	# validate name (strip leading slash if present)
	name=${name#/}

	# truncate string header values to their maximum length
	name=${name:0:100}
	link=${link:0:100}
	username=${username:0:32}
	groupname=${groupname:0:32}

	# construct header part before checksum field
	local header1="${name}${pad:0:$((100 - ${#name}))}"
	header1="${header1}$(printf '%07d\1' $mode)"
	header1="${header1}$(printf '%07o\1' $uid)"
	header1="${header1}$(printf '%07o\1' $gid)"
	header1="${header1}$(printf '%011o\1' $size)"
	header1="${header1}$(printf '%011o\1' $mtime)"

	# construct header part after checksum field
	local header2="$(printf '%d' $type)"
	header2="${header2}${link}${pad:0:$((100 - ${#link}))}"
	header2="${header2}ustar  ${pad:0:1}"
	header2="${header2}${username}${pad:0:$((32 - ${#username}))}"
	header2="${header2}${groupname}${pad:0:$((32 - ${#groupname}))}"

	# calculate checksum over header fields
	local checksum=0
	for byte in $(printf '%s%8s%s' "$header1" "" "$header2" | tr '\1' '\0' | hexdump -ve '1/1 "%u "'); do
		checksum=$((checksum + byte))
	done

	# print member header, padded to 512 byte
	printf '%s%06o\0 %s' "$header1" $checksum "$header2" | tr '\1' '\0'
	__tar_print_padding 183

	# print content data, padded to multiple of 512 byte
	printf "%s" "$content"
	__tar_print_padding $((512 - (size % 512)))
}

tar_print_trailer() {
	__tar_print_padding 1024
}
