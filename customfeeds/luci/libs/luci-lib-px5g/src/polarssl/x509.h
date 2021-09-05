/**
 * \file x509.h
 *
 *  Based on XySSL: Copyright (C) 2006-2008  Christophe Devine
 *
 *  Copyright (C) 2009  Paul Bakker <polarssl_maintainer at polarssl dot org>
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the names of PolarSSL or XySSL nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef POLARSSL_X509_H
#define POLARSSL_X509_H

#include "polarssl/rsa.h"

#define POLARSSL_ERR_ASN1_OUT_OF_DATA                      -0x0014
#define POLARSSL_ERR_ASN1_UNEXPECTED_TAG                   -0x0016
#define POLARSSL_ERR_ASN1_INVALID_LENGTH                   -0x0018
#define POLARSSL_ERR_ASN1_LENGTH_MISMATCH                  -0x001A
#define POLARSSL_ERR_ASN1_INVALID_DATA                     -0x001C

#define POLARSSL_ERR_X509_FEATURE_UNAVAILABLE              -0x0020
#define POLARSSL_ERR_X509_CERT_INVALID_PEM                 -0x0040
#define POLARSSL_ERR_X509_CERT_INVALID_FORMAT              -0x0060
#define POLARSSL_ERR_X509_CERT_INVALID_VERSION             -0x0080
#define POLARSSL_ERR_X509_CERT_INVALID_SERIAL              -0x00A0
#define POLARSSL_ERR_X509_CERT_INVALID_ALG                 -0x00C0
#define POLARSSL_ERR_X509_CERT_INVALID_NAME                -0x00E0
#define POLARSSL_ERR_X509_CERT_INVALID_DATE                -0x0100
#define POLARSSL_ERR_X509_CERT_INVALID_PUBKEY              -0x0120
#define POLARSSL_ERR_X509_CERT_INVALID_SIGNATURE           -0x0140
#define POLARSSL_ERR_X509_CERT_INVALID_EXTENSIONS          -0x0160
#define POLARSSL_ERR_X509_CERT_UNKNOWN_VERSION             -0x0180
#define POLARSSL_ERR_X509_CERT_UNKNOWN_SIG_ALG             -0x01A0
#define POLARSSL_ERR_X509_CERT_UNKNOWN_PK_ALG              -0x01C0
#define POLARSSL_ERR_X509_CERT_SIG_MISMATCH                -0x01E0
#define POLARSSL_ERR_X509_CERT_VERIFY_FAILED               -0x0200
#define POLARSSL_ERR_X509_KEY_INVALID_PEM                  -0x0220
#define POLARSSL_ERR_X509_KEY_INVALID_VERSION              -0x0240
#define POLARSSL_ERR_X509_KEY_INVALID_FORMAT               -0x0260
#define POLARSSL_ERR_X509_KEY_INVALID_ENC_IV               -0x0280
#define POLARSSL_ERR_X509_KEY_UNKNOWN_ENC_ALG              -0x02A0
#define POLARSSL_ERR_X509_KEY_PASSWORD_REQUIRED            -0x02C0
#define POLARSSL_ERR_X509_KEY_PASSWORD_MISMATCH            -0x02E0
#define POLARSSL_ERR_X509_POINT_ERROR                      -0x0300
#define POLARSSL_ERR_X509_VALUE_TO_LENGTH                  -0x0320

#define BADCERT_EXPIRED                 1
#define BADCERT_REVOKED                 2
#define BADCERT_CN_MISMATCH             4
#define BADCERT_NOT_TRUSTED             8

/*
 * DER constants
 */
#define ASN1_BOOLEAN                 0x01
#define ASN1_INTEGER                 0x02
#define ASN1_BIT_STRING              0x03
#define ASN1_OCTET_STRING            0x04
#define ASN1_NULL                    0x05
#define ASN1_OID                     0x06
#define ASN1_UTF8_STRING             0x0C
#define ASN1_SEQUENCE                0x10
#define ASN1_SET                     0x11
#define ASN1_PRINTABLE_STRING        0x13
#define ASN1_T61_STRING              0x14
#define ASN1_IA5_STRING              0x16
#define ASN1_UTC_TIME                0x17
#define ASN1_UNIVERSAL_STRING        0x1C
#define ASN1_BMP_STRING              0x1E
#define ASN1_PRIMITIVE               0x00
#define ASN1_CONSTRUCTED             0x20
#define ASN1_CONTEXT_SPECIFIC        0x80

/*
 * various object identifiers
 */
#define X520_COMMON_NAME                3
#define X520_COUNTRY                    6
#define X520_LOCALITY                   7
#define X520_STATE                      8
#define X520_ORGANIZATION              10
#define X520_ORG_UNIT                  11
#define PKCS9_EMAIL                     1

#define X509_OUTPUT_DER              0x01
#define X509_OUTPUT_PEM              0x02
#define PEM_LINE_LENGTH                72
#define X509_ISSUER                  0x01
#define X509_SUBJECT                 0x02

#define OID_X520                "\x55\x04"
#define OID_CN                  "\x55\x04\x03"
#define OID_PKCS1               "\x2A\x86\x48\x86\xF7\x0D\x01\x01"
#define OID_PKCS1_RSA           "\x2A\x86\x48\x86\xF7\x0D\x01\x01\x01"
#define OID_PKCS1_RSA_SHA       "\x2A\x86\x48\x86\xF7\x0D\x01\x01\x05"
#define OID_PKCS9               "\x2A\x86\x48\x86\xF7\x0D\x01\x09"
#define OID_PKCS9_EMAIL         "\x2A\x86\x48\x86\xF7\x0D\x01\x09\x01"

/*
 * Structures for parsing X.509 certificates
 */
typedef struct _x509_buf
{
    int tag;
    int len;
    unsigned char *p;
}
x509_buf;

typedef struct _x509_name
{
    x509_buf oid;
    x509_buf val;
    struct _x509_name *next;
}
x509_name;

typedef struct _x509_time
{
    int year, mon, day;
    int hour, min, sec;
}
x509_time;

typedef struct _x509_cert
{
    x509_buf raw;
    x509_buf tbs;

    int version;
    x509_buf serial;
    x509_buf sig_oid1;

    x509_buf issuer_raw;
    x509_buf subject_raw;

    x509_name issuer;
    x509_name subject;

    x509_time valid_from;
    x509_time valid_to;

    x509_buf pk_oid;
    rsa_context rsa;

    x509_buf issuer_id;
    x509_buf subject_id;
    x509_buf v3_ext;

    int ca_istrue;
    int max_pathlen;

    x509_buf sig_oid2;
    x509_buf sig;

    struct _x509_cert *next; 
}
x509_cert;

/*
 * Structures for writing X.509 certificates
 */
typedef struct _x509_node
{
    unsigned char *data;
    unsigned char *p;
    unsigned char *end;

    size_t len;
}
x509_node;

typedef struct _x509_raw
{
    x509_node raw;
    x509_node tbs;

    x509_node version;
    x509_node serial;
    x509_node tbs_signalg;
    x509_node issuer;
    x509_node validity;
    x509_node subject;
    x509_node subpubkey;

    x509_node signalg;
    x509_node sign;
}
x509_raw;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Parse one or more certificates and add them
 *                 to the chained list
 *
 * \param chain    points to the start of the chain
 * \param buf      buffer holding the certificate data
 * \param buflen   size of the buffer
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509parse_crt( x509_cert *crt, unsigned char *buf, int buflen );

/**
 * \brief          Load one or more certificates and add them
 *                 to the chained list
 *
 * \param chain    points to the start of the chain
 * \param path     filename to read the certificates from
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509parse_crtfile( x509_cert *crt, char *path );

/**
 * \brief          Parse a private RSA key
 *
 * \param rsa      RSA context to be initialized
 * \param buf      input buffer
 * \param buflen   size of the buffer
 * \param pwd      password for decryption (optional)
 * \param pwdlen   size of the password
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509parse_key( rsa_context *rsa,
                   unsigned char *buf, int buflen,
                   unsigned char *pwd, int pwdlen );

/**
 * \brief          Load and parse a private RSA key
 *
 * \param rsa      RSA context to be initialized
 * \param path     filename to read the private key from
 * \param pwd      password to decrypt the file (can be NULL)
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509parse_keyfile( rsa_context *rsa, char *path, char *password );

/**
 * \brief          Store the certificate DN in printable form into buf;
 *                 no more than (end - buf) characters will be written.
 */
int x509parse_dn_gets( char *buf, char *end, x509_name *dn );

/**
 * \brief          Returns an informational string about the
 *                 certificate.
 */
char *x509parse_cert_info( char *prefix, x509_cert *crt );

/**
 * \brief          Return 0 if the certificate is still valid,
 *                 or BADCERT_EXPIRED
 */
int x509parse_expired( x509_cert *crt );

/**
 * \brief          Verify the certificate signature
 *
 * \param crt      a certificate to be verified
 * \param trust_ca the trusted CA chain
 * \param cn       expected Common Name (can be set to
 *                 NULL if the CN must not be verified)
 * \param flags    result of the verification
 *
 * \return         0 if successful or POLARSSL_ERR_X509_SIG_VERIFY_FAILED,
 *                 in which case *flags will have one or more of
 *                 the following values set:
 *                      BADCERT_EXPIRED --
 *                      BADCERT_REVOKED --
 *                      BADCERT_CN_MISMATCH --
 *                      BADCERT_NOT_TRUSTED
 *
 * \note           TODO: add two arguments, depth and crl
 */
int x509parse_verify( x509_cert *crt,
                      x509_cert *trust_ca,
                      char *cn, int *flags );

/**
 * \brief          Unallocate all certificate data
 */
void x509_free( x509_cert *crt );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int x509_self_test( int verbose );

/**
 * \brief          Write a certificate info file
 *
 * \param chain    points to the raw certificate data
 * \param path     filename to write the certificate to
 * \param format   X509_OUTPUT_DER or X509_OUTPUT_PEM
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509write_crtfile( x509_raw *chain,
                       unsigned char *path,
                       int format );

/**
 * \brief          Write a certificate signing request message format file
 *
 * \param chain    points to the raw certificate (with x509write_create_csr) data
 * \param path     filename to write the certificate to
 * \param format   X509_OUTPUT_DER or X509_OUTPUT_PEM
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509write_csrfile( x509_raw *chain,
                       unsigned char *path,
                       int format );

/*
 * \brief          Write a private RSA key into a file
 *
 * \param rsa      points to an RSA key
 * \param path     filename to write the key to
 * \param format   X509_OUTPUT_DER or X509_OUTPUT_PEM
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509write_keyfile( rsa_context *rsa,
                       char *path,
                       int format );

/**
 * \brief          Add a public key to certificate
 *
 * \param chain    points to the raw certificate data
 * \param pubkey   points to an RSA key
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509write_add_pubkey( x509_raw *chain, rsa_context *pubkey );

/**
 * \brief          Create x509 subject/issuer field to raw certificate
 *                 from string or CA cert. Make string NULL if you will
 *                 use the CA copy function or make CA NULL then used
 *                 the string parse.
 *
 * \param chain    points to the raw certificate data
 * \param names    a string that can hold (separete with ";"):
 *                     CN=CommonName
 *                 --   O=Organization
 *                 --  OU=OrgUnit
 *                 --  ST=State
 *                 --   L=Locality
 *                 --   R=Email
 *                 --   C=Country
 *                 . Make that NULL if you didn't need that.
 * \param flag     flag is X509_ISSUER or X509_SUBJECT that defined
 *                 where change
 * \param ca       the certificate for copy data. Make that NULL if you
 *                 didn't need that.
 * \param ca_flag  set the ca field from copy to crt
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509write_add_customize ( x509_raw *crt, 
                          unsigned char *names, 
                          int flag, 
                          x509_cert *ca, 
                          int ca_flag );

/**
* \brief          Add x509 issuer field
*
* \param chain    points to the raw certificate data
* \param issuer   a string holding (separete with ";"):
*                     CN=CommonName
*                 --   O=Organization
*                 --  OU=OrgUnit
*                 --  ST=State
*                 --   L=Locality
*                 --   R=Email
*                 --   C=Country
*                 . Set this to NULL if not needed.
* \return         0 if successful, or a specific X509 error code
*/
int x509write_add_issuer( x509_raw *crt, unsigned char *issuer);

/**
 * \brief          Add x509 subject field
 *
 * \param chain    points to the raw certificate data
 * \param subject  a string holding (separete with ";"):
 *                     CN=CommonName
 *                 --   O=Organization
 *                 --  OU=OrgUnit
 *                 --  ST=State
 *                 --   L=Locality
 *                 --   R=Email
 *                 --   C=Country
 *                 . Set this to NULL if not needed.
 * \return         0 if successful, or a specific X509 error code
 */
int x509write_add_subject( x509_raw *crt, unsigned char *subject);

/**
* \brief          Copy x509 issuer field from another certificate
*
* \param chain    points to the raw certificate data
* \param from_crt the certificate whose issuer is to be copied.
* \return         0 if successful, or a specific X509 error code
*/
int x509write_copy_issuer(x509_raw *crt, x509_cert *from_crt);

/**
* \brief          Copy x509 subject field from another certificate
*
* \param chain    points to the raw certificate data
* \param from_crt the certificate whose subject is to be copied.
* \return         0 if successful, or a specific X509 error code
*/
int x509write_copy_subject(x509_raw *crt, x509_cert *from_crt);

/**
* \brief          Copy x509 issuer field from the subject of another certificate
*
* \param chain    points to the raw certificate data
* \param from_crt the certificate whose subject is to be copied.
* \return         0 if successful, or a specific X509 error code
*/
int x509write_copy_issuer_from_subject(x509_raw *crt, x509_cert *from_crt);

/**
* \brief          Copy x509 subject field from the issuer of another certificate
*
* \param chain    points to the raw certificate data
* \param from_crt the certificate whose issuer is to be copied.
* \return         0 if successful, or a specific X509 error code
*/
int x509write_copy_subject_from_issuer(x509_raw *crt, x509_cert *from_crt);

/**
 * \brief          Create x509 validity time in UTC
 *
 * \param chain    points to the raw certificate data
 * \param before   valid not before in format YYYY-MM-DD hh:mm:ss
 * \param after    valid not after  in format YYYY-MM-DD hh:mm:ss
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509write_add_validity( x509_raw *crt,
                               unsigned char *before,
                               unsigned char *after );

/**
 * \brief          Create a self-signed certificate
 *
 * \param chain    points to the raw certificate data
 * \param rsa      a private key to sign the certificate
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509write_create_selfsign( x509_raw *crt, rsa_context *raw );

/**
 * \brief          Create a certificate
 *
 * \param chain    points to the raw certificate data
 * \param rsa      a private key to sign the certificate
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509write_create_sign( x509_raw *crt, rsa_context *raw );

/**
 * \brief          Create a certificate signing request
 *
 * \param chain    points to the raw certificate data. Didn't use the
 *                 same chain that u have use for certificate.
 * \param privkey  a rsa private key
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509write_create_csr( x509_raw *chain, rsa_context *privkey );

/**
 * \brief           Serialize an rsa key into DER
 *
 * \param rsa       a rsa key for output
 * \param node      a x509 node for write into
 *
 * \return          0 if successful, or a specific X509 error code
 */
int x509write_serialize_key( rsa_context *rsa, x509_node *node );

/**
 * \brief          Unallocate all raw certificate data
 */
void x509write_free_raw( x509_raw *crt );

/**
 * \brief          Allocate all raw certificate data
 */
void x509write_init_raw( x509_raw *crt );

/**
 * \brief          Unallocate all node certificate data
 */
void x509write_free_node( x509_node *crt_node );

/**
 * \brief          Allocate all node certificate data
 */
void x509write_init_node( x509_node *crt_node );

#ifdef __cplusplus
}
#endif

#endif /* x509.h */
