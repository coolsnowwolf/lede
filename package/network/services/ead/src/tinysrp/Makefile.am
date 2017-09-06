AUTOMAKE_OPTIONS = foreign no-dependencies

noinst_HEADERS = t_client.h t_pwd.h t_server.h t_sha.h \
  bn.h bn_lcl.h bn_prime.h t_defines.h t_read.h

include_HEADERS = tinysrp.h

lib_LIBRARIES = libtinysrp.a

CFLAGS = -O2 @signed@

libtinysrp_a_SOURCES = \
  tinysrp.c t_client.c t_getconf.c t_conv.c t_getpass.c t_sha.c t_math.c \
  t_misc.c t_pw.c t_read.c t_server.c t_truerand.c \
  bn_add.c bn_ctx.c bn_div.c bn_exp.c bn_mul.c bn_word.c bn_asm.c bn_lib.c \
  bn_shift.c bn_sqr.c

noinst_PROGRAMS = srvtest clitest
srvtest_SOURCES = srvtest.c
clitest_SOURCES = clitest.c

bin_PROGRAMS = tconf tphrase
tconf_SOURCES = tconf.c t_conf.c
tphrase_SOURCES = tphrase.c

LDADD = libtinysrp.a

EXTRA_DIST = tpasswd Notes
