# default PKI dir
#export EASYRSA=${EASYRSA:-/etc/easy-rsa}
#export EASYRSA_PKI=${EASYRSA_PKI:-$EASYRSA/pki}
#export EASYRSA_VARS_FILE=${EASYRSA_VARS_FILE:-$EASYRSA/vars}
export EASYRSA_TEMP_DIR=${EASYRSA_TEMP_DIR:-${TMPDIR:-/tmp/}}
