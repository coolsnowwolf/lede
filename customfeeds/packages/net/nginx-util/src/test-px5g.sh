#!/bin/sh

PRINT_PASSED=2

printf "Initializing tests ...\n"

OPENSSL_PEM="$(mktemp)"
OPENSSL_DER="$(mktemp)"

NONCE=$(dd if=/dev/urandom bs=1 count=4 2>/dev/null | hexdump -e '1/1 "%02x"')
SUBJECT="/C=ZZ/ST=Somewhere/L=None/O=OpenWrt'$NONCE'/CN=OpenWrt"

openssl req -x509 -nodes -days 1 -keyout /dev/null 2>/dev/null \
    -out "$OPENSSL_PEM" -subj "$SUBJECT" \
|| ( printf "error: generating PEM certificate with openssl"; return 1)
openssl req -x509 -nodes -days 1 -keyout /dev/null 2>/dev/null \
    -out "$OPENSSL_DER" -outform der -subj "$SUBJECT" \
|| ( printf "error: generating DER certificate with openssl"; return 1)


test() {
    eval "$1 >/dev/null "
    if [ $? -eq "$2" ]
    then
        [ "${PRINT_PASSED}" -gt 0 ] \
        && printf "%-72s%-1s\n" "$1" ">/dev/null (-> $2?) passed."
    else
        printf "%-72s%-1s\n" "$1" ">/dev/null (-> $2?) failed!!!"
        [ "${PRINT_PASSED}" -gt 1 ] && exit 1
    fi
}


[ "$PRINT_PASSED" -gt 0 ] && printf "\nTesting openssl itself ...\n"

[ "$PRINT_PASSED" -gt 1 ] && printf "  * right PEM:\n"
test 'cat "$OPENSSL_PEM" | openssl x509 -checkend 0                         ' 0
test 'cat "$OPENSSL_PEM" | openssl x509 -checkend 86300                     ' 0
test 'cat "$OPENSSL_PEM" | openssl x509 -checkend 86400                     ' 1

[ "$PRINT_PASSED" -gt 1 ] && printf "  * right DER:\n"
test 'cat "$OPENSSL_DER" | openssl x509 -checkend 0    -inform der          ' 0
test 'cat "$OPENSSL_DER" | openssl x509 -checkend 86300 -inform der         ' 0
test 'cat "$OPENSSL_DER" | openssl x509 -checkend 86400 -inform der         ' 1

[ "$PRINT_PASSED" -gt 1 ] && printf "  * wrong:\n"
test 'cat "$OPENSSL_PEM" | openssl x509 -checkend 0 -inform der  2>/dev/null' 1
test 'cat "$OPENSSL_DER" | openssl x509 -checkend 0              2>/dev/null' 1


[ "$PRINT_PASSED" -gt 0 ] && printf "\nTesting px5g checkend ...\n"

[ "$PRINT_PASSED" -gt 1 ] && printf "  * right PEM:\n"
test 'cat "$OPENSSL_PEM" | ./px5g checkend 0                                ' 0
test 'cat "$OPENSSL_PEM" | ./px5g checkend 86300                            ' 0
test 'cat "$OPENSSL_PEM" | ./px5g checkend 86400                            ' 1

[ "$PRINT_PASSED" -gt 1 ] && printf "  * right DER:\n"
test 'cat "$OPENSSL_DER" | ./px5g checkend -der 0                           ' 0
test 'cat "$OPENSSL_DER" | ./px5g checkend -der 86300                       ' 0
test 'cat "$OPENSSL_DER" | ./px5g checkend -der 86400                       ' 1

[ "$PRINT_PASSED" -gt 1 ] && printf "  * in option:\n"
test 'cat "$OPENSSL_DER" | ./px5g checkend -in /proc/self/fd/0 -der 0       ' 0
test 'cat "$OPENSSL_DER" | ./px5g checkend -der -in /proc/self/fd/0 99      ' 0

[ "$PRINT_PASSED" -gt 1 ] && printf "  * wrong:\n"
test 'cat "$OPENSSL_PEM" | ./px5g checkend -der 0                2>/dev/null' 1
test 'cat "$OPENSSL_DER" | ./px5g checkend 0                     2>/dev/null' 1


[ "$PRINT_PASSED" -gt 0 ] && printf "\nTesting px5g eckey ...\n"

[ "$PRINT_PASSED" -gt 1 ] && printf "  * standard curves:\n"
test './px5g eckey P-256        | openssl ec -check              2>/dev/null' 0
test './px5g eckey P-384        | openssl ec -check              2>/dev/null' 0
test './px5g eckey secp384r1    | openssl ec -check              2>/dev/null' 0
test './px5g eckey secp256r1    | openssl ec -check              2>/dev/null' 0
test './px5g eckey secp256k1    | openssl ec -check              2>/dev/null' 0

[ "$PRINT_PASSED" -gt 1 ] && printf "  * more curves:\n"
test './px5g eckey P-521        | openssl ec -check              2>/dev/null' 0
test './px5g eckey secp521r1    | openssl ec -check              2>/dev/null' 0
test './px5g eckey secp224r1    | openssl ec -check              2>/dev/null' 0
test './px5g eckey secp224k1    | openssl ec -check              2>/dev/null' 0
test './px5g eckey secp192r1    | openssl ec -check              2>/dev/null' 0
test './px5g eckey secp192k1    | openssl ec -check              2>/dev/null' 0
test './px5g eckey brainpoolP512r1        | openssl ec -check    2>/dev/null' 0
test './px5g eckey brainpoolP384r1        | openssl ec -check    2>/dev/null' 0
test './px5g eckey brainpoolP256r1        | openssl ec -check    2>/dev/null' 0

[ "$PRINT_PASSED" -gt 1 ] && printf "  * other options:\n"
test './px5g eckey -out /proc/self/fd/1   | openssl ec -check    2>/dev/null' 0
test './px5g eckey -der         | openssl ec -check -inform der  2>/dev/null' 0


[ "$PRINT_PASSED" -gt 0 ] && printf "\nTesting px5g rsakey ...\n"

[ "$PRINT_PASSED" -gt 1 ] && printf "  * standard exponent:\n"
test './px5g rsakey             | openssl rsa -check             2>/dev/null' 0
test './px5g rsakey 512         | openssl rsa -check             2>/dev/null' 0
test './px5g rsakey 1024        | openssl rsa -check             2>/dev/null' 0
test './px5g rsakey 2048        | openssl rsa -check             2>/dev/null' 0
test './px5g rsakey 4096        | openssl rsa -check             2>/dev/null' 0
test './px5g rsakey 1111        | openssl rsa -check             2>/dev/null' 0
test './px5g rsakey 0                                            2>/dev/null' 1

[ "$PRINT_PASSED" -gt 1 ] && printf "  * small exponent:\n"
test './px5g rsakey -3          | openssl rsa -check             2>/dev/null' 0
test './px5g rsakey -3 512      | openssl rsa -check             2>/dev/null' 0
test './px5g rsakey -3 1024     | openssl rsa -check             2>/dev/null' 0
test './px5g rsakey -3 2048     | openssl rsa -check             2>/dev/null' 0
test './px5g rsakey -3 4096     | openssl rsa -check             2>/dev/null' 0
test './px5g rsakey -3 1111     | openssl rsa -check             2>/dev/null' 0
test './px5g rsakey -3 0                                         2>/dev/null' 1

[ "$PRINT_PASSED" -gt 1 ] && printf "  * other options:\n"
test './px5g rsakey -out /proc/self/fd/1  | openssl rsa -check   2>/dev/null' 0
test './px5g rsakey -der        | openssl rsa -check -inform der 2>/dev/null' 0


[ "$PRINT_PASSED" -gt 0 ] && printf "\nTesting px5g selfsigned ...\n"

test './px5g selfsigned -der | openssl x509 -checkend 0 -inform der         ' 0
test './px5g selfsigned -days 1               | openssl x509 -checkend 0    ' 0
test './px5g selfsigned -days 1               | openssl x509 -checkend 86300' 0
test './px5g selfsigned -days 1               | openssl x509 -checkend 86400' 1
test './px5g selfsigned -out /proc/self/fd/1  | openssl x509 -checkend 0    ' 0
test './px5g selfsigned -newkey rsa:666       | openssl x509 -checkend 0    ' 0
test './px5g selfsigned -newkey ec            | openssl x509 -checkend 0    ' 0
test './px5g selfsigned -newkey ec -pkeyopt ec_paramgen_curve:secp384r1 \
      | openssl x509 -checkend 0                                        ' 0
test './px5g selfsigned -subj "$SUBJECT" | openssl x509 -noout \
      -subject -nameopt compat | grep -q subject="$SUBJECT" 2>/dev/null' 0
test './px5g selfsigned -out /dev/null -keyout /proc/self/fd/1 \
      | openssl rsa -check 2>/dev/null                                  ' 0


rm "$OPENSSL_PEM" "$OPENSSL_DER"
