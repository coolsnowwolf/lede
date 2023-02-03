/* //device/system/reference-ril/atchannel.h
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ATCHANNEL_H
#define ATCHANNEL_H 1

#ifdef __cplusplus
extern "C" {
#endif

/* define AT_DEBUG to send AT traffic to /tmp/radio-at.log" */
#define AT_DEBUG  0

#if AT_DEBUG
extern void  AT_DUMP(const char* prefix, const char*  buff, int  len);
#else
#define  AT_DUMP(prefix,buff,len)  do{}while(0)
#endif

#define AT_ERROR_GENERIC -1
#define AT_ERROR_COMMAND_PENDING -2
#define AT_ERROR_CHANNEL_CLOSED -3
#define AT_ERROR_TIMEOUT -4
#define AT_ERROR_INVALID_THREAD -5 /* AT commands may not be issued from
                                       reader thread (or unsolicited response
                                       callback */
#define AT_ERROR_INVALID_RESPONSE -6 /* eg an at_send_command_singleline that
                                        did not get back an intermediate
                                        response */


typedef enum {
    NO_RESULT,   /* no intermediate response expected */
    NUMERIC,     /* a single intermediate response starting with a 0-9 */
    SINGLELINE,  /* a single intermediate response starting with a prefix */
    MULTILINE    /* multiple line intermediate response
                    starting with a prefix */
} ATCommandType;

/** a singly-lined list of intermediate responses */
typedef struct ATLine  {
    struct ATLine *p_next;
    char *line;
} ATLine;

/** Free this with at_response_free() */
typedef struct {
    int success;              /* true if final response indicates
                                    success (eg "OK") */
    char *finalResponse;      /* eg OK, ERROR */
    ATLine  *p_intermediates; /* any intermediate responses */
} ATResponse;

/**
 * a user-provided unsolicited response handler function
 * this will be called from the reader thread, so do not block
 * "s" is the line, and "sms_pdu" is either NULL or the PDU response
 * for multi-line TS 27.005 SMS PDU responses (eg +CMT:)
 */
typedef void (*ATUnsolHandler)(const char *s, const char *sms_pdu);

int at_open(int fd, ATUnsolHandler h, int proxy);
void at_close();

/* This callback is invoked on the command thread.
   You should reset or handshake here to avoid getting out of sync */
void at_set_on_timeout(void (*onTimeout)(void));
/* This callback is invoked on the reader thread (like ATUnsolHandler)
   when the input stream closes before you call at_close
   (not when you call at_close())
   You should still call at_close()
   It may also be invoked immediately from the current thread if the read
   channel is already closed */
void at_set_on_reader_closed(void (*onClose)(void));

int at_send_command_singleline (const char *command,
                                const char *responsePrefix,
                                 ATResponse **pp_outResponse);

int at_send_command_numeric (const char *command,
                                 ATResponse **pp_outResponse);

int at_send_command_multiline (const char *command,
                                const char *responsePrefix,
                                 ATResponse **pp_outResponse);

int at_send_command_raw (const char *command,
                                const char *raw_data, unsigned int raw_len,
                                const char *responsePrefix,
                                 ATResponse **pp_outResponse);

int at_handshake();

int at_send_command (const char *command, ATResponse **pp_outResponse);

int at_send_command_sms (const char *command, const char *pdu,
                            const char *responsePrefix,
                            ATResponse **pp_outResponse);

void at_response_free(ATResponse *p_response);

int strStartsWith(const char *line, const char *prefix);

typedef enum {
    CME_ERROR_NON_CME = -1,
    CME_SUCCESS = 0,

    CME_OPERATION_NOT_ALLOWED = 3,
    CME_OPERATION_NOT_SUPPORTED = 4,
    CME_PH_SIM_PIN= 5,
    CME_PH_FSIM_PIN = 6,
    CME_PH_FSIM_PUK = 7,
    CME_SIM_NOT_INSERTED =10,
    CME_SIM_PIN_REQUIRED = 11,
    CME_SIM_PUK_REQUIRED = 12,
    CME_FAILURE = 13,
    CME_SIM_BUSY = 14,
    CME_SIM_WRONG = 15,
    CME_INCORRECT_PASSWORD = 16,
    CME_SIM_PIN2_REQUIRED = 17,
    CME_SIM_PUK2_REQUIRED = 18,
    CME_MEMORY_FULL = 20,
    CME_INVALID_INDEX = 21,
    CME_NOT_FOUND = 22,
    CME_MEMORY_FAILURE = 23,
    CME_STRING_TO_LONG = 24,
    CME_INVALID_CHAR = 25,
    CME_DIALSTR_TO_LONG = 26,
    CME_INVALID_DIALCHAR = 27,
} AT_CME_Error;

AT_CME_Error at_get_cme_error(const ATResponse *p_response);

#ifdef __cplusplus
}
#endif

#endif /*ATCHANNEL_H*/
