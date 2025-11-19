uint_max=4294967295

d_10_0_0_0=167772160
d_10_255_255_255=184549375

d_172_16_0_0=2886729728
d_172_31_255_255=2887778303

d_192_168_0_0=3232235520
d_192_168_255_255=3232301055

d_169_254_0_0=2851995648
d_169_254_255_255=2852061183

d_127_0_0_0=2130706432
d_127_255_255_255=2147483647

d_224_0_0_0=3758096384
d_239_255_255_255=4026531839

# check that $1 is only base 10 digits, and that it doesn't
# exceed 2^32-1
assert_uint32() {
    local __n="$1"

    if [ -z "$__n" -o -n "${__n//[0-9]/}" ]; then
	printf "Not a decimal integer (%s)\n" "$__n ">&2
	return 1
    fi

    if [ "$__n" -gt $uint_max ]; then
	printf "Out of range (%s)\n" "$__n" >&2
	return 1
    fi

    if [ "$((__n + 0))" != "$__n" ]; then
	printf "Not normalized notation (%s)\n" "$__n" >&2
	return 1
    fi

    return 0
}

# return a count of the number of bits set in $1
bitcount() {
    local __var="$1" __c="$2"
    assert_uint32 "$__c" || return 1

    __c=$((((__c >> 1) & 0x55555555) + (__c & 0x55555555)))
    __c=$((((__c >> 2) & 0x33333333) + (__c & 0x33333333)))
    __c=$((((__c >> 4) & 0x0f0f0f0f) + (__c & 0x0f0f0f0f)))
    __c=$((((__c >> 8) & 0x00ff00ff) + (__c & 0x00ff00ff)))
    __c=$((((__c >> 16) & 0x0000ffff) + (__c & 0x0000ffff)))

    export -- "$__var=$__c"
}

# tedious but portable with busybox's limited shell
# we check each octet to be in the range of 0..255,
# and also make sure there's no extaneous characters.
str2ip() {
    local __var="$1" __ip="$2" __n __val=0

    case "$__ip" in
    [0-9].*)
	__n="${__ip:0:1}"
	__ip="${__ip:2}"
	;;
    [1-9][0-9].*)
	__n="${__ip:0:2}"
	__ip="${__ip:3}"
	;;
    1[0-9][0-9].*|2[0-4][0-9].*|25[0-5].*)
	__n="${__ip:0:3}"
	__ip="${__ip:4}"
	;;
    *)
	printf "Not a dotted quad (%s)\n" "$2" >&2
	return 1
	;;
    esac

    __val=$((__n << 24))

    case "$__ip" in
    [0-9].*)
	__n="${__ip:0:1}"
	__ip="${__ip:2}"
	;;
    [1-9][0-9].*)
	__n="${__ip:0:2}"
	__ip="${__ip:3}"
	;;
    1[0-9][0-9].*|2[0-4][0-9].*|25[0-5].*)
	__n="${__ip:0:3}"
	__ip="${__ip:4}"
	;;
    *)
	printf "Not a dotted quad (%s)\n" "$2" >&2
	return 1
	;;
    esac

    __val=$((__val + (__n << 16)))

    case "$__ip" in
    [0-9].*)
	__n="${__ip:0:1}"
	__ip="${__ip:2}"
	;;
    [1-9][0-9].*)
	__n="${__ip:0:2}"
	__ip="${__ip:3}"
	;;
    1[0-9][0-9].*|2[0-4][0-9].*|25[0-5].*)
	__n="${__ip:0:3}"
	__ip="${__ip:4}"
	;;
    *)
	printf "Not a dotted quad (%s)\n" "$2" >&2
	return 1
	;;
    esac

    __val=$((__val + (__n << 8)))

    case "$__ip" in
    [0-9])
	__n="${__ip:0:1}"
	__ip="${__ip:1}"
	;;
    [1-9][0-9])
	__n="${__ip:0:2}"
	__ip="${__ip:2}"
	;;
    1[0-9][0-9]|2[0-4][0-9]|25[0-5])
	__n="${__ip:0:3}"
	__ip="${__ip:3}"
	;;
    *)
	printf "Not a dotted quad (%s)\n" "$2" >&2
	return 1
	;;
    esac

    __val=$((__val + __n))

    if [ -n "$__ip" ]; then
	printf "Not a dotted quad (%s)\n" "$2" >&2
	return 1
    fi

    export -- "$__var=$__val"
    return 0
}

# convert back from an integer to dotted-quad.
ip2str() {
    local __var="$1" __n="$2"
    assert_uint32 "$__n" || return 1

    export -- "$__var=$((__n >> 24)).$(((__n >> 16) & 255)).$(((__n >> 8) & 255)).$((__n & 255))"
}

# convert prefix into an integer bitmask
prefix2netmask() {
    local __var="$1" __n="$2"
    assert_uint32 "$__n" || return 1

    if [ "$__n" -gt 32 ]; then
	printf "Prefix out-of-range (%s)" "$__n" >&2
	return 1
    fi

    export -- "$__var=$(((~(uint_max >> __n)) & uint_max))"
}

_is_contiguous() {
    local __x="$1"	# no checking done
    local __y=$((~__x & uint_max))
    local __z=$(((__y + 1) & uint_max))

    [ $((__z & __y)) -eq 0 ]
}

# check argument as being contiguous upper bits (and yes,
# 0 doesn't have any discontiguous bits).
is_contiguous() {
    local __var="$1" __x="$2" __val=0
    assert_uint32 "$__x" || return 1

    local __y=$((~__x & uint_max))
    local __z=$(((__y + 1) & uint_max))

    [ $((__z & __y)) -eq 0 ] && __val=1

    export -- "$__var=$__val"
}

# convert mask to prefix, validating that it's a conventional
# (contiguous) netmask.
netmask2prefix() {
    local __var="$1" __n="$2" __cont __bits
    assert_uint32 "$__n" || return 1

    is_contiguous __cont "$__n" || return 1
    if [ $__cont -eq 0 ]; then
	printf "Not a contiguous netmask (%08x)\n" "$__n" >&2
	return 1
    fi

    bitcount __bits "$__n"		# already checked

    export -- "$__var=$__bits"
}

# check the argument as being an rfc-1918 address
is_rfc1918() {
    local __var="$1" __x="$2" __val=0
    assert_uint32 "$__x" || return 1

    if [ $d_10_0_0_0 -le $__x ] && [ $__x -le $d_10_255_255_255 ]; then
	__val=1
    elif [ $d_172_16_0_0 -le $__x ] && [ $__x -le $d_172_31_255_255 ]; then
	__val=1
    elif [ $d_192_168_0_0 -le $__x ] && [ $__x -le $d_192_168_255_255 ]; then
	__val=1
    fi

    export -- "$__var=$__val"
}

# check the argument as being an rfc-3927 address
is_rfc3927() {
    local __var="$1" __x="$2" __val=0
    assert_uint32 "$__x" || return 1

    if [ $d_169_254_0_0 -le $__x ] && [ $__x -le $d_169_254_255_255 ]; then
	__val=1
    fi

    export -- "$__var=$__val"
}

# check the argument as being an rfc-1122 loopback address
is_loopback() {
    local __var="$1" __x="$2" __val=0
    assert_uint32 "$__x" || return 1

    if [ $d_127_0_0_0 -le $__x ] && [ $__x -le $d_127_255_255_255 ]; then
	__val=1
    fi

    export -- "$__var=$__val"
}

# check the argument as being a multicast address
is_multicast() {
    local __var="$1" __x="$2" __val=0
    assert_uint32 "$__x" || return 1

    if [ $d_224_0_0_0 -le $__x ] && [ $__x -le $d_239_255_255_255 ]; then
	__val=1
    fi

    export -- "$__var=$__val"
}

