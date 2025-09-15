/*----------------------------------------------------------------------------
 *      ATMEL Microcontroller Software Support  -  ROUSSET  -
 *----------------------------------------------------------------------------
 * The software is delivered "AS IS" without warranty or condition of any
 * kind, either express, implied or statutory. This includes without
 * limitation any warranty or condition with respect to merchantability or
 * fitness for any particular purpose, or against the infringements of
 * intellectual property rights of others.
 *----------------------------------------------------------------------------
 * File Name           : com.h
 * Object              : 
 *
 * 1.0 27/03/03 HIi    : Creation
 *----------------------------------------------------------------------------
 */
#ifndef com_h
#define com_h

#define AT91C_CB_SIZE 20 /* size of the console buffer */

/* Escape sequences */
#define ESC \033

extern int AT91F_ReadLine (const char *const prompt, char *console_buffer);
extern void AT91F_WaitKeyPressed(void);

#endif

