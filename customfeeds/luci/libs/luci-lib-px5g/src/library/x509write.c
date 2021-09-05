/*
 *  X.509 certificate and private key writing
 *
 *  Copyright (C) 2006-2007  Pascal Vizeli <pvizeli@yahoo.de>
 *  Modifications (C) 2009 Steven Barth <steven@midlink.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License, version 2.1 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */
/*
 *  The ITU-T X.509 standard defines a certificat format for PKI.
 *
 *  http://www.ietf.org/rfc/rfc2459.txt
 *  http://www.ietf.org/rfc/rfc3279.txt
 *
 *  ftp://ftp.rsasecurity.com/pub/pkcs/ascii/pkcs-1v2.asc
 *
 *  http://www.itu.int/ITU-T/studygroups/com17/languages/X.680-0207.pdf
 *  http://www.itu.int/ITU-T/studygroups/com17/languages/X.690-0207.pdf
 *
 *  For CRS:
 *  http://www.faqs.org/rfcs/rfc2314.html
 */
#include "polarssl/config.h"
#include "polarssl/x509.h"
/* #include "polarssl/base64.h" */
#include "polarssl/sha1.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define and &&
#define or ||

#if defined _MSC_VER && !defined snprintf
#define snprintf _snprintf
#endif

static int x509write_realloc_node(x509_node *node, size_t larger);
static int x509write_file(x509_node *node, char *path, int format, const char* pem_prolog, const char* pem_epilog);

/*
 * evaluate how mani octet have this integer
 */
static int asn1_eval_octet(unsigned int digit)
{
    int i, byte;

    for (byte = 4, i = 24; i >= 0; i -= 8, --byte)
        if (((digit >> i) & 0xFF) != 0)
            return byte;

    return 0;
}

/*
 * write the asn.1 lenght form into p
 */
static int asn1_add_len(unsigned int size, x509_node *node)
{
    if (size > 127) {

        /* long size */
        int byte = asn1_eval_octet(size);
        int i = 0;

        *(node->p) = (0x80 | byte) & 0xFF;
        ++node->p;

        for (i = byte; i > 0; --i) {

            *(node->p) = (size >> ((i - 1) * 8)) & 0xFF;
            ++node->p;
        }

    } else {

        /* short size */
        *(node->p) = size & 0xFF;
        if (size != 0)
            ++node->p;
    }

    return 0;
}

/*
 * write a ans.1 object into p
 */
static int asn1_add_obj(unsigned char *value, unsigned int size, int tag,
        x509_node *node)
{
    int tl = 2;

    if (tag == ASN1_BIT_STRING)
        ++tl;

    if (size > 127)
        x509write_realloc_node(node, (size_t) size + tl +
                asn1_eval_octet(size));
    else
        x509write_realloc_node(node, (size_t) size + tl);

    if (node->data == NULL)
        return 1;

    /* tag */
    *(node->p) = tag & 0xFF;
    ++node->p;

    /* len */
    if (tag == ASN1_BIT_STRING) {
        asn1_add_len((unsigned int) size + 1, node);
        *(node->p) = 0x00;
        ++node->p;
    } else {
        asn1_add_len((unsigned int) size, node);
    }

    /* value */
    if (size > 0) {

        memcpy(node->p, value, (size_t) size);
        if ((node->p += size -1) != node->end)
            return POLARSSL_ERR_X509_POINT_ERROR;
    } else {
        /* make nothing -> NULL */
    }

    return 0;
}

/*
 * write a asn.1 conform integer object
 */
static int asn1_add_int(signed int value, x509_node *node)
{
    signed int i = 0, neg = 1;
    unsigned int byte, u_val = 0, tmp_val = 0;

    /* if negate? */
    if (value < 0) {
        neg = -1;
        u_val = ~value;
    } else {
        u_val = value;
    }

    byte = asn1_eval_octet(u_val);
    /* 0 isn't NULL */
    if (byte == 0)
        byte = 1;

    /* ASN.1 integer is signed! */
    if (byte < 4 and ((u_val >> ((byte -1) * 8)) & 0xFF) == 0x80)
        byte += 1;

    if (x509write_realloc_node(node, (size_t) byte + 2) != 0)
        return 1;

    /* tag */
    *(node->p) = ASN1_INTEGER;
    ++node->p;

    /* len */
    asn1_add_len(byte, node);

    /* value */
    for (i = byte; i > 0; --i) {

        tmp_val = (u_val >> ((i - 1) * 8)) & 0xFF;
        if (neg == 1)
            *(node->p) = tmp_val;
        else
            *(node->p) = ~tmp_val;

        if (i > 1)
          ++node->p;
    }

    if (node->p != node->end)
        return POLARSSL_ERR_X509_POINT_ERROR;

    return 0;
}

/*
 * write a asn.1 conform mpi object
 */
static int asn1_add_mpi(mpi *value, int tag, x509_node *node)
{
    size_t size = (mpi_msb(value) / 8) + 1;
    unsigned char *buf;
    int buf_len = (int) size, tl = 2;

    if (tag == ASN1_BIT_STRING)
        ++tl;

    if (size > 127)
        x509write_realloc_node(node, size + (size_t) tl +
            asn1_eval_octet((unsigned int)size));
    else
        x509write_realloc_node(node, size + (size_t) tl);

    if (node->data == NULL)
        return 1;

    buf = (unsigned char*) malloc(size);
    if (mpi_write_binary(value, buf, buf_len) != 0)
        return POLARSSL_ERR_MPI_BUFFER_TOO_SMALL;

    /* tag */
    *(node->p) = tag & 0xFF;
    ++node->p;

    /* len */
    if (tag == ASN1_BIT_STRING) {
        asn1_add_len((unsigned int) size + 1, node);
        *(node->p) = 0x00;
        ++node->p;
    } else {
        asn1_add_len((unsigned int) size, node);
    }

    /* value */
    memcpy(node->p, buf, size);
    free(buf);

    if ((node->p += (int) size -1) != node->end)
        return POLARSSL_ERR_X509_POINT_ERROR;

    return 0;
}

/*
 * write a node into asn.1 conform object
 */
static int asn1_append_tag(x509_node *node, int tag)
{
    int tl = 2;

    x509_node tmp;
    x509write_init_node(&tmp);

    if (tag == ASN1_BIT_STRING)
        ++tl;

    if (node->len > 127)
        x509write_realloc_node(&tmp, node->len + (size_t) tl +
            asn1_eval_octet((unsigned int)node->len));
    else
        x509write_realloc_node(&tmp, node->len + (size_t) tl);

    if (tmp.data == NULL) {
        x509write_free_node(&tmp);
        return 1;
    }

    /* tag */
    *(tmp.p) = tag & 0xFF;
    ++tmp.p;

    /* len */
    if (tag == ASN1_BIT_STRING) {
        asn1_add_len((unsigned int) node->len + 1, &tmp);
        *(tmp.p) = 0x00;
        ++tmp.p;
    } else {
        asn1_add_len((unsigned int) node->len, &tmp);
    }

    /* value */
    memcpy(tmp.p, node->data, node->len);

    /* good? */
    if ((tmp.p += (int) node->len -1) != tmp.end) {
        x509write_free_node(&tmp);
        return POLARSSL_ERR_X509_POINT_ERROR;
    }

    free(node->data);
    node->data = tmp.data;
    node->p = tmp.p;
    node->end = tmp.end;
    node->len = tmp.len;

    return 0;
}

/*
 * write nodes into a asn.1 object
 */
static int asn1_append_nodes(x509_node *node, int tag, int anz, ...)
{
    va_list ap;
    size_t size = 0;
    x509_node *tmp;
    int count;

    va_start(ap, anz);
    count = anz;

    while (count--) {

        tmp = va_arg(ap, x509_node*);
        if (tmp->data != NULL)
            size += tmp->len;
    }

    if ( size > 127) {
        if (x509write_realloc_node(node, size + (size_t) 2 +
                    asn1_eval_octet(size)) != 0)
            return 1;
    } else {
        if (x509write_realloc_node(node, size + (size_t) 2) != 0)
            return 1;
    }

    /* tag */
    *(node->p) = tag & 0xFF;
    ++node->p;

    /* len */
    asn1_add_len(size, node);

    /* value */
    va_start(ap, anz);
    count = anz;

    while (count--) {

        tmp = va_arg(ap, x509_node*);
        if (tmp->data != NULL) {

            memcpy(node->p, tmp->data, tmp->len);
            if ((node->p += (int) tmp->len -1) != node->end)
                ++node->p;
        }
    }

    va_end(ap);
    return 0;
}

/*
 * write a ASN.1 conform object identifiere include a "tag"
 */
static int asn1_add_oid(x509_node *node, unsigned char *oid, size_t len,
        int tag, int tag_val, unsigned char *value, size_t val_len)
{
    int ret;
    x509_node tmp;

    x509write_init_node(&tmp);

    /* OBJECT IDENTIFIER */
    if ((ret = asn1_add_obj(oid, len, ASN1_OID, &tmp)) != 0) {
        x509write_free_node(&tmp);
        return ret;
    }

    /* value */
    if ((ret = asn1_add_obj(value, val_len, tag_val, &tmp)) != 0) {
        x509write_free_node(&tmp);
        return ret;
    }

    /* SET/SEQUENCE */
    if ((ret = asn1_append_nodes(node, tag, 1, &tmp)) != 0) {
        x509write_free_node(&tmp);
        return ret;
    }

    x509write_free_node(&tmp);
    return 0;
}

/*
 *  utcTime        UTCTime
 */
static int asn1_add_date_utc(unsigned char *time, x509_node *node)
{
    unsigned char date[13], *sp;
    x509_time xtime;
    int ret;

    sscanf((char*)time, "%d-%d-%d %d:%d:%d", &xtime.year, &xtime.mon,
    	&xtime.day, &xtime.hour, &xtime.min, &xtime.sec);

    /* convert to YY */
    if (xtime.year > 2000)
        xtime.year -= 2000;
    else
        xtime.year -= 1900;

    snprintf((char*)date, 13, "%2d%2d%2d%2d%2d%2d", xtime.year, xtime.mon, xtime.day,
        xtime.hour, xtime.min, xtime.sec);

    /* replace ' ' to '0' */
    for (sp = date; *sp != '\0'; ++sp)
        if (*sp == '\x20')
            *sp = '\x30';

    date[12] = 'Z';

    if ((ret = asn1_add_obj(date, 13, ASN1_UTC_TIME, node)) != 0)
        return ret;

    return 0;
}

/*
 * serialize an rsa key into DER
 */

int x509write_serialize_key(rsa_context *rsa, x509_node *node)
{
    int ret = 0;
    x509write_init_node(node);

    /* vers, n, e, d, p, q, dp, dq, pq */
    if ((ret = asn1_add_int(rsa->ver, node)) != 0)
        return ret;
    if ((ret = asn1_add_mpi(&rsa->N, ASN1_INTEGER, node)) != 0)
        return ret;
    if ((ret = asn1_add_mpi(&rsa->E, ASN1_INTEGER, node)) != 0)
        return ret;
    if ((ret = asn1_add_mpi(&rsa->D, ASN1_INTEGER, node)) != 0)
        return ret;
    if ((ret = asn1_add_mpi(&rsa->P, ASN1_INTEGER, node)) != 0)
        return ret;
    if ((ret = asn1_add_mpi(&rsa->Q, ASN1_INTEGER, node)) != 0)
        return ret;
    if ((ret = asn1_add_mpi(&rsa->DP, ASN1_INTEGER, node)) != 0)
        return ret;
    if ((ret = asn1_add_mpi(&rsa->DQ, ASN1_INTEGER, node)) != 0)
        return ret;
    if ((ret = asn1_add_mpi(&rsa->QP, ASN1_INTEGER, node)) != 0)
        return ret;
    if ((ret = asn1_append_tag(node, ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
        return ret;

    return 0;
}

/*
 * write a der/pem encoded rsa private key into a file
 */
int x509write_keyfile(rsa_context *rsa, char *path, int out_flag)
{
    int ret = 0;
    const char	key_beg[] = "-----BEGIN RSA PRIVATE KEY-----\n",
                key_end[] = "-----END RSA PRIVATE KEY-----\n";
    x509_node node;

    x509write_init_node(&node);
    if ((ret = x509write_serialize_key(rsa,&node)) != 0) {
        x509write_free_node(&node);
	      return ret;
    }

    ret = x509write_file(&node,path,out_flag,key_beg,key_end);
    x509write_free_node(&node);

    return ret;
}


/*
 * reasize the memory for node
 */
static int x509write_realloc_node(x509_node *node, size_t larger)
{
    /* init len */
    if (node->data == NULL) {
        node->len = 0;
        node->data = malloc(larger);
        if(node->data == NULL)
            return 1;
    } else {
        /* realloc memory */
        if ((node->data = realloc(node->data, node->len + larger)) == NULL)
            return 1;
    }

    /* init pointer */
    node->p = &node->data[node->len];
    node->len += larger;
    node->end = &node->data[node->len -1];

    return 0;
}

/*
 * init node
 */
void x509write_init_node(x509_node *node)
{
    memset(node, 0, sizeof(x509_node));
}

/*
 * clean memory
 */
void x509write_free_node(x509_node *node)
{
    if (node->data != NULL)
        free(node->data);
    node->p = NULL;
    node->end = NULL;
    node->len = 0;
}

/*
 * write a x509 certificate into file
 */
int x509write_crtfile(x509_raw *chain, unsigned char *path, int out_flag)
{
    const char	cer_beg[] = "-----BEGIN CERTIFICATE-----\n",
		cer_end[] = "-----END CERTIFICATE-----\n";

    return x509write_file(&chain->raw, (char*)path, out_flag, cer_beg, cer_end);
}

/*
 * write a x509 certificate into file
 */
int x509write_csrfile(x509_raw *chain, unsigned char *path, int out_flag)
{
    const char	cer_beg[] = "-----BEGIN CERTIFICATE REQUEST-----\n",
		cer_end[] = "-----END CERTIFICATE REQUEST-----\n";

    return x509write_file(&chain->raw, (char*)path, out_flag, cer_beg, cer_end);
}

/*
 * write an x509 file
 */
static int x509write_file(x509_node *node, char *path, int format,
        const char* pem_prolog, const char* pem_epilog)
{
    FILE *ofstream;
    int is_err = 1/*, buf_len, i, n*/;
    /* char* base_buf; */

    if ((ofstream = fopen(path, "wb")) == NULL)
        return 1;

    switch (format) {
        case X509_OUTPUT_DER:
            if (fwrite(node->data, 1, node->len, ofstream)
                != node->len)
                is_err = -1;
            break;
/*
        case X509_OUTPUT_PEM:
            if (fprintf(ofstream,pem_prolog)<0) {
                is_err = -1;
                break;
            }

            buf_len = node->len << 1;
            base_buf = (char*) malloc((size_t)buf_len);
            memset(base_buf,0,buf_len);
            if (base64_encode(base_buf, &buf_len, node->data,
                        (int) node->len) != 0) {
                is_err = -1;
                break;
            }

            n=strlen(base_buf);
            for(i=0;i<n;i+=64) {
                fprintf(ofstream,"%.64s\n",&base_buf[i]);
            }

            if (fprintf(ofstream, pem_epilog)<0) {
                is_err = -1;
                break;
            }

            free(base_buf); */
    }

    fclose(ofstream);

    if (is_err == -1)
        return 1;

    return 0;
}


/*
 * add the owner public key to x509 certificate
 */
int x509write_add_pubkey(x509_raw *chain, rsa_context *pubkey)
{
    x509_node n_tmp, n_tmp2, *node;
    int ret;

    node = &chain->subpubkey;

    x509write_init_node(&n_tmp);
    x509write_init_node(&n_tmp2);

    /*
    *  RSAPublicKey ::= SEQUENCE {
    *      modulus           INTEGER,  -- n
    *      publicExponent    INTEGER   -- e
    *  }
    */
    if ((ret = asn1_add_mpi(&pubkey->N, ASN1_INTEGER, &n_tmp)) != 0) {
        x509write_free_node(&n_tmp);
        x509write_free_node(&n_tmp2);
        return ret;
    }
    if ((ret = asn1_add_mpi(&pubkey->E, ASN1_INTEGER, &n_tmp)) != 0) {
        x509write_free_node(&n_tmp);
        x509write_free_node(&n_tmp2);
        return ret;
    }
    if ((ret = asn1_append_tag(&n_tmp, ASN1_CONSTRUCTED | ASN1_SEQUENCE))
            != 0) {
        x509write_free_node(&n_tmp);
        x509write_free_node(&n_tmp2);
        return ret;
    }

    /*
     *  SubjectPublicKeyInfo  ::=  SEQUENCE  {
     *       algorithm            AlgorithmIdentifier,
     *       subjectPublicKey     BIT STRING }
     */
    if ((ret = asn1_append_tag(&n_tmp, ASN1_BIT_STRING)) != 0) {
        x509write_free_node(&n_tmp);
        x509write_free_node(&n_tmp2);
       return ret;
    }
    if ((ret = asn1_add_oid(&n_tmp2, (unsigned char*)OID_PKCS1_RSA, 9,
                  ASN1_CONSTRUCTED | ASN1_SEQUENCE, ASN1_NULL,
                  (unsigned char *)"", 0)) != 0) {
        x509write_free_node(&n_tmp);
        x509write_free_node(&n_tmp2);
        return ret;
    }

    if ((ret = asn1_append_nodes(node, ASN1_CONSTRUCTED | ASN1_SEQUENCE, 2,
                   &n_tmp2, &n_tmp))) {
        x509write_free_node(&n_tmp);
        x509write_free_node(&n_tmp2);
        return ret;
    }

    x509write_free_node(&n_tmp);
    x509write_free_node(&n_tmp2);
    return 0;
}

/*
 *  RelativeDistinguishedName ::=
 *    SET OF AttributeTypeAndValue
 *
 *  AttributeTypeAndValue ::= SEQUENCE {
 *    type     AttributeType,
 *    value    AttributeValue }
 */
static int x509write_add_name(x509_node *node, unsigned char *oid,
        unsigned int oid_len, unsigned char *value, int len, int value_tag)
{
    int ret;
    x509_node n_tmp;

    x509write_init_node(&n_tmp);

    if ((ret = asn1_add_oid(&n_tmp, oid, oid_len,
                ASN1_CONSTRUCTED | ASN1_SEQUENCE, value_tag,
                value, len))) {
        x509write_free_node(&n_tmp);
        return ret;
    }

    if ((asn1_append_nodes(node, ASN1_CONSTRUCTED | ASN1_SET, 1, &n_tmp))
            != 0) {
        x509write_free_node(&n_tmp);
        return ret;
    }

    x509write_free_node(&n_tmp);
    return 0;
}

/*
 * Parse the name string and add to node
 */
static int x509write_parse_names(x509_node *node, unsigned char *names)
{
    unsigned char *sp, *begin = NULL;
    unsigned char oid[3] = OID_X520, tag[4], *tag_sp = tag;
    unsigned char *C = NULL, *CN = NULL, *O = NULL, *OU = NULL,
                  *ST = NULL, *L = NULL, *R = NULL;
    int C_len = 0, CN_len = 0, O_len = 0, OU_len = 0, ST_len = 0,
		L_len = 0, R_len = 0;
    int ret = 0, is_tag = 1, is_begin = -1, len = 0;


    for (sp = names; ; ++sp) {

        /* filter tag */
        if (is_tag == 1) {

            if (tag_sp == &tag[3])
                return POLARSSL_ERR_X509_VALUE_TO_LENGTH;

            /* is tag end? */
            if (*sp == '=') {
                is_tag = -1;
                *tag_sp = '\0';
                is_begin = 1;
                /* set len 0 (reset) */
                len = 0;
            } else {
                /* tag hasn't ' '! */
                if (*sp != ' ') {
                    *tag_sp = *sp;
                    ++tag_sp;
                }
            }
        /* filter value */
        } else {

            /* set pointer of value begin */
            if (is_begin == 1) {
                begin = sp;
                is_begin = -1;
            }

            /* is value at end? */
            if (*sp == ';' or *sp == '\0') {
                is_tag = 1;

                /* common name */
                if (tag[0] == 'C' and tag[1] == 'N') {
                    CN = begin;
                    CN_len = len;

                /* organization */
                } else if (tag[0] == 'O' and tag[1] == '\0') {
                    O = begin;
                    O_len = len;

                /* country */
                } else if (tag[0] == 'C' and tag[1] == '\0') {
                    C = begin;
                    C_len = len;

                /* organisation unit */
                } else if (tag[0] == 'O' and tag[1] == 'U') {
                    OU = begin;
                    OU_len = len;

                /* state */
                } else if (tag[0] == 'S' and tag[1] == 'T') {
                    ST = begin;
                    ST_len = len;

                /* locality */
                } else if (tag[0] == 'L' and tag[1] == '\0') {
                    L = begin;
                    L_len = len;

                /* email */
                } else if (tag[0] == 'R' and tag[1] == '\0') {
                    R = begin;
                    R_len = len;
                }

                /* set tag poiner to begin */
                tag_sp = tag;

                /* is at end? */
                if (*sp == '\0' or *(sp +1) == '\0')
                    break;
            } else {
                ++len;
            }
        }

        /* make saver */
        if (*sp == '\0')
          break;
    } /* end for */

    /* country */
    if (C != NULL) {
        oid[2] = X520_COUNTRY;
        if ((ret = x509write_add_name(node, oid, 3, C, C_len,
                        ASN1_PRINTABLE_STRING)) != 0)
            return ret;
    }

    /* state */
    if (ST != NULL) {
        oid[2] = X520_STATE;
        if ((ret = x509write_add_name(node, oid, 3, ST, ST_len,
                        ASN1_PRINTABLE_STRING)) != 0)
            return ret;
    }

    /* locality */
    if (L != NULL) {
        oid[2] = X520_LOCALITY;
        if ((ret = x509write_add_name(node, oid, 3, L, L_len,
                        ASN1_PRINTABLE_STRING)) != 0)
            return ret;
    }

    /* organization */
    if (O != NULL) {
        oid[2] = X520_ORGANIZATION;
        if ((ret = x509write_add_name(node, oid, 3, O, O_len,
                        ASN1_PRINTABLE_STRING)) != 0)
            return ret;
    }

    /* organisation unit */
    if (OU != NULL) {
        oid[2] = X520_ORG_UNIT;
        if ((ret = x509write_add_name(node, oid, 3, OU, OU_len,
                        ASN1_PRINTABLE_STRING)) != 0)
            return ret;
    }

    /* common name */
    if (CN != NULL) {
        oid[2] = X520_COMMON_NAME;
        if ((ret = x509write_add_name(node, oid, 3, CN, CN_len,
                        ASN1_PRINTABLE_STRING)) != 0)
            return ret;
    }

    /* email */
    if (R != NULL) {
        if ((ret = x509write_add_name(node, (unsigned char*)OID_PKCS9_EMAIL,
        		9, R, R_len, ASN1_IA5_STRING)) != 0)
            return ret;
    }

    if ((asn1_append_tag(node, ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
        return ret;

    return 0;
}

/*
 * Copy raw data from orginal ca to node
 */
static int x509write_copy_from_raw(x509_node *node, x509_buf *raw)
{
    if (x509write_realloc_node(node, raw->len) != 0)
        return 1;

    memcpy(node->p, raw->p, (size_t)raw->len);
    if ((node->p += raw->len -1) != node->end)
        return POLARSSL_ERR_X509_POINT_ERROR;

    return 0;
}

/*
 * Add the issuer
 */

int x509write_add_issuer(x509_raw *crt, unsigned char *issuer)
{
    return x509write_parse_names(&crt->issuer, issuer);
}

/*
 * Add the subject
 */
int x509write_add_subject(x509_raw *crt, unsigned char *subject)
{
    return x509write_parse_names(&crt->subject, subject);
}

/*
 * Copy issuer line from another cert to issuer
 */
int x509write_copy_issuer(x509_raw *crt, x509_cert *from_crt)
{
    return x509write_copy_from_raw(&crt->issuer, &from_crt->issuer_raw);
}

/*
 * Copy subject line from another cert
 */
int x509write_copy_subject(x509_raw *crt, x509_cert *from_crt)
{
    return x509write_copy_from_raw(&crt->subject, &from_crt->subject_raw);
}

/*
 * Copy subject line form antoher cert into issuer
 */
int x509write_copy_issuer_form_subject(x509_raw *crt,
        x509_cert *from_crt)
{
    return x509write_copy_from_raw(&crt->issuer, &from_crt->subject_raw);
}

/*
 * Copy issuer line from another cert into subject
 */
int x509write_copy_subject_from_issuer(x509_raw *crt,
        x509_cert * from_crt)
{
    return x509write_copy_from_raw(&crt->subject, &from_crt->issuer_raw);
}

/*
 *  Validity ::= SEQUENCE {
 *       notBefore      Time,
 *       notAfter       Time }
 *
 *  Time ::= CHOICE {
 *       utcTime        UTCTime,
 *       generalTime    GeneralizedTime }
 */
/* TODO: No handle GeneralizedTime! */
int x509write_add_validity(x509_raw *chain, unsigned char *befor,
        unsigned char *after)
{
    int ret;

    x509_node *node = &chain->validity;

    /* notBefore */
    if ((ret = asn1_add_date_utc(befor, node)) != 0)
        return ret;

    /* notAfter */
    if ((ret = asn1_add_date_utc(after, node)) != 0)
        return ret;

    if ((ret = asn1_append_tag(node, ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
        return ret;

    return 0;
}

/*
 * make hash from tbs and sign that with private key
 */
static int x509write_make_sign(x509_raw *chain, rsa_context *privkey)
{
    int ret;
    unsigned char hash[20], *sign;
    size_t sign_len = (size_t) mpi_size(&privkey->N);

    /* make hash */
    sha1(chain->tbs.data, chain->tbs.len, hash);

    /* create sign */
    sign = (unsigned char *) malloc(sign_len);
    if (sign == NULL)
        return 1;

    if ((ret = rsa_pkcs1_sign(privkey, RSA_PRIVATE, RSA_SHA1, 20, hash,
                    sign)) != 0)
        return ret;

    if ((ret = asn1_add_obj(sign, sign_len, ASN1_BIT_STRING,
                    &chain->sign)) != 0)
        return ret;

    /*
     *  AlgorithmIdentifier  ::=  SEQUENCE  {
     *       algorithm               OBJECT IDENTIFIER,
     *       parameters              ANY DEFINED BY algorithm OPTIONAL  }
     */
    return asn1_add_oid(&chain->signalg, (unsigned char*)OID_PKCS1_RSA_SHA, 9,
                  ASN1_CONSTRUCTED | ASN1_SEQUENCE, ASN1_NULL,
                  (unsigned char*)"", 0);
}

/*
 * Create a self signed certificate
 */
int x509write_create_sign(x509_raw *chain, rsa_context *privkey)
{
    int ret, serial;

    /*
     *  Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
     */
    if ((ret = asn1_add_int(2, &chain->version)) != 0)
        return ret;

    if ((ret = asn1_append_tag(&chain->version, ASN1_CONTEXT_SPECIFIC |
                    ASN1_CONSTRUCTED)) != 0)
        return ret;


    /*
     *  CertificateSerialNumber  ::=  INTEGER
     */
    srand((unsigned int) time(NULL));
    serial = rand();
    if ((ret = asn1_add_int(serial, &chain->serial)) != 0)
        return ret;

    /*
     *  AlgorithmIdentifier  ::=  SEQUENCE  {
     *       algorithm               OBJECT IDENTIFIER,
     *       parameters              ANY DEFINED BY algorithm OPTIONAL  }
     */
    if ((ret = asn1_add_oid(&chain->tbs_signalg,
				(unsigned char*)OID_PKCS1_RSA_SHA, 9, ASN1_CONSTRUCTED |
				ASN1_SEQUENCE, ASN1_NULL, (unsigned char*)"", 0)) != 0)
        return ret;

   /*
    *  Create the tbs
    */
    if ((ret = asn1_append_nodes(&chain->tbs, ASN1_CONSTRUCTED |
                    ASN1_SEQUENCE, 7, &chain->version, &chain->serial,
                    &chain->tbs_signalg, &chain->issuer, &chain->validity,
                    &chain->subject, &chain->subpubkey)) != 0)
        return ret;

    /* make signing */
    if ((ret = x509write_make_sign(chain, privkey)) != 0)
        return ret;

    /* finishing */
    if ((ret = asn1_append_nodes(&chain->raw, ASN1_CONSTRUCTED |
                    ASN1_SEQUENCE, 3, &chain->tbs, &chain->signalg,
                    &chain->sign)) != 0)
        return ret;

    return 0;
}

int x509write_create_selfsign(x509_raw *chain, rsa_context *privkey)
{
    /*
     * On self signed certificate are subject and issuer the same
     */
    x509write_free_node(&chain->issuer);
    chain->issuer = chain->subject;
    return x509write_create_sign(chain, privkey);
}

/*
 * CertificationRequestInfo ::= SEQUENCE                    {
 *    version                       Version,
 *    subject                       Name,
 *    subjectPublicKeyInfo          SubjectPublicKeyInfo,
 *    attributes                    [0] IMPLICIT Attributes }
 *
 * CertificationRequest ::=   SEQUENCE                      {
 *    certificationRequestInfo  CertificationRequestInfo,
 *    signatureAlgorithm        SignatureAlgorithmIdentifier,
 *    signature                 Signature                   }
 *
 * It use chain.serail for attributes!
 *
 */
int x509write_create_csr(x509_raw *chain, rsa_context *privkey)
{
    int ret;

    /* version ::= INTEGER */
    if ((ret = asn1_add_int(0, &chain->version)) != 0)
        return ret;

    /* write attributes */
    if ((ret = asn1_add_obj((unsigned char*)"", 0, ASN1_CONTEXT_SPECIFIC |
                    ASN1_CONSTRUCTED, &chain->serial)) != 0)
        return ret;

    /* create CertificationRequestInfo */
    if ((ret = asn1_append_nodes(&chain->tbs, ASN1_CONSTRUCTED |
                    ASN1_SEQUENCE, 4, &chain->version, &chain->subject,
                    &chain->subpubkey, &chain->serial)) != 0)
        return ret;

    /* make signing */
    if ((ret = x509write_make_sign(chain, privkey)) != 0)
        return ret;

    /* finish */
    if ((ret = asn1_append_nodes(&chain->raw, ASN1_CONSTRUCTED | ASN1_SEQUENCE,
                    3, &chain->tbs, &chain->signalg, &chain->sign)) != 0)
        return ret;

    return ret;
}

/*
 * Free memory
 */
void x509write_free_raw(x509_raw *chain)
{
    x509write_free_node(&chain->raw);
    x509write_free_node(&chain->tbs);
    x509write_free_node(&chain->version);
    x509write_free_node(&chain->serial);
    x509write_free_node(&chain->tbs_signalg);
    x509write_free_node(&chain->issuer);
    x509write_free_node(&chain->validity);
    if (chain->subject.data != chain->issuer.data)
        x509write_free_node(&chain->subject);
    x509write_free_node(&chain->subpubkey);
    x509write_free_node(&chain->signalg);
    x509write_free_node(&chain->sign);
}

void x509write_init_raw(x509_raw *chain)
{
    memset((void *) chain, 0, sizeof(x509_raw));
}

