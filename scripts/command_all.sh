#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Reduced version of which -a using command utility

case $PATH in
	(*[!:]:) PATH="$PATH:" ;;
esac

for ELEMENT in $(echo $PATH | tr ":" "\n"); do
        PATH=$ELEMENT command -v "$@"
done
