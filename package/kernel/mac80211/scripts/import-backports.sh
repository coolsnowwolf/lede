#!/usr/bin/env bash
BASE=$1; shift

usage() {
	echo "Usage: $0 NNN <file>..."
	exit 1
}

check_number() {
	case "$1" in
		[0-9][0-9][0-9]) return 0;;
	esac
	return 1;
}

patch_header()
{
	awk '
	/^(---|\*\*\*|Index:)[ \t][^ \t]|^diff -/ \
		{ exit }
		{ print }
	'
}

strip_diffstat()
{
	awk '
	/#? .* \| / \
		{ eat = eat $0 "\n"
		  next }
	/^#? .* files? changed(, .* insertions?\(\+\))?(, .* deletions?\(-\))?/ \
		{ eat = ""
		  next }
		{ print eat $0
		  eat = "" }
	'
}

strip_trailing_whitespace() {
	sed -e 's:[ '$'\t'']*$::'
}

fixup_header() {
	awk '
		/^From / { next }
		/^Subject: / {
			sub("Subject: \\[[^\]]*\\]", "Subject: [PATCH]")
		}
		{ print }
	'
}

check_number "$BASE" || usage

quilt series > /dev/null || {
	echo "Not in quilt directory"
	exit 2
}

get_next() {
	NEW=$BASE
	quilt series | while read CUR; do
		[ -n "$CUR" ] || break
		CUR=${CUR%%-*}
		check_number "$CUR" || continue
		[ "$CUR" -lt "$NEW" ] && continue
		[ "$CUR" -ge "$(($BASE + 100))" ] && continue
		NEW="$(($CUR + 1))"
		echo $NEW
	done | tail -n1
}

CUR=$(get_next)
CUR="${CUR:-$BASE}"

while [ -n "$1" ]; do
	FILE="$1"; shift
	NAME="$(basename $FILE)"
	NAME="${NAME#[0-9]*-}"
	echo -n "Processing patch $NAME: "

	[ -e "$FILE" ] || {
		echo "file $FILE not found"
		exit 1
	}

	grep -qE "$NAME$" patches/series && {
		echo "already applied"
		continue
	}

	quilt new "$CUR-$NAME" || exit 1
	patch_header < "$FILE" |
		strip_diffstat |
		strip_trailing_whitespace |
		fixup_header > "patches/$CUR-$NAME"

	quilt fold < "$FILE" || {
		cp "$FILE" ./cur_patch
		echo "patch $FILE failed to apply, copied to ./cur_patch"
		exit 1
	}

	quilt refresh -p ab --no-index --no-timestamps

	CUR="$(($CUR + 1))"
done

exit 0
