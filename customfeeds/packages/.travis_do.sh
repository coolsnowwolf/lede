#!/bin/bash
#
# MIT Alexander Couzens <lynxis@fe80.eu>

set -e

SDK_HOME="$HOME/sdk"
SDK_PATH=https://downloads.openwrt.org/snapshots/targets/mpc85xx/p2020/
SDK=-sdk-mpc85xx-p2020_
PACKAGES_DIR="$PWD"

echo_red()   { printf "\033[1;31m$*\033[m\n"; }
echo_green() { printf "\033[1;32m$*\033[m\n"; }
echo_blue()  { printf "\033[1;34m$*\033[m\n"; }

exec_status() {
	PATTERN="$1"
	shift
	while :;do sleep 590;echo "still running (please don't kill me Travis)";done &
	("$@" 2>&1) | tee logoutput
	R=${PIPESTATUS[0]}
	kill $! && wait $! 2>/dev/null
	if [ $R -ne 0 ]; then
		echo_red   "=> '$*' failed (return code $R)"
		return 1
	fi
	if grep -qE "$PATTERN" logoutput; then
		echo_red   "=> '$*' failed (log matched '$PATTERN')"
		return 1
	fi

	echo_green "=> '$*' successful"
	return 0
}

get_sdk_file() {
	if [ -e "$SDK_HOME/sha256sums" ] ; then
		grep -- "$SDK" "$SDK_HOME/sha256sums" | awk '{print $2}' | sed 's/*//g'
	else
		false
	fi
}

# download will run on the `before_script` step
# The travis cache will be used (all files under $HOME/sdk/). Meaning
# We don't have to download the file again
download_sdk() {
	mkdir -p "$SDK_HOME"
	cd "$SDK_HOME"

	echo_blue "=== download SDK"
	wget "$SDK_PATH/sha256sums" -O sha256sums
	wget "$SDK_PATH/sha256sums.gpg" -O sha256sums.asc

	# LEDE Build System (LEDE GnuPG key for unattended build jobs)
	gpg --import $PACKAGES_DIR/.keys/626471F1.asc
	echo '54CC74307A2C6DC9CE618269CD84BCED626471F1:6:' | gpg --import-ownertrust
	# LEDE Release Builder (17.01 "Reboot" Signing Key)
	gpg --import $PACKAGES_DIR/.keys/D52BBB6B.asc
	echo 'B09BE781AE8A0CD4702FDCD3833C6010D52BBB6B:6:' | gpg --import-ownertrust

	echo_blue "=== Verifying sha256sums signature"
	gpg --verify sha256sums.asc
	echo_blue "=== Verified sha256sums signature."
	if ! grep -- "$SDK" sha256sums > sha256sums.small ; then
		echo_red "=== Can not find $SDK file in sha256sums."
		echo_red "=== Is \$SDK out of date?"
		false
	fi

	# if missing, outdated or invalid, download again
	if ! sha256sum -c ./sha256sums.small ; then
		local sdk_file
		sdk_file="$(get_sdk_file)"
		echo_blue "=== sha256 doesn't match or SDK file wasn't downloaded yet."
		echo_blue "=== Downloading a fresh version"
		wget "$SDK_PATH/$sdk_file" -O "$sdk_file"
	fi

	# check again and fail here if the file is still bad
	echo_blue "Checking sha256sum a second time"
	if ! sha256sum -c ./sha256sums.small ; then
		echo_red "=== SDK can not be verified!"
		false
	fi
	echo_blue "=== SDK is up-to-date"
}

# test_package will run on the `script` step.
# test_package call make download check for very new/modified package
test_packages2() {
	local commit_range=$TRAVIS_COMMIT_RANGE
	if [ -z "$TRAVIS_PULL_REQUEST_SHA" ]; then
		echo_blue "Using only the latest commit, since we're not in a Pull Request"
		commit_range=HEAD~1
	fi

	# search for new or modified packages. PKGS will hold a list of package like 'admin/muninlite admin/monit ...'
	PKGS=$(git diff --diff-filter=d --name-only "$commit_range" | grep 'Makefile$' | grep -v '/files/' | awk -F'/Makefile' '{ print $1 }')

	if [ -z "$PKGS" ] ; then
		echo_blue "No new or modified packages found!"
		return 0
	fi

	echo_blue "=== Found new/modified packages:"
	for pkg in $PKGS ; do
		echo "===+ $pkg"
	done

	echo_blue "=== Setting up SDK"
	tmp_path=$(mktemp -d)
	cd "$tmp_path"
	tar Jxf "$SDK_HOME/$(get_sdk_file)" --strip=1

	# use github mirrors to spare lede servers
	cat > feeds.conf <<EOF
src-git base https://github.com/lede-project/source.git
src-link packages $PACKAGES_DIR
src-git luci https://github.com/openwrt/luci.git
EOF

	# enable BUILD_LOG
	sed -i '1s/^/config BUILD_LOG\n\tbool\n\tdefault y\n\n/' Config-build.in

	./scripts/feeds update -a > /dev/null
	./scripts/feeds install -a > /dev/null
	make defconfig > /dev/null
	echo_blue "=== Setting up SDK done"

	RET=0
	# E.g: pkg_dir => admin/muninlite
	# pkg_name => muninlite
	for pkg_dir in $PKGS ; do
		pkg_name=$(echo "$pkg_dir" | awk -F/ '{ print $NF }')
		echo_blue "=== $pkg_name: Starting quick tests"

		exec_status '^ERROR' make "package/$pkg_name/download" V=s || RET=1
		badhash_msg_regex="HASH does not match "
		badhash_msg_regex="$badhash_msg_regex|HASH uses deprecated hash,"
		badhash_msg_regex="$badhash_msg_regex|HASH is missing,"
		exec_status '^ERROR'"|$badhash_msg_regex" make "package/$pkg_name/check" V=s || RET=1

		echo_blue "=== $pkg_name: quick tests done"
	done

	[ $RET -ne 0 ] && return $RET

	for pkg_dir in $PKGS ; do
		pkg_name=$(echo "$pkg_dir" | awk -F/ '{ print $NF }')
		echo_blue "=== $pkg_name: Starting compile test"

		# we can't enable verbose built else we often hit Travis limits
		# on log size and the job get killed
		exec_status '^ERROR' make "package/$pkg_name/compile" -j3 || RET=1

		echo_blue "=== $pkg_name: compile test done"

		echo_blue "=== $pkg_name: begin compile logs"
		for f in $(find logs/package/feeds/packages/$pkg_name/ -type f); do
			echo_blue "Printing last 200 lines of $f"
			tail -n200 "$f"
		done
		echo_blue "=== $pkg_name: end compile logs"

		echo_blue "=== $pkg_name: begin packages sizes"
		du -ba bin/
		echo_blue "=== $pkg_name: end packages sizes"
	done

	return $RET
}

test_commits() {
	RET=0
	if [ -z "$TRAVIS_PULL_REQUEST_SHA" ]; then
		echo_blue "Skipping commits tests (not in a Pull Request)"
		return 0
	fi
	for commit in $(git rev-list ${TRAVIS_COMMIT_RANGE/.../..}); do
		echo_blue "=== Checking commit '$commit'"
		if git show --format='%P' -s $commit | grep -qF ' '; then
			echo_red "Pull request should not include merge commits"
			RET=1
		fi

		author="$(git show -s --format=%aN $commit)"
		if echo $author | grep -q '\S\+\s\+\S\+'; then
			echo_green "Author name ($author) seems ok"
		fi

		subject="$(git show -s --format=%s $commit)"
		if echo "$subject" | grep -q -e '^[0-9A-Za-z,/_-]\+: ' -e '^Revert '; then
			echo_green "Commit subject line seems ok ($subject)"
		fi

		body="$(git show -s --format=%b $commit)"
		sob="$(git show -s --format='Signed-off-by: %aN <%aE>' $commit)"
		if echo "$body" | grep -qF "$sob"; then
			echo_green "Signed-off-by match author"
		fi
	done

	return $RET
}

test_packages() {
	test_commits && test_packages2 || return 1
}

echo_blue "=== Travis ENV"
env
echo_blue "=== Travis ENV"

if [ -n "$TRAVIS_PULL_REQUEST_SHA" ]; then
	while true; do
		# if clone depth is too small, git rev-list / diff return incorrect or empty results
		C="$(git rev-list ${TRAVIS_COMMIT_RANGE/.../..} | tail -n1)" 2>/dev/null
		[ -n "$C" -a "$C" != "a22de9b74cf9579d1ce7e6cf1845b4afa4277b00" ] && break
		echo_blue "Fetching 50 commits more"
		git fetch origin --deepen=50
	done
fi

if [ $# -ne 1 ] ; then
	cat <<EOF
Usage: $0 (download_sdk|test_packages)
download_sdk - download the SDK to $HOME/sdk.tar.xz
test_packages - do a make check on the package
EOF
	exit 1
fi

$@
