#!/bin/sh
set -e

process_filespec() {
	local src_dir="$1"
	local dst_dir="$2"
	local filespec="$3"
	echo "$filespec" | (
	IFS='|'
	while read fop fspec fperm; do
		local fop=`echo "$fop" | tr -d ' \t\n'`
		if [ "$fop" = "+" ]; then
			if [ ! -e "${src_dir}${fspec}" ]; then
				echo "File not found '${src_dir}${fspec}'"
				exit 1
			fi
			dpath=`dirname "$fspec"`
			if [ -z "$fperm" ]; then
				dperm=`stat -c "%a" ${src_dir}${dpath}`
			fi
			mkdir -p -m$dperm ${dst_dir}${dpath}
			echo "copying: '$fspec'"
			cp -fpR ${src_dir}${fspec} ${dst_dir}${dpath}/
			if [ -n "$fperm" ]; then
				chmod -R $fperm ${dst_dir}${fspec}
			fi
		elif [ "$fop" = "-" ]; then
			echo "removing: '$fspec'"
			rm -fR ${dst_dir}${fspec}
		elif [ "$fop" = "=" ]; then
			echo "setting permissions: '$fperm' on '$fspec'"
			chmod -R $fperm ${dst_dir}${fspec}
		fi
	done
	)
}

src_dir="$1"
dst_dir="$2"
filespec="$3"

process_filespec "$src_dir" "$dst_dir" "$filespec" || {
	echo "process filespec error-ed"
	exit 1
}
