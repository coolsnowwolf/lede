// Copyright 2020 Paul Spooren <mail@aparcar.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/asn.h>
#include <wolfssl/wolfcrypt/asn_public.h>
#include <wolfssl/wolfcrypt/ecc.h>
#include <wolfssl/wolfcrypt/error-crypt.h>
#include <wolfssl/wolfcrypt/rsa.h>
#include <wolfssl/wolfcrypt/settings.h>

#define HEAP_HINT NULL
#define FOURK_SZ 4096
#define WOLFSSL_MIN_RSA_BITS 2048

enum {
  EC_KEY_TYPE = 0,
  RSA_KEY_TYPE = 1,
};

int write_file(byte *buf, int bufSz, char *path) {
  int ret;
  FILE *file;
  if (path) {
    file = fopen(path, "wb");
    if (file == NULL) {
      perror("Error opening file");
      exit(1);
    }
  } else {
    file = stdout;
  }
  ret = (int)fwrite(buf, 1, bufSz, file);
  if (path) {
    fclose(file);
  }
  if (ret > 0) {
    /* ret > 0 indicates a successful file write, set to zero for return */
    ret = 0;
  }
  return ret;
}

int write_key(ecc_key *ecKey, RsaKey *rsaKey, int type, int keySz, char *fName,
              bool write_pem) {
  int ret;
  byte der[FOURK_SZ] = {};
  byte pem[FOURK_SZ] = {};
  int derSz, pemSz;
  if (type == EC_KEY_TYPE) {
    ret = wc_EccKeyToDer(ecKey, der, sizeof(der));
  } else {
    ret = wc_RsaKeyToDer(rsaKey, der, sizeof(der));
  }
  if (ret <= 0) {
    fprintf(stderr, "Key To DER failed: %d\n", ret);
  }
  derSz = ret;

  if (write_pem) {
    if (type == EC_KEY_TYPE) {
      ret = wc_DerToPem(der, derSz, pem, sizeof(pem), ECC_PRIVATEKEY_TYPE);
    } else {
      ret = wc_DerToPem(der, derSz, pem, sizeof(pem), PRIVATEKEY_TYPE);
    }
    if (ret <= 0) {
      fprintf(stderr, "DER to PEM failed: %d\n", ret);
    }
    pemSz = ret;
    ret = write_file(pem, pemSz, fName);
  } else {
    ret = write_file(der, derSz, fName);
  }
  return ret;
}

int gen_key(WC_RNG *rng, ecc_key *ecKey, RsaKey *rsaKey, int type, int keySz,
            long exp, int curve) {
  int ret;

  if (type == EC_KEY_TYPE) {
    ret = wc_ecc_init(ecKey);
    (void)rsaKey;
  } else {
    ret = wc_InitRsaKey(rsaKey, NULL);
    (void)ecKey;
  }
  if (ret != 0) {
    fprintf(stderr, "Key initialization failed: %d\n", ret);
    return ret;
  }

  if (type == EC_KEY_TYPE) {
    fprintf(stderr, "Generating EC private key\n");
    ret = wc_ecc_make_key_ex(rng, 32, ecKey, curve);
  } else {
    fprintf(stderr, "Generating RSA private key, %i bit long modulus\n", keySz);
    ret = wc_MakeRsaKey(rsaKey, keySz, WC_RSA_EXPONENT, rng);
  }
  if (ret != 0) {
    fprintf(stderr, "Key generation failed: %d\n", ret);
  }
  return ret;
}

int selfsigned(WC_RNG *rng, char **arg) {
  ecc_key ecKey;
  RsaKey rsaKey;
  int ret;
  char *subject = "";
  int keySz = WOLFSSL_MIN_RSA_BITS;
  int type = EC_KEY_TYPE;
  int exp = WC_RSA_EXPONENT;
  int curve = ECC_SECP256R1;
  unsigned int days = 3653; // 10 years
  char *keypath = NULL, *certpath = NULL;
  char fstr[20], tstr[20];
  bool pem = true;
  Cert newCert;
#ifdef __USE_TIME_BITS64
  time_t to, from = time(NULL);
#else
  unsigned long to, from = time(NULL);
#endif
  byte derBuf[FOURK_SZ] = {};
  byte pemBuf[FOURK_SZ] = {};
  int pemSz = -1;
  int derSz = -1;
  char *key, *val, *tmp;

  ret = wc_InitCert(&newCert);
  if (ret != 0) {
    fprintf(stderr, "Init Cert failed: %d\n", ret);
    return ret;
  }
  newCert.isCA = 0;

  while (*arg && **arg == '-') {
    if (!strncmp(*arg, "-der", 4)) {
      pem = false;
    } else if (!strncmp(*arg, "-newkey", 6) && arg[1]) {
      if (!strncmp(arg[1], "rsa:", 4)) {
        type = RSA_KEY_TYPE;
        keySz = (unsigned int)atoi(arg[1] + 4);
      } else if (!strncmp(arg[1], "ec", 2)) {
        type = EC_KEY_TYPE;
      } else {
        fprintf(stderr, "error: invalid algorithm\n");
        return 1;
      }
      arg++;
    } else if (!strncmp(*arg, "-days", 5) && arg[1]) {
      days = (unsigned int)atoi(arg[1]);
      arg++;
    } else if (!strncmp(*arg, "-pkeyopt", 8) && arg[1]) {
      if (strncmp(arg[1], "ec_paramgen_curve:", 18)) {
        fprintf(stderr, "error: invalid pkey option: %s\n", arg[1]);
        return 1;
      }
      if (!strncmp(arg[1] + 18, "P-256:", 5)) {
        curve = ECC_SECP256R1;
      } else if (!strncmp(arg[1] + 18, "P-384:", 5)) {
        curve = ECC_SECP384R1;
      } else if (!strncmp(arg[1] + 18, "P-521:", 5)) {
        curve = ECC_SECP521R1;
      } else {
        fprintf(stderr, "error: invalid curve name: %s\n", arg[1] + 18);
        return 1;
      }
      arg++;
    } else if (!strncmp(*arg, "-keyout", 7) && arg[1]) {
      keypath = arg[1];
      arg++;
    } else if (!strncmp(*arg, "-out", 4) && arg[1]) {
      certpath = arg[1];
      arg++;
    } else if (!strcmp(*arg, "-subj") && arg[1]) {
      subject = strdupa(arg[1]);
      key = arg[1];
      do {
        tmp = strchr(key, '/');
        if (tmp)
          *tmp = '\0';

        val = strchr(key, '=');
        if (val) {
          *val = '\0';
          ++val;

          if (!strcmp(key, "C"))
            strncpy(newCert.subject.country, val, CTC_NAME_SIZE);
          else if (!strcmp(key, "ST"))
            strncpy(newCert.subject.state, val, CTC_NAME_SIZE);
          else if (!strcmp(key, "L"))
            strncpy(newCert.subject.locality, val, CTC_NAME_SIZE);
          else if (!strcmp(key, "O"))
            strncpy(newCert.subject.org, val, CTC_NAME_SIZE);
          else if (!strcmp(key, "OU"))
            strncpy(newCert.subject.unit, val, CTC_NAME_SIZE);
          else if (!strcmp(key, "CN")) {
            strncpy(newCert.subject.commonName, val, CTC_NAME_SIZE);

#ifdef WOLFSSL_ALT_NAMES
            if(strlen(val) + 2 > 256) {
              fprintf(stderr, "error: CN is too long: %s\n", val);
              return 1;
            }

            newCert.altNames[0] = 0x30; //Sequence with one element
            newCert.altNames[1] = strlen(val) + 2; // Length of entire sequence
            newCert.altNames[2] = 0x82; //8 - String, 2 - DNS Name
            newCert.altNames[3] = strlen(val); //DNS Name length
            memcpy(newCert.altNames + 4, val, strlen(val)); //DNS Name
            newCert.altNamesSz = strlen(val) + 4;
#endif
          }
          else if (!strcmp(key, "EMAIL"))
            strncpy(newCert.subject.email, val, CTC_NAME_SIZE);
          else
            printf("warning: unknown attribute %s=%s\n", key, val);
        }
      } while (tmp && (key = ++tmp));
    }
    arg++;
  }
  newCert.daysValid = days;

  newCert.keyUsage = KEYUSE_DIGITAL_SIG | KEYUSE_CONTENT_COMMIT | KEYUSE_KEY_ENCIPHER;
  newCert.extKeyUsage = EXTKEYUSE_SERVER_AUTH;

  gen_key(rng, &ecKey, &rsaKey, type, keySz, exp, curve);
  write_key(&ecKey, &rsaKey, type, keySz, keypath, pem);

  from = (from < 1000000000) ? 1000000000 : from;
  strftime(fstr, sizeof(fstr), "%Y%m%d%H%M%S", gmtime(&from));
  to = from + 60 * 60 * 24 * days;
  if (to < from)
    to = INT_MAX;
  strftime(tstr, sizeof(tstr), "%Y%m%d%H%M%S", gmtime(&to));

  fprintf(stderr,
          "Generating selfsigned certificate with subject '%s'"
          " and validity %s-%s\n",
          subject, fstr, tstr);

  if (type == EC_KEY_TYPE) {
    newCert.sigType = CTC_SHA256wECDSA;
    ret = wc_MakeCert(&newCert, derBuf, sizeof(derBuf), NULL, &ecKey, rng);
  } else {
    newCert.sigType = CTC_SHA256wRSA;
    ret = wc_MakeCert(&newCert, derBuf, sizeof(derBuf), &rsaKey, NULL, rng);
  }
  if (ret <= 0) {
    fprintf(stderr, "Make Cert failed: %d\n", ret);
    return ret;
  }

  if (type == EC_KEY_TYPE) {
    ret = wc_SignCert(newCert.bodySz, newCert.sigType, derBuf, sizeof(derBuf),
                      NULL, &ecKey, rng);
  } else {
    ret = wc_SignCert(newCert.bodySz, newCert.sigType, derBuf, sizeof(derBuf),
                      &rsaKey, NULL, rng);
  }
  if (ret <= 0) {
    fprintf(stderr, "Sign Cert failed: %d\n", ret);
    return ret;
  }
  derSz = ret;

  ret = wc_DerToPem(derBuf, derSz, pemBuf, sizeof(pemBuf), CERT_TYPE);
  if (ret <= 0) {
    fprintf(stderr, "DER to PEM failed: %d\n", ret);
    return ret;
  }
  pemSz = ret;

  ret = write_file(pemBuf, pemSz, certpath);
  if (ret != 0) {
    fprintf(stderr, "Write Cert failed: %d\n", ret);
    return ret;
  }

  if (type == EC_KEY_TYPE) {
    wc_ecc_free(&ecKey);
  } else {
    wc_FreeRsaKey(&rsaKey);
  }
  return 0;
}

int dokey(WC_RNG *rng, int type, char **arg) {
  ecc_key ecKey;
  RsaKey rsaKey;
  int ret;
  int curve = ECC_SECP256R1;
  int keySz = WOLFSSL_MIN_RSA_BITS;
  int exp = WC_RSA_EXPONENT;
  char *path = NULL;
  bool pem = true;

  while (*arg && **arg == '-') {
    if (!strncmp(*arg, "-out", 4) && arg[1]) {
      path = arg[1];
      arg++;
    } else if (!strncmp(*arg, "-3", 2)) {
      exp = 3;
    } else if (!strncmp(*arg, "-der", 4)) {
      pem = false;
    }
    arg++;
  }

  if (*arg && type == RSA_KEY_TYPE) {
    keySz = (unsigned int)atoi(*arg);
  } else if (*arg) {
    if (!strncmp(*arg, "P-256", 5)) {
      curve = ECC_SECP256R1;
    } else if (!strncmp(*arg, "P-384", 5)) {
      curve = ECC_SECP384R1;
    } else if (!strncmp(*arg, "P-521", 5)) {
      curve = ECC_SECP521R1;
    } else {
      fprintf(stderr, "Invalid Curve Name: %s\n", *arg);
      return 1;
    }
  }

  ret = gen_key(rng, &ecKey, &rsaKey, type, keySz, exp, curve);
  if (ret != 0)
    return ret;

  ret = write_key(&ecKey, &rsaKey, type, keySz, path, pem);

  if (type == EC_KEY_TYPE) {
    wc_ecc_free(&ecKey);
  } else {
    wc_FreeRsaKey(&rsaKey);
  }
  return ret;
}

int main(int argc, char *argv[]) {
  int ret;
  WC_RNG rng;
  ret = wc_InitRng(&rng);
  if (ret != 0) {
    fprintf(stderr, "Init Rng failed: %d\n", ret);
    return ret;
  }

  if (argv[1]) {
    if (!strncmp(argv[1], "eckey", 5))
      return dokey(&rng, EC_KEY_TYPE, argv + 2);

    if (!strncmp(argv[1], "rsakey", 5))
      return dokey(&rng, RSA_KEY_TYPE, argv + 2);

    if (!strncmp(argv[1], "selfsigned", 10))
      return selfsigned(&rng, argv + 2);
  }

  fprintf(stderr, "PX5G X.509 Certificate Generator Utilit using WolfSSL\n\n");
  fprintf(stderr, "Usage: [eckey|rsakey|selfsigned]\n");
  return 1;
}
