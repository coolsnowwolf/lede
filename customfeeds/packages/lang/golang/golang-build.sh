#!/bin/sh

nl="
"

log() {
	# shellcheck disable=SC2039
	local IFS=" "
	printf '%s\n' "$*"
}

log_error() {
	# shellcheck disable=SC2039
	local IFS=" "
	printf 'Error: %s\n' "$*" >&2
}

link_contents() {
	# shellcheck disable=SC2039
	local src="$1" dest="$2" IFS="$nl" dirs dir base

	if [ -n "$(find "$src" -mindepth 1 -maxdepth 1 -name "*.go" -not -type d)" ]; then
		log_error "$src is already a Go library"
		return 1
	fi

	dirs="$(find "$src" -mindepth 1 -maxdepth 1 -type d)"
	for dir in $dirs; do
		base="${dir##*/}"
		if [ -d "$dest/$base" ]; then
			case "$dir" in
			*$GO_BUILD_DEPENDS_SRC/$GO_PKG)
				log "$GO_PKG is already installed. Please check for circular dependencies."
				;;
			*)
				link_contents "$src/$base" "$dest/$base"
				;;
			esac
		else
			log "...${src#$GO_BUILD_DEPENDS_SRC}/$base"
			ln -sf "$src/$base" "$dest/$base"
		fi
	done

	return 0
}

configure() {
	# shellcheck disable=SC2039
	local files code testdata gomod pattern extra IFS file dest

	cd "$BUILD_DIR" || return 1

	files="$(find ./ -path "*/.*" -prune -o -not -type d -print)"

	if [ "$GO_INSTALL_ALL" != 1 ]; then
		code="$(printf '%s\n' "$files" | grep '\.\(c\|cc\|cpp\|go\|h\|hh\|hpp\|proto\|s\)$')"
		testdata="$(printf '%s\n' "$files" | grep '/testdata/')"
		gomod="$(printf '%s\n' "$files" | grep '/go\.\(mod\|sum\)$')"

		for pattern in $GO_INSTALL_EXTRA; do
			extra="$(printf '%s\n' "$extra"; printf '%s\n' "$files" | grep -e "$pattern")"
		done

		files="$(printf '%s\n%s\n%s\n%s\n' "$code" "$testdata" "$gomod" "$extra" | grep -v '^[[:space:]]*$' | sort -u)"
	fi

	IFS="$nl"

	log "Copying files from $BUILD_DIR into $GO_BUILD_DIR/src/$GO_PKG"
	mkdir -p "$GO_BUILD_DIR/src"
	for file in $files; do
		log "${file#./}"
		dest="$GO_BUILD_DIR/src/$GO_PKG/${file#./}"
		mkdir -p "${dest%/*}"
		cp -fpR "$file" "$dest"
	done
	log

	if [ "$GO_SOURCE_ONLY" != 1 ]; then
		if [ -d "$GO_BUILD_DEPENDS_SRC" ]; then
			log "Symlinking directories from $GO_BUILD_DEPENDS_SRC into $GO_BUILD_DIR/src"
			link_contents "$GO_BUILD_DEPENDS_SRC" "$GO_BUILD_DIR/src"
		else
			log "$GO_BUILD_DEPENDS_SRC does not exist, skipping symlinks"
		fi
	else
		log "Not building binaries, skipping symlinks"
	fi
	log

	return 0
}

build() {
	# shellcheck disable=SC2039
	local modargs pattern targets retval

	cd "$GO_BUILD_DIR" || return 1

	if [ -f "$BUILD_DIR/go.mod" ] ; then
		mkdir -p "$GO_MOD_CACHE_DIR"
		modargs="$GO_MOD_ARGS"
	fi

	log "Finding targets"
	# shellcheck disable=SC2086
	targets="$(go list $modargs $GO_BUILD_PKG)"
	for pattern in $GO_EXCLUDES; do
		targets="$(printf '%s\n' "$targets" | grep -v "$pattern")"
	done
	log

	if [ "$GO_GO_GENERATE" = 1 ]; then
		log "Calling go generate"
		# shellcheck disable=SC2086
		GOOS='' GOARCH='' GO386='' GOARM='' GOMIPS='' GOMIPS64='' \
		go generate -v $targets
		log
	fi

	if [ "$GO_SOURCE_ONLY" = 1 ]; then
		return 0
	fi

	log "Building targets"
	mkdir -p "$GO_BUILD_DIR/bin" "$GO_BUILD_CACHE_DIR"
	# shellcheck disable=SC2086
	go install $modargs "$@" $targets
	retval="$?"
	log

	if [ "$retval" -eq 0 ] && [ -z "$(find "$GO_BUILD_BIN_DIR" -maxdepth 0 -type d -not -empty 2>/dev/null)" ]; then
		log_error "No binaries were built"
		retval=1
	fi

	if [ "$retval" -ne 0 ]; then
		cache_cleanup
	fi

	return "$retval"
}

install_bin() {
	# shellcheck disable=SC2039
	local dest="$1"
	install -d -m0755 "$dest/$GO_INSTALL_BIN_PATH"
	install -m0755 "$GO_BUILD_BIN_DIR"/* "$dest/$GO_INSTALL_BIN_PATH/"
}

install_src() {
	# shellcheck disable=SC2039
	local dest="$1" dir="${GO_PKG%/*}"
	install -d -m0755 "$dest/$GO_BUILD_DEPENDS_PATH/src/$dir"
	cp -fpR "$GO_BUILD_DIR/src/$GO_PKG" "$dest/$GO_BUILD_DEPENDS_PATH/src/$dir/"
}

cache_cleanup() {
	if ! [ -d "$GO_MOD_CACHE_DIR" ]; then
		return 0
	fi

	# in case go is called without -modcacherw
	find "$GO_MOD_CACHE_DIR" -type d -not -perm -u+w -exec chmod u+w '{}' +

	if [ -n "$CONFIG_GOLANG_MOD_CACHE_WORLD_READABLE" ]; then
		find "$GO_MOD_CACHE_DIR"      -type d -not -perm -go+rx -exec chmod go+rx '{}' +
		find "$GO_MOD_CACHE_DIR" -not -type d -not -perm -go+r  -exec chmod go+r  '{}' +
	fi

	return 0
}


if [ "$#" -lt 1 ]; then
	log_error "Missing command"
	exit 1
fi

command="$1"
shift 1

case "$command" in
	configure)
		configure
		;;
	build)
		build "$@"
		;;
	install_bin)
		install_bin "$@"
		;;
	install_src)
		install_src "$@"
		;;
	cache_cleanup)
		cache_cleanup
		;;
	*)
		log_error "Invalid command \"$command\""
		exit 1
		;;
esac
