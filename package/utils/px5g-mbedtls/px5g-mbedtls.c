/*
 * px5g - Embedded x509 key and certificate generator based on PolarSSL
 *
 *   Copyright (C) 2009 Steven Barth <steven@midlink.org>
 *   Copyright (C) 2014 Felix Fietkau <nbd@nbd.name>
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

#include <sys/types.h>
#include <sys/random.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#include <mbedtls/bignum.h>
#include <mbedtls/entropy.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/ecp.h>
#include <mbedtls/rsa.h>
#include <mbedtls/pk.h>

#define PX5G_VERSION "0.2"
#define PX5G_COPY "Copyright (c) 2009 Steven Barth <steven@midlink.org>"
#define PX5G_LICENSE "Licensed under the GNU Lesser General Public License v2.1"

static char buf[16384];

static int _urandom(void *ctx, unsigned char *out, size_t len)
{
	ssize_t ret;

	ret = getrandom(out, len, 0);
	if (ret < 0 || (size_t)ret != len)
		return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;

	return 0;
}

static void write_file(const char *path, int len, bool pem)
{
	FILE *f = stdout;
	const char *buf_start = buf;

	if (!pem)
		buf_start += sizeof(buf) - len;

	if (!len) {
		fprintf(stderr, "No data to write\n");
		exit(1);
	}

	if (!f) {
		fprintf(stderr, "error: I/O error\n");
		exit(1);
	}

	if (path)
		f = fopen(path, "w");

	fwrite(buf_start, 1, len, f);
	fclose(f);
}

static mbedtls_ecp_group_id ecp_curve(const char *name)
{
	const mbedtls_ecp_curve_info *curve_info;

	if (!strcmp(name, "P-256"))
		return MBEDTLS_ECP_DP_SECP256R1;
	else if (!strcmp(name, "P-384"))
		return MBEDTLS_ECP_DP_SECP384R1;
	else if (!strcmp(name, "P-521"))
		return MBEDTLS_ECP_DP_SECP521R1;
	curve_info = mbedtls_ecp_curve_info_from_name(name);
	if (curve_info == NULL)
		return MBEDTLS_ECP_DP_NONE;
	else
		return curve_info->grp_id;
}

static void write_key(mbedtls_pk_context *key, const char *path, bool pem)
{
	int len = 0;

	if (pem) {
		if (mbedtls_pk_write_key_pem(key, (void *) buf, sizeof(buf)) == 0)
			len = strlen(buf);
	} else {
		len = mbedtls_pk_write_key_der(key, (void *) buf, sizeof(buf));
		if (len < 0)
			len = 0;
	}

	write_file(path, len, pem);
}

static void gen_key(mbedtls_pk_context *key, bool rsa, int ksize, int exp,
		    mbedtls_ecp_group_id curve, bool pem)
{
	mbedtls_pk_init(key);
	if (rsa) {
		fprintf(stderr, "Generating RSA private key, %i bit long modulus\n", ksize);
		mbedtls_pk_setup(key, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
		if (!mbedtls_rsa_gen_key(mbedtls_pk_rsa(*key), _urandom, NULL, ksize, exp))
			return;
	} else {
		fprintf(stderr, "Generating EC private key\n");
		mbedtls_pk_setup(key, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
		if (!mbedtls_ecp_gen_key(curve, mbedtls_pk_ec(*key), _urandom, NULL))
			return;
	}
	fprintf(stderr, "error: key generation failed\n");
	exit(1);
}

int dokey(bool rsa, char **arg)
{
	mbedtls_pk_context key;
	unsigned int ksize = 512;
	int exp = 65537;
	char *path = NULL;
	bool pem = true;
	mbedtls_ecp_group_id curve = MBEDTLS_ECP_DP_SECP256R1;

	while (*arg && **arg == '-') {
		if (!strcmp(*arg, "-out") && arg[1]) {
			path = arg[1];
			arg++;
		} else if (!strcmp(*arg, "-3")) {
			exp = 3;
		} else if (!strcmp(*arg, "-der")) {
			pem = false;
		}
		arg++;
	}

	if (*arg && rsa) {
		ksize = (unsigned int)atoi(*arg);
	} else if (*arg) {
		curve = ecp_curve((const char *)*arg);
		if (curve == MBEDTLS_ECP_DP_NONE) {
			fprintf(stderr, "error: invalid curve name: %s\n", *arg);
			return 1;
		}
	}

	gen_key(&key, rsa, ksize, exp, curve, pem);
	write_key(&key, path, pem);

	mbedtls_pk_free(&key);

	return 0;
}

int selfsigned(char **arg)
{
	mbedtls_pk_context key;
	mbedtls_x509write_cert cert;
	mbedtls_mpi serial;

	char *subject = "";
	unsigned int ksize = 512;
	int exp = 65537;
	unsigned int days = 30;
	char *keypath = NULL, *certpath = NULL;
	bool pem = true;
	time_t from = time(NULL), to;
	char fstr[20], tstr[20], sstr[17];
	int len;
	bool rsa = true;
	mbedtls_ecp_group_id curve = MBEDTLS_ECP_DP_SECP256R1;

	while (*arg && **arg == '-') {
		if (!strcmp(*arg, "-der")) {
			pem = false;
		} else if (!strcmp(*arg, "-newkey") && arg[1]) {
			if (!strncmp(arg[1], "rsa:", 4)) {
				rsa = true;
				ksize = (unsigned int)atoi(arg[1] + 4);
			} else if (!strcmp(arg[1], "ec")) {
				rsa = false;
			} else {
				fprintf(stderr, "error: invalid algorithm\n");
				return 1;
			}
			arg++;
		} else if (!strcmp(*arg, "-days") && arg[1]) {
			days = (unsigned int)atoi(arg[1]);
			arg++;
		} else if (!strcmp(*arg, "-pkeyopt") && arg[1]) {
			if (strncmp(arg[1], "ec_paramgen_curve:", 18)) {
				fprintf(stderr, "error: invalid pkey option: %s\n", arg[1]);
				return 1;
			}
			curve = ecp_curve((const char *)(arg[1] + 18));
			if (curve == MBEDTLS_ECP_DP_NONE) {
				fprintf(stderr, "error: invalid curve name: %s\n", arg[1] + 18);
				return 1;
			}
			arg++;
		} else if (!strcmp(*arg, "-keyout") && arg[1]) {
			keypath = arg[1];
			arg++;
		} else if (!strcmp(*arg, "-out") && arg[1]) {
			certpath = arg[1];
			arg++;
		} else if (!strcmp(*arg, "-subj") && arg[1]) {
			if (arg[1][0] != '/' || strchr(arg[1], ';')) {
				fprintf(stderr, "error: invalid subject");
				return 1;
			}
			subject = calloc(strlen(arg[1]) + 1, 1);
			char *oldc = arg[1] + 1, *newc = subject, *delim;
			do {
				delim = strchr(oldc, '=');
				if (!delim) {
					fprintf(stderr, "error: invalid subject");
					return 1;
				}
				memcpy(newc, oldc, delim - oldc + 1);
				newc += delim - oldc + 1;
				oldc = delim + 1;

				delim = strchr(oldc, '/');
				if (!delim) {
					delim = arg[1] + strlen(arg[1]);
				}
				memcpy(newc, oldc, delim - oldc);
				newc += delim - oldc;
				*newc++ = ',';
				oldc = delim + 1;
			} while(*delim);
			arg++;
		}
		arg++;
	}
	gen_key(&key, rsa, ksize, exp, curve, pem);

	if (keypath)
		write_key(&key, keypath, pem);

	from = (from < 1000000000) ? 1000000000 : from;
	strftime(fstr, sizeof(fstr), "%Y%m%d%H%M%S", gmtime(&from));
	to = from + 60 * 60 * 24 * days;
	if (to < from)
		to = INT_MAX;
	strftime(tstr, sizeof(tstr), "%Y%m%d%H%M%S", gmtime(&to));

	fprintf(stderr, "Generating selfsigned certificate with subject '%s'"
			" and validity %s-%s\n", subject, fstr, tstr);

	mbedtls_x509write_crt_init(&cert);
	mbedtls_x509write_crt_set_md_alg(&cert, MBEDTLS_MD_SHA256);
	mbedtls_x509write_crt_set_issuer_key(&cert, &key);
	mbedtls_x509write_crt_set_subject_key(&cert, &key);
	mbedtls_x509write_crt_set_subject_name(&cert, subject);
	mbedtls_x509write_crt_set_issuer_name(&cert, subject);
	mbedtls_x509write_crt_set_validity(&cert, fstr, tstr);
	mbedtls_x509write_crt_set_basic_constraints(&cert, 0, -1);
	mbedtls_x509write_crt_set_subject_key_identifier(&cert);
	mbedtls_x509write_crt_set_authority_key_identifier(&cert);

	_urandom(NULL, (void *) buf, 8);
	for (len = 0; len < 8; len++)
		sprintf(sstr + len*2, "%02x", (unsigned char) buf[len]);

	mbedtls_mpi_init(&serial);
	mbedtls_mpi_read_string(&serial, 16, sstr);
	mbedtls_x509write_crt_set_serial(&cert, &serial);

	if (pem) {
		if (mbedtls_x509write_crt_pem(&cert, (void *) buf, sizeof(buf), _urandom, NULL) < 0) {
			fprintf(stderr, "Failed to generate certificate\n");
			return 1;
		}

		len = strlen(buf);
	} else {
		len = mbedtls_x509write_crt_der(&cert, (void *) buf, sizeof(buf), _urandom, NULL);
		if (len < 0) {
			fprintf(stderr, "Failed to generate certificate: %d\n", len);
			return 1;
		}
	}
	write_file(certpath, len, pem);

	mbedtls_x509write_crt_free(&cert);
	mbedtls_mpi_free(&serial);
	mbedtls_pk_free(&key);

	return 0;
}

int main(int argc, char *argv[])
{
	if (!argv[1]) {
		//Usage
	} else if (!strcmp(argv[1], "eckey")) {
		return dokey(false, argv+2);
	} else if (!strcmp(argv[1], "rsakey")) {
		return dokey(true, argv+2);
	} else if (!strcmp(argv[1], "selfsigned")) {
		return selfsigned(argv+2);
	}

	fprintf(stderr,
		"PX5G X.509 Certificate Generator Utility v" PX5G_VERSION "\n" PX5G_COPY
		"\nbased on PolarSSL by Christophe Devine and Paul Bakker\n\n");
	fprintf(stderr, "Usage: %s [eckey|rsakey|selfsigned]\n", *argv);
	return 1;
}
