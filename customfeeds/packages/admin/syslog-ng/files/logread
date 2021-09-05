#!/bin/sh
# Shell script compatibility wrapper for /sbin/logread
#
# Copyright (C) 2019 Dirk Brenken <dev@brenken.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

logfile="/var/log/messages"

if [ ! -f "${logfile}" ]
then
	printf "%s\n" "Error: logfile not found!"
	exit 2
fi

usage()
{
	printf "%s\n" "Usage: logread [options]"
	printf "%s\n" "Options:"
	printf "%5s %-10s%s\n" "-l" "<count>" "Got only the last 'count' messages"
	printf "%5s %-10s%s\n" "-e" "<pattern>" "Filter messages with a regexp"
	printf "%5s %-10s%s\n" "-f" "" "Follow log messages"
	printf "%5s %-10s%s\n" "-h" "" "Print this help message"
}

if [ -z "${1}" ]
then
	cat "${logfile}"
	exit 0
else
	while [ "${1}" ]
	do
		case "${1}" in
			-l)
				shift
				count="${1//[^0-9]/}"
				tail -n "${count:-50}" "${logfile}"
				exit 0
				;;
			-e)
				shift
				pattern="${1}"
				grep -E "${pattern}" "${logfile}"
				exit 0
				;;
			-f)
				tail -f "${logfile}"
				exit 0
				;;
			-fe)
				shift
				pattern="${1}"
				tail -f "${logfile}" | grep -E "${pattern}"
				exit 0
				;;
			-h|*)
				usage
				exit 1
				;;
		esac
		shift
	done
fi
