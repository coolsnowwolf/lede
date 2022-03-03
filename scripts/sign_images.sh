#!/bin/sh

# directory where search for images
TOP_DIR="${TOP_DIR:-./bin/targets}"
# key to sign images
BUILD_KEY="${BUILD_KEY:-key-build}" # TODO unify naming?
# remove other signatures (added e.g.  by buildbot)
REMOVE_OTER_SIGNATURES="${REMOVE_OTER_SIGNATURES:-1}"

# find all sysupgrade images in TOP_DIR
# factory images don't need signatures as non OpenWrt system doesn't check them anyway
for image in $(find $TOP_DIR -type f -name "*-sysupgrade.bin"); do
	# check if image actually support metadata
	if fwtool -i /dev/null "$image"; then
		# remove all previous signatures
		if [ -n "$REMOVE_OTER_SIGNATURES" ]; then
			while [ "$?" = 0 ]; do
				fwtool -t -s /dev/null "$image"
			done
		fi
		# run same operation as build root does for signing
		cp "$BUILD_KEY.ucert" "$image.ucert"
		usign -S -m "$image" -s "$BUILD_KEY" -x "$image.sig"
		ucert -A -c "$image.ucert" -x "$image.sig"
		fwtool -S "$image.ucert" "$image"
	fi
done
