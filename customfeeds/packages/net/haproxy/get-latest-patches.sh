#!/bin/sh

CLONEURL=https://git.haproxy.org/git/haproxy-2.4.git
BASE_TAG=v2.4.2
TMP_REPODIR=tmprepo
PATCHESDIR=patches

if test -d "${TMP_REPODIR}"; then rm -rf "${TMP_REPODIR}"; fi

git clone "${CLONEURL}" "${TMP_REPODIR}"

printf "Cleaning patches\n"
find ${PATCHESDIR} -type f -name "*.patch" -exec rm -f "{}" \;

i=0
for cid in $(git -C "${TMP_REPODIR}" rev-list ${BASE_TAG}..HEAD | tac); do
	filename="$(printf "%03d" $i)-$(git -C "${TMP_REPODIR}" log --format=%s -n 1 "$cid" | sed -e"s/[()']//g" -e's/[^_a-zA-Z0-9+-]\+/-/g' -e's/-$//').patch"
	printf "Creating %s\n" "${filename}"
	git -C "${TMP_REPODIR}" show "$cid" > "${PATCHESDIR}/$filename"
	git add "${PATCHESDIR}/$filename"
	i=$((i+1))
done

rm -rf "${TMP_REPODIR}"

printf "finished\n"

