#!/bin/sh

#
# see:
#	https://www.howtoforge.com/tutorial/strongswan-based-ipsec-vpn-using-certificates-and-pre-shared-key-on-ubuntu-16-04/
#

PROG=$(basename "$0")

[ -z "$EUID" ] && EUID=$(id -u)

if [ $# -lt 5 ]; then
	echo "Usage: $PROG { -s | -c | -u } country domain organization identities [ ... ]" >&2
	exit 1
fi

case "$1" in
-s)
	S_OPT=1 ;;
-c)
	C_OPT=1 ;;
-u)
	U_OPT=1 ;;
*)
	echo "$PROG: require an option specifying server/client/user credential type" >&2
	exit 1
	;;
esac
shift

C="$1"; shift
DOMAIN="$1"; shift
SHORT_DOMAIN="${DOMAIN%%.*}"
ORG="$1"; shift

# invariants...
SYSCONFDIR=/etc
SWANCTL_DIR="$SYSCONFDIR/swanctl"
: ${KEYINFO:="rsa:4096"}
: ${CADAYS:=3650}
: ${CRTDAYS:=730}

makeDN()
{
	printf "C=%s, O=%s, CN=%s" "$1" "$2" "$3"
}

field()
{
	local arg="$1"
	local nth="$2"

	echo "$arg" | cut -d ':' -f "$nth"
}

genmasterkey()
{
	local keytype keybits

	keytype=$(field "$KEYINFO" 1)
	keybits=$(field "$KEYINFO" 2)

	pki --gen --type "$keytype" --size "$keybits" --outform pem > "$SWANCTL_DIR/private/$SHORT_DOMAIN.key"
	chmod 0400 "$SWANCTL_DIR/private/$SHORT_DOMAIN.key"
}

genca()
{
	local keytype

	keytype=$(field "$KEYINFO" 1)

	pki --self --ca --lifetime "$CADAYS" --in "$SWANCTL_DIR/private/$SHORT_DOMAIN.key" --type "$keytype" \
		--dn "$ROOTDN" --outform pem > "$SWANCTL_DIR/x509ca/$SHORT_DOMAIN.crt"
	chmod 0444 "$SWANCTL_DIR/x509ca/$SHORT_DOMAIN.crt"
}

genclientkey()
{
	local name="$1" keytype keybits

	keytype=$(field "$KEYINFO" 1)
	keybits=$(field "$KEYINFO" 2)

	pki --gen --type "$keytype" --size "$keybits" --outform pem > "$SWANCTL_DIR/private/$name.key"
	chmod 0400 "$SWANCTL_DIR/private/$name.key"
}

gendevcert()
{
	local dn="$1"
	local san="$2"
	local name="$3"

	# reads key from input
	pki --issue --lifetime "$CRTDAYS" \
	      --cacert "$SWANCTL_DIR/x509ca/$SHORT_DOMAIN.crt" \
	      --cakey "$SWANCTL_DIR/private/$SHORT_DOMAIN.key" \
	      --dn "$dn" --san "$san" \
	      ${S_OPT:+--flag serverAuth} \
	      ${S_OPT:---flag clientAuth} \
	      --flag ikeIntermediate \
	      --outform pem > "$SWANCTL_DIR/x509/$name.crt"
	chmod 0444 "$SWANCTL_DIR/x509/$name.crt"
}

gendev()
{
	local keytype

	keytype=$(field "$KEYINFO" 1)

	[ -f "$SWANCTL_DIR/private/$NAME.key" ] || genclientkey "$NAME"

	[ -f "$SWANCTL_DIR/x509/$NAME.crt" ] || \
		pki --pub --in "$SWANCTL_DIR/private/$NAME.key" --type "$keytype" \
			| gendevcert "$DEVDN" "$DEVSAN" "$NAME"
}

setparams()
{
	NAME="$1"

	if [ -n "$U_OPT" ]; then
		DEVSAN="$NAME@$DOMAIN"
		DEVDN="$(makeDN "$C" "$ORG" "$DEVSAN")"
	else
		DEVSAN="$NAME.$DOMAIN"
		DEVDN="$(makeDN "$C" "$ORG" "$NAME")"
	fi
}

umask 077

[ "$EUID" -eq 0 ] || { echo "Must run as root!" >&2 ; exit 1; }

ROOTDN="$(makeDN "$C" "$ORG" "Root CA")"

[ -f "$SWANCTL_DIR/private/$SHORT_DOMAIN.key" ] || genmasterkey

[ -f "$SWANCTL_DIR/x509ca/$SHORT_DOMAIN.crt" ] || genca

PARENT="$SYSCONFDIR"
BASEDIR="${SWANCTL_DIR##$PARENT/}"

for name in "$@"; do
	setparams "$name"
	gendev

	tar -zcf "$name-certs.tar.gz" -C "$PARENT" "$BASEDIR/x509ca/$SHORT_DOMAIN.crt" "$BASEDIR/x509/$name.crt" "$BASEDIR/private/$name.key"
	chmod 600 "$name-certs.tar.gz"
	echo "Generated as $name-certs.tar.gz"
done

exit 0
