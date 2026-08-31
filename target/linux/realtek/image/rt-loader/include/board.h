/*
 * rt-loader header
 * (c) 2025 Markus Stockhausen
 */

#ifndef _BOARD_H_
#define _BOARD_H_

unsigned int board_get_memory(void);
void board_get_system(char *buffer, int len);
void board_panic(void);
void board_putchar(int ch, void *ctx);;

#endif  // _BOARD_H_
