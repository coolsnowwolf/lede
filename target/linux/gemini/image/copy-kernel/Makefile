#
# Makefile for Gemin kernel copy stub
#
# Copyright (C) 2019 Linus Walleij <linus.walleij@linaro.org>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as published
# by the Free Software Foundation.
#

AS		:= $(CROSS_COMPILE)as
OBJCOPY		:= $(CROSS_COMPILE)objcopy

BIN_FLAGS	:= -O binary -S

SRC_DIR		:= $(CURDIR)/
OUT_DIR		:= $(if $(O),$(if $(patsubst %/,,$(O)),$(O)/,$(O)),$(SRC_DIR))

all: $(OUT_DIR)copy-kernel.bin

# Don't build dependencies, this may die if $(CC) isn't gcc
dep:

install:

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OUT_DIR)%.o : $(SRC_DIR)%.S | $(OUT_DIR)
	$(AS) $(ASFLAGS) -k -o $@ $<

$(OUT_DIR)%.bin: $(OUT_DIR)%.o
	$(OBJCOPY) $(BIN_FLAGS) $< $@

mrproper: clean

clean:
	rm -f $(OUT_DIR)copy-kernel.bin $(OUT_DIR)copy-kernel.o
