#!/bin/sh

[ -d ./build ] || {
	echo "Execute as ./build/i18n-sync.sh" >&2
	exit 1
}

./build/mkbasepot.sh

find . -name '*.pot' -and -not -name base.pot -and -not -name rrdtool.pot | \
	while read path; do
		dir="${path%/po/templates/*}"
		echo -n "Updating ${path#./} ... "
		./build/i18n-scan.pl "$dir" > "$path"
		echo "done"
	done

./build/i18n-update.pl
