/*
 * Copyright (C) 2008 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __EAD_CRYPT_H
#define __EAD_CRYPT_H

extern void ead_set_key(unsigned char *skey);
extern void ead_encrypt_message(struct ead_msg *msg, unsigned int len);
extern int ead_decrypt_message(struct ead_msg *msg);

#endif
