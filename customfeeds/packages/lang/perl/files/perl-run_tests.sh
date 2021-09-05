#!/bin/sh

PERL_TESTSDIR="/usr/share/perl/perl-tests"
PERL_LIBDIR="/usr/lib/perl5/%%PERL_VERSION%%/"
PERL_DISABLEDTESTS="%%PERL_DISABLEDTESTS%%"

no_run=""
manual_run=""
manual_run_no_base=""

while [ ! -z "$1" ]; do
	case $1 in
		-n)
			no_run="yes"
			;;
		-m)
			manual_run="yes"
			;;
		-mb)
			manual_run="yes"
			manual_run_no_base="yes"
			;;
		--help)
			echo "run_tests.sh [-n|-m|-mb|--help]"
			echo ""
			echo "Options:"
			echo "	-n        Just prepare the environment. Don't actually run any tests"
			echo "	-m        Run tests manually according to MANIFEST, instead of whatever t/TEST chooses"
			echo "	-mb       Don't run base tests. Implies -m"
			echo "	--help    Print this help ;)"
			echo ""
			exit 0
			;;
		*)
			echo "Invalid argument: $1"
			;;
	esac
	shift
done

if [ ! -f "$PERL_TESTSDIR/__prepared" ]; then
	# Many tests insist on having PERL5LIB in $PERL_TESTSDIR/lib. However,
	# that directory may also contain tests. Some of them(FindBin.t in particular)
	# also demand being located in a directory ending with "lib". So we can't do symlink
	# trickery here.
	# Our solution is to just copy PERL5LIB over.
	if [ -d "$PERL_TESTSDIR/lib" ]; then
		cp -a "$PERL_LIBDIR/"* "$PERL_TESTSDIR/lib/"
	else
		ln -s "$PERL_LIBDIR" "$PERL_TESTSDIR/lib"
	fi

	ln -s /usr/bin/perl "$PERL_TESTSDIR/perl"
	ln -s /usr/bin/perl "$PERL_TESTSDIR/t/perl"
	touch "$PERL_TESTSDIR/__prepared"
	
	for i in $PERL_DISABLEDTESTS; do
		echo "Disabling $i tests"
		sed 's!^'$i'.*$!!' -i $PERL_TESTSDIR/MANIFEST
	done
	
	cat $PERL_TESTSDIR/MANIFEST | grep -v '^$' > $PERL_TESTSDIR/MANIFEST_NEW
	rm $PERL_TESTSDIR/MANIFEST
	mv $PERL_TESTSDIR/MANIFEST_NEW $PERL_TESTSDIR/MANIFEST
fi

if [ -z "$no_run" ]; then
	cd "$PERL_TESTSDIR/t"
	if [ ! -z "$manual_run" ]; then
		for i in $(cat ../MANIFEST | sed 's/\t.*$//g' | grep '\.t$'); do
			if [ ! -z "$manual_run_no_base" ] && [ ! -z "$(echo $i | grep '^t/')" ]; then
				continue;
			fi
			echo "Running $i"
			./TEST ../$i
			echo ""
		done
	else
		./perl TEST
	fi
fi
