# Makefile
#

# Compiler options
CC := gcc
CFLAGS := -g -O0
LDFLAGS := 
EXEEXT := 


# Useful commands
XGETTEXT := xgettext
MSGINIT := msginit
MSGMERGE := msgmerge
MSGFMT := msgfmt
MKDIR := mkdir -p
RM := rm -rf
INSTALL := install
INSTALL_PROGRAM := $(INSTALL)
INSTALL_DATA := $(INSTALL) -m 644
INSTALL_DIR := $(INSTALL) -d

# Installation configuration
PACKAGE_NAME := njit-tests
prefix := /usr/local
exec_prefix := ${prefix}
bindir := ${exec_prefix}/bin
datarootdir := ${prefix}/share
datadir := ${datarootdir}
localedir := ${datarootdir}/locale

# Configure project files
srcdir := .
podir := $(srcdir)/po
po_files := $(notdir $(wildcard $(podir)/*.po))
languages := $(basename $(po_files))
gmo_files := $(addsuffix .gmo, $(languages))
vpath %.c $(srcdir)
vpath %.h $(srcdir)
vpath %.po $(podir)
vpath %.pot $(podir)
executable_files :=
source_files  :=
object_files  :=
xml_files :=
data_files :=

# Load all building dependences...
.PHONY: all
all:

-include $(srcdir)/*.mk

all: Makefile $(executable_files)

#------------------------------
# Default building rules.
%: %.o
	$(LINK.o) $^ $(LDLIBS) -o $@
%.o: %.c
	$(COMPILE.c) $< -o $@
%.pot: %.c
	$(XGETTEXT) --from-code=UTF-8 -k_ $^ --output-dir=$(podir) --output=$@
%.gmo: %.po
	$(MSGFMT) --directory=$(podir) $(notdir $<) -o $@
#------------------------------
# make clean
.PHONY: clean
clean:
	$(RM) $(executable_files)
	$(RM) $(object_files)
ifdef gmo_files
	$(RM) $(gmo_files)
endif
#------------------------------
# make distclean
.PHONY: distclean
distclean: clean
	$(RM) config.log config.status
#------------------------------
# make update-po
.PHONY: update-po
update-po: $(po_files)
#------------------------------
# make install
.PHONY: install install-exec install-data install-mo
install: install-exec install-data install-mo
install-exec: $(executable_files)
	$(INSTALL_DIR) $(bindir)
	$(INSTALL_PROGRAM) $(executable_files) $(bindir)
install-data: $(data_files) $(xml_files)
	$(INSTALL_DIR) $(datadir)
	$(INSTALL_DATA) $(data_files) $(xml_files) $(datadir)
install-mo: $(gmo_files)
	for i in $^ ; do \
		lang=`basename $$i .gmo` ; \
		$(INSTALL_DIR) $(localedir)/$$lang/LC_MESSAGES ; \
		$(INSTALL_DATA) $$i $(localedir)/$$lang/LC_MESSAGES/$(PACKAGE_NAME).mo ; \
	done

