/* Add passphrases to the tpasswd file.  Use the last entry in the config
file by default or a particular one specified by index. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "config.h"
#include "t_pwd.h"
#include "t_read.h"
#include "t_sha.h"
#include "t_defines.h"

char *Progname;
char Usage[] = "usage: %s [-n configindex] [-p passfile] user\n";
#define USAGE() fprintf(stderr, Usage, Progname)

void doit(char *);

int Configindex = -1;
char *Passfile = DEFAULT_PASSWD;

int main(int argc, char **argv)
{
	int c;

	Progname = *argv;

	/* Parse option arguments. */

	while ((c = getopt(argc, argv, "n:p:")) != EOF) {
		switch (c) {

		case 'n':
			Configindex = atoi(optarg);
			break;

		case 'p':
			Passfile = optarg;
			break;

		default:
			USAGE();
			exit(1);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1) {
		USAGE();
		exit(1);
	}
	doit(argv[0]);

	return 0;
}

void doit(char *name)
{
	char passphrase[128], passphrase1[128];
	FILE *f;
	struct t_confent *tcent;
	struct t_pw eps_passwd;

	/* Get the config entry. */

	if (Configindex <= 0) {
		Configindex = t_getprecount();
	}
	tcent = gettcid(Configindex);
	if (tcent == NULL) {
		fprintf(stderr, "Invalid configuration file entry.\n");
		exit(1);
	}

	/* Ask for the passphrase twice. */

	printf("Setting passphrase for %s\n", name);

	if (t_getpass(passphrase, sizeof(passphrase), "Enter passphrase: ") < 0) {
		exit(1);
	}
	if (t_getpass(passphrase1, sizeof(passphrase1), "Verify: ") < 0) {
		exit(1);
	}
	if (strcmp(passphrase, passphrase1) != 0) {
		fprintf(stderr, "mismatch\n");
		exit(1);
	}

	/* Create the passphrase verifier. */

	t_makepwent(&eps_passwd, name, passphrase, NULL, tcent);

	/* Don't need these anymore. */

	memset(passphrase, 0, sizeof(passphrase));
	memset(passphrase1, 0, sizeof(passphrase1));

	/* See if the passphrase file is there; create it if not. */

	if ((f = fopen(Passfile, "r+")) == NULL) {
		creat(Passfile, 0400);
	} else {
		fclose(f);
	}

	/* Change the passphrase. */

	if (t_changepw(Passfile, &eps_passwd.pebuf) < 0) {
		fprintf(stderr, "Error changing passphrase\n");
		exit(1);
	}
}

/* TODO: Implement a more general method to handle delete/change */

_TYPE( int )
t_changepw(pwname, diff)
     const char * pwname;
     const struct t_pwent * diff;
{
  char * bakfile;
  char * bakfile2;
  struct stat st;
  FILE * passfp;
  FILE * bakfp;

  if(pwname == NULL)
    pwname = DEFAULT_PASSWD;

  if((passfp = fopen(pwname, "rb")) == NULL || fstat(fileno(passfp), &st) < 0)
    return -1;

  if((bakfile = malloc(strlen(pwname) + 5)) == NULL) {
    fclose(passfp);
    return -1;
  }
  else if((bakfile2 = malloc(strlen(pwname) + 5)) == NULL) {
    fclose(passfp);
    free(bakfile);
    return -1;
  }

  sprintf(bakfile, "%s.bak", pwname);
  sprintf(bakfile2, "%s.sav", pwname);

  if((bakfp = fopen(bakfile2, "wb")) == NULL &&
     (unlink(bakfile2) < 0 || (bakfp = fopen(bakfile2, "wb")) == NULL)) {
    fclose(passfp);
    free(bakfile);
    free(bakfile2);
    return -1;
  }

#ifdef NO_FCHMOD
  chmod(bakfile2, st.st_mode & 0777);
#else
  fchmod(fileno(bakfp), st.st_mode & 0777);
#endif

  t_pwcopy(bakfp, passfp, diff);

  fclose(bakfp);
  fclose(passfp);

#ifdef USE_RENAME
  unlink(bakfile);
  if(rename(pwname, bakfile) < 0) {
    free(bakfile);
    free(bakfile2);
    return -1;
  }
  if(rename(bakfile2, pwname) < 0) {
    free(bakfile);
    free(bakfile2);
    return -1;
  }
#else
  unlink(bakfile);
  link(pwname, bakfile);
  unlink(pwname);
  link(bakfile2, pwname);
  unlink(bakfile2);
#endif
  free(bakfile);
  free(bakfile2);

  return 0;
}

_TYPE( struct t_pwent * )
t_makepwent(tpw, user, pass, salt, confent)
     struct t_pw * tpw;
     const char * user;
     const char * pass;
     const struct t_num * salt;
     const struct t_confent * confent;
{
  BigInteger x, v, n, g;
  unsigned char dig[SHA_DIGESTSIZE];
  SHA1_CTX ctxt;

  tpw->pebuf.name = tpw->userbuf;
  tpw->pebuf.password.data = tpw->pwbuf;
  tpw->pebuf.salt.data = tpw->saltbuf;

  strncpy(tpw->pebuf.name, user, MAXUSERLEN);
  tpw->pebuf.index = confent->index;

  if(salt) {
    tpw->pebuf.salt.len = salt->len;
    memcpy(tpw->pebuf.salt.data, salt->data, salt->len);
  }
  else {
    memset(dig, 0, SALTLEN);            /* salt is 80 bits */
    tpw->pebuf.salt.len = SALTLEN;
    do {
      t_random(tpw->pebuf.salt.data, SALTLEN);
    } while(memcmp(tpw->pebuf.salt.data, dig, SALTLEN) == 0);
    if(tpw->pebuf.salt.data[0] == 0)
      tpw->pebuf.salt.data[0] = 0xff;
  }

  n = BigIntegerFromBytes(confent->modulus.data, confent->modulus.len);
  g = BigIntegerFromBytes(confent->generator.data, confent->generator.len);
  v = BigIntegerFromInt(0);

  SHA1Init(&ctxt);
  SHA1Update(&ctxt, user, strlen(user));
  SHA1Update(&ctxt, ":", 1);
  SHA1Update(&ctxt, pass, strlen(pass));
  SHA1Final(dig, &ctxt);

  SHA1Init(&ctxt);
  SHA1Update(&ctxt, tpw->pebuf.salt.data, tpw->pebuf.salt.len);
  SHA1Update(&ctxt, dig, sizeof(dig));
  SHA1Final(dig, &ctxt);

  /* x = H(s, H(u, ':', p)) */
  x = BigIntegerFromBytes(dig, sizeof(dig));

  BigIntegerModExp(v, g, x, n);
  tpw->pebuf.password.len = BigIntegerToBytes(v, tpw->pebuf.password.data);

  BigIntegerFree(v);
  BigIntegerFree(x);
  BigIntegerFree(g);
  BigIntegerFree(n);

  return &tpw->pebuf;
}

int
t_pwcopy(pwdest, pwsrc, diff)
     FILE * pwdest;
     FILE * pwsrc;
     struct t_pwent * diff;
{
  struct t_pw * src;
  struct t_pwent * ent;

  if((src = t_openpw(pwsrc)) == NULL)
    return -1;

  while((ent = t_getpwent(src)) != NULL)
    if(diff && strcmp(diff->name, ent->name) == 0) {
      t_putpwent(diff, pwdest);
      diff = NULL;
    }
    else
      t_putpwent(ent, pwdest);

  if(diff)
    t_putpwent(diff, pwdest);

  return 0;
}

_TYPE( struct t_pwent * )
t_getpwent(tpw)
     struct t_pw * tpw;
{
  char indexbuf[16];
  char passbuf[MAXB64PARAMLEN];
  char saltstr[MAXB64SALTLEN];

#ifdef ENABLE_YP
  struct t_passwd * nisent;
  /* FIXME: should tell caller to get conf entry from NIS also */

  if(tpw->state == IN_NIS) {
    nisent = _yp_gettpent();
    if(nisent != NULL) {
      savepwent(tpw, &nisent->tp);
      return &tpw->pebuf;
    }
    tpw->state = FILE_NIS;
  }
#endif

  while(1) {
    if(t_nextfield(tpw->instream, tpw->userbuf, MAXUSERLEN) > 0) {
#ifdef ENABLE_YP
      if(tpw->state == FILE_NIS && *tpw->userbuf == '+') {
	t_nextline(tpw->instream);
	if(strlen(tpw->userbuf) > 1) {  /* +name:... */
	  nisent = _yp_gettpnam(tpw->userbuf + 1);
	  if(nisent != NULL) {
	    savepwent(tpw, nisent);
	    return &tpw->pebuf;
	  }
	}
	else {  /* +:... */
	  tpw->state = IN_NIS;
	  _yp_settpent();
	  return t_getpwent(tpw);
	}
      }
#endif
      if(t_nextfield(tpw->instream, passbuf, MAXB64PARAMLEN) > 0 &&
	 (tpw->pebuf.password.len = t_fromb64(tpw->pwbuf, passbuf)) > 0 &&
	 t_nextfield(tpw->instream, saltstr, MAXB64SALTLEN) > 0 &&
	 (tpw->pebuf.salt.len = t_fromb64(tpw->saltbuf, saltstr)) > 0 &&
	 t_nextfield(tpw->instream, indexbuf, 16) > 0 &&
	 (tpw->pebuf.index = atoi(indexbuf)) > 0) {
	tpw->pebuf.name = tpw->userbuf;
	tpw->pebuf.password.data = tpw->pwbuf;
	tpw->pebuf.salt.data = tpw->saltbuf;
	t_nextline(tpw->instream);
	return &tpw->pebuf;
      }
    }
    if(t_nextline(tpw->instream) < 0)
      return NULL;
  }
}

_TYPE( void )
t_putpwent(ent, fp)
     const struct t_pwent * ent;
     FILE * fp;
{
  char strbuf[MAXB64PARAMLEN];
  char saltbuf[MAXB64SALTLEN];

  fprintf(fp, "%s:%s:%s:%d\n", ent->name,
	  t_tob64(strbuf, ent->password.data, ent->password.len),
	  t_tob64(saltbuf, ent->salt.data, ent->salt.len), ent->index);
}

