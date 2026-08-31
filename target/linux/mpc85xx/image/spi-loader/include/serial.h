// SPDX-License-Identifier: BSD-2-Clause

#pragma once

int serial_console_getchar(void);
int serial_console_tstc(void);
void serial_console_putchar(char c);
void serial_console_init(void);

