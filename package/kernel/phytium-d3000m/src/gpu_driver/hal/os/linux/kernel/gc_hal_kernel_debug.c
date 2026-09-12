/****************************************************************************
*
*    The MIT License (MIT)
*
*    Copyright (c) 2025, Phytium Technology Co., Ltd.
*
*    Permission is hereby granted, free of charge, to any person obtaining a
*    copy of this software and associated documentation files (the "Software"),
*    to deal in the Software without restriction, including without limitation
*    the rights to use, copy, modify, merge, publish, distribute, sublicense,
*    and/or sell copies of the Software, and to permit persons to whom the
*    Software is furnished to do so, subject to the following conditions:
*
*    The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
*
*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*    DEALINGS IN THE SOFTWARE.
*
*****************************************************************************
*
*    The GPL License (GPL)
*
*    Copyright (c) 2025, Phytium Technology Co., Ltd.
*
*    This program is free software; you can redistribute it and/or
*    modify it under the terms of the GNU General Public License
*    as published by the Free Software Foundation; either version 2
*    of the License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*****************************************************************************
*
*    Note: This software is released under dual MIT and GPL licenses. A
*    recipient may use this file under the terms of either the MIT license or
*    GPL License. If you wish to use only one license not the other, you can
*    indicate your decision by deleting one of the above license notices in your
*    version of this file.
*
*****************************************************************************/


#include "gc_hal_kernel_precomp.h"
#include <gc_hal_kernel_debug.h>

/******************************************************************************
 ******************************* Debug Variables ******************************
 ******************************************************************************/

static gceSTATUS _lastError = gcvSTATUS_OK;
static gctUINT32 _debugLevel = gcvLEVEL_ERROR;
/*
 * _debugZones config value
 * Please Reference define in gc_hal_base.h
 */
static gctUINT32 _debugZones = gcdZONE_NONE;

/******************************************************************************
 ******************************** Debug Switches ******************************
 ******************************************************************************/

/*
 *   gcdTHREAD_BUFFERS
 *
 *   When greater then one, will accumulate messages from the specified number
 *   of threads in separate output buffers.
 */
#define gcdTHREAD_BUFFERS       1

/*
 *   gcdSHOW_LINE_NUMBER
 *
 *   When enabledm each print statement will be preceded with the current
 *   line number.
 */
#define gcdSHOW_LINE_NUMBER     0

/*
 *   gcdSHOW_PROCESS_ID
 *
 *   When enabledm each print statement will be preceded with the current
 *   process ID.
 */
#define gcdSHOW_PROCESS_ID      0

/*
 *   gcdSHOW_THREAD_ID
 *
 *   When enabledm each print statement will be preceded with the current
 *   thread ID.
 */
#define gcdSHOW_THREAD_ID       0

/*
 *   gcdSHOW_TIME
 *
 *   When enabled each print statement will be preceded with the current
 *   high-resolution time.
 */
#define gcdSHOW_TIME            0

/******************************************************************************
 ***************************** Private Structures *****************************
 ******************************************************************************/

typedef struct _gcsBUFFERED_OUTPUT *gcsBUFFERED_OUTPUT_PTR;
typedef struct _gcsBUFFERED_OUTPUT {
#if gcdTHREAD_BUFFERS > 1
    gctUINT32 threadID;
#endif

#if gcdSHOW_LINE_NUMBER
    gctUINT lineNumber;
#endif

    gctINT indent;

    gcsBUFFERED_OUTPUT_PTR prev;
    gcsBUFFERED_OUTPUT_PTR next;
} gcsBUFFERED_OUTPUT;

static gcsBUFFERED_OUTPUT _outputBuffer[gcdTHREAD_BUFFERS];
static gcsBUFFERED_OUTPUT_PTR _outputBufferHead = gcvNULL;
static gcsBUFFERED_OUTPUT_PTR _outputBufferTail = gcvNULL;

/******************************************************************************
 ****************************** Printing Functions ****************************
 ******************************************************************************/

static int
_AppendIndent(gctINT Indent, char *Buffer, int BufferSize)
{
    gctINT i;

    gctINT len = 0;
    gctINT indent = Indent % 40;

    for (i = 0; i < indent; i += 1)
        Buffer[len++] = ' ';

    if (indent != Indent) {
        len += gcmkSPRINTF(Buffer + len, BufferSize - len, " <%d> ", Indent);

        Buffer[len] = '\0';
    }

    return len;
}

static gctUINT
_PrintString(gcsBUFFERED_OUTPUT_PTR OutputBuffer, gctINT Indent,
             gctCONST_STRING Message, gctPOINTER Data, char Buffer[], gctUINT Size)
{
    gctINT len;

    /* Append the indent string. */
    len = _AppendIndent(Indent, Buffer, Size);

    /* Format the string. */
    len += gcmkVSPRINTF(Buffer + len, Size - len, Message, Data);
    Buffer[len] = '\0';

    /* Add end-of-line if missing. */
    if (Buffer[len - 1] != '\n') {
        Buffer[len++] = '\n';
        Buffer[len] = '\0';
    }

    return (gctUINT)len;
}

/******************************************************************************
 ****************************** Private Functions *****************************
 ******************************************************************************/

static gcmkINLINE void
_InitBuffers(void)
{
    int i;

    if (_outputBufferHead == gcvNULL) {
        for (i = 0; i < gcdTHREAD_BUFFERS; i += 1) {
            if (_outputBufferTail == gcvNULL)
                _outputBufferHead = &_outputBuffer[i];
            else
                _outputBufferTail->next = &_outputBuffer[i];

#if gcdTHREAD_BUFFERS > 1
            _outputBuffer[i].threadID = ~0U;
#endif

            _outputBuffer[i].prev = _outputBufferTail;
            _outputBuffer[i].next = gcvNULL;

            _outputBufferTail = &_outputBuffer[i];
        }
    }
}

static gcmkINLINE gcsBUFFERED_OUTPUT_PTR
_GetOutputBuffer(void)
{
    gcsBUFFERED_OUTPUT_PTR outputBuffer;

#if gcdTHREAD_BUFFERS > 1
    /* Get the current thread ID. */
    gctUINT32 ThreadID = gcmkGETTHREADID();

    /* Locate the output buffer for the thread. */
    outputBuffer = _outputBufferHead;

    while (outputBuffer != gcvNULL) {
        if (outputBuffer->threadID == ThreadID)
            break;

        outputBuffer = outputBuffer->next;
    }

    /* No matching buffer found? */
    if (outputBuffer == gcvNULL) {
        /* Get the tail for the buffer. */
        outputBuffer = _outputBufferTail;

        /* Move it to the head. */
        _outputBufferTail = _outputBufferTail->prev;
        _outputBufferTail->next = gcvNULL;

        outputBuffer->prev = gcvNULL;
        outputBuffer->next = _outputBufferHead;

        _outputBufferHead->prev = outputBuffer;
        _outputBufferHead = outputBuffer;

        /* Reset the buffer. */
        outputBuffer->threadID = ThreadID;
#if gcdSHOW_LINE_NUMBER
        outputBuffer->lineNumber = 0;
# endif
    }
#else
    outputBuffer = _outputBufferHead;
#endif  /* #if gcdTHREAD_BUFFERS > 1 */

    return outputBuffer;
}

static void
_Print(gctCONST_STRING Message, gctARGUMENTS *Arguments)
{
    gcsBUFFERED_OUTPUT_PTR outputBuffer;
    char buffer[256];
    char *ptr = buffer;
    gctINT len = 0;

    static gcmkDECLARE_MUTEX(printMutex);

    gcmkMUTEX_LOCK(printMutex);

    /* Initialize output buffer list. */
    _InitBuffers();

    /* Locate the proper output buffer. */
    outputBuffer = _GetOutputBuffer();

    /* Form the indent string. */
    if (Message[0] == '-' && Message[1] == '-')
        outputBuffer->indent -= 2;

    /* Print the message. */
    len += _PrintString(outputBuffer,
                        outputBuffer->indent,
                        Message, ((gctPOINTER)Arguments), ptr,
                        gcmSIZEOF(buffer) - outputBuffer->indent - len);

    gcmkOUTPUT_STRING(buffer);

    /* Check increasing indent. */
    if (Message[0] == '+' && Message[1] == '+')
        outputBuffer->indent += 2;

    gcmkMUTEX_UNLOCK(printMutex);
}

/******************************************************************************
 ******************************** Debug Macros ********************************
 ******************************************************************************/

# define gcmDEBUGPRINT(Message)                               \
    {                                                         \
        gctARGUMENTS __arguments__;                           \
        gcmkARGUMENTS_START(__arguments__, Message);          \
        _Print(Message, &__arguments__);                      \
        gcmkARGUMENTS_END(__arguments__);                     \
    }


/******************************************************************************
 ********************************* Debug Code *********************************
 ******************************************************************************/

/*******************************************************************************
 **
 **  gckOS_Print
 **
 **  Send a message to the debugger.
 **
 **  INPUT:
 **
 **      gctCONST_STRING Message
 **          Pointer to message.
 **
 **      ...
 **          Optional arguments.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */

void
gckOS_Print(gctCONST_STRING Message, ...)
{
    gcmDEBUGPRINT(Message);
}

/*******************************************************************************
 **
 **  gckOS_DebugTrace
 **
 **  Send a leveled message to the debugger.
 **
 **  INPUT:
 **
 **      gctUINT32 Level
 **          Debug level of message.
 **
 **      gctCONST_STRING Message
 **          Pointer to message.
 **
 **      ...
 **          Optional arguments.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */

void
gckOS_DebugTrace(gctUINT32 Level, gctCONST_STRING Message, ...)
{
    if (Level > _debugLevel)
        return;

    gcmDEBUGPRINT(Message);
}

/*******************************************************************************
 **
 **  gckOS_DebugTraceZone
 **
 **  Send a leveled and zoned message to the debugger.
 **
 **  INPUT:
 **
 **      gctUINT32 Level
 **          Debug level for message.
 **
 **      gctUINT32 Zone
 **          Debug zone for message.
 **
 **      gctCONST_STRING Message
 **          Pointer to message.
 **
 **      ...
 **          Optional arguments.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */

void
gckOS_DebugTraceZone(gctUINT32 Level, gctUINT32 Zone, gctCONST_STRING Message, ...)
{
    if (Level > _debugLevel || !(Zone & _debugZones))
        return;

    gcmDEBUGPRINT(Message);
}

/*******************************************************************************
 **
 **  gckOS_DebugBreak
 **
 **  Break into the debugger.
 **
 **  INPUT:
 **
 **      Nothing.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
void
gckOS_DebugBreak(void)
{
    gckOS_DebugTrace(gcvLEVEL_ERROR, "%s(%d)", __func__, __LINE__);
}

/*******************************************************************************
 **
 **  gckOS_DebugFatal
 **
 **  Send a message to the debugger and break into the debugger.
 **
 **  INPUT:
 **
 **      gctCONST_STRING Message
 **          Pointer to message.
 **
 **      ...
 **          Optional arguments.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
void
gckOS_DebugFatal(gctCONST_STRING Message, ...)
{
    gcmkPRINT_VERSION();
    gcmDEBUGPRINT(Message);

    /* Break into the debugger. */
    gckOS_DebugBreak();
}

/*******************************************************************************
 **
 **  gckOS_SetDebugLevel
 **
 **  Set the debug level.
 **
 **  INPUT:
 **
 **      gctUINT32 Level
 **          New debug level.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */

void
gckOS_SetDebugLevel(gctUINT32 Level)
{
    _debugLevel = Level;
}

/*******************************************************************************
 **
 **  gckOS_SetDebugZone
 **
 **  Set the debug zone.
 **
 **  INPUT:
 **
 **      gctUINT32 Zone
 **          New debug zone.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
void
gckOS_SetDebugZone(gctUINT32 Zone)
{
    _debugZones = Zone;
}

/*******************************************************************************
 **
 **  gckOS_SetDebugLevelZone
 **
 **  Set the debug level and zone.
 **
 **  INPUT:
 **
 **      gctUINT32 Level
 **          New debug level.
 **
 **      gctUINT32 Zone
 **          New debug zone.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */

void
gckOS_SetDebugLevelZone(gctUINT32 Level, gctUINT32 Zone)
{
    _debugLevel = Level;
    _debugZones = Zone;
}

/*******************************************************************************
 **
 **  gckOS_SetDebugZones
 **
 **  Enable or disable debug zones.
 **
 **  INPUT:
 **
 **      gctUINT32 Zones
 **          Debug zones to enable or disable.
 **
 **      gctBOOL Enable
 **          Set to gcvTRUE to enable the zones (or the Zones with the current
 **          zones) or gcvFALSE to disable the specified Zones.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */

void
gckOS_SetDebugZones(gctUINT32 Zones, gctBOOL Enable)
{
    if (Enable) {
        /* Enable the zones. */
        _debugZones |= Zones;
    } else {
        /* Disable the zones. */
        _debugZones &= ~Zones;
    }
}

/*******************************************************************************
 **
 **  gckOS_Verify
 **
 **  Called to verify the result of a function call.
 **
 **  INPUT:
 **
 **      gceSTATUS Status
 **          Function call result.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */

void
gckOS_Verify(gceSTATUS status)
{
    _lastError = status;
}

gctCONST_STRING
gckOS_DebugStatus2Name(gceSTATUS status)
{
    switch (status) {
    case gcvSTATUS_OK:
        return "gcvSTATUS_OK";
    case gcvSTATUS_TRUE:
        return "gcvSTATUS_TRUE";
    case gcvSTATUS_NO_MORE_DATA:
        return "gcvSTATUS_NO_MORE_DATA";
    case gcvSTATUS_CACHED:
        return "gcvSTATUS_CACHED";
    case gcvSTATUS_MIPMAP_TOO_LARGE:
        return "gcvSTATUS_MIPMAP_TOO_LARGE";
    case gcvSTATUS_NAME_NOT_FOUND:
        return "gcvSTATUS_NAME_NOT_FOUND";
    case gcvSTATUS_NOT_OUR_INTERRUPT:
        return "gcvSTATUS_NOT_OUR_INTERRUPT";
    case gcvSTATUS_MISMATCH:
        return "gcvSTATUS_MISMATCH";
    case gcvSTATUS_MIPMAP_TOO_SMALL:
        return "gcvSTATUS_MIPMAP_TOO_SMALL";
    case gcvSTATUS_LARGER:
        return "gcvSTATUS_LARGER";
    case gcvSTATUS_SMALLER:
        return "gcvSTATUS_SMALLER";
    case gcvSTATUS_CHIP_NOT_READY:
        return "gcvSTATUS_CHIP_NOT_READY";
    case gcvSTATUS_NEED_CONVERSION:
        return "gcvSTATUS_NEED_CONVERSION";
    case gcvSTATUS_SKIP:
        return "gcvSTATUS_SKIP";
    case gcvSTATUS_DATA_TOO_LARGE:
        return "gcvSTATUS_DATA_TOO_LARGE";
    case gcvSTATUS_INVALID_CONFIG:
        return "gcvSTATUS_INVALID_CONFIG";
    case gcvSTATUS_CHANGED:
        return "gcvSTATUS_CHANGED";
    case gcvSTATUS_NOT_SUPPORT_DITHER:
        return "gcvSTATUS_NOT_SUPPORT_DITHER";

    case gcvSTATUS_INVALID_ARGUMENT:
        return "gcvSTATUS_INVALID_ARGUMENT";
    case gcvSTATUS_INVALID_OBJECT:
        return "gcvSTATUS_INVALID_OBJECT";
    case gcvSTATUS_OUT_OF_MEMORY:
        return "gcvSTATUS_OUT_OF_MEMORY";
    case gcvSTATUS_MEMORY_LOCKED:
        return "gcvSTATUS_MEMORY_LOCKED";
    case gcvSTATUS_MEMORY_UNLOCKED:
        return "gcvSTATUS_MEMORY_UNLOCKED";
    case gcvSTATUS_HEAP_CORRUPTED:
        return "gcvSTATUS_HEAP_CORRUPTED";
    case gcvSTATUS_GENERIC_IO:
        return "gcvSTATUS_GENERIC_IO";
    case gcvSTATUS_INVALID_ADDRESS:
        return "gcvSTATUS_INVALID_ADDRESS";
    case gcvSTATUS_CONTEXT_LOSSED:
        return "gcvSTATUS_CONTEXT_LOSSED";
    case gcvSTATUS_TOO_COMPLEX:
        return "gcvSTATUS_TOO_COMPLEX";
    case gcvSTATUS_BUFFER_TOO_SMALL:
        return "gcvSTATUS_BUFFER_TOO_SMALL";
    case gcvSTATUS_INTERFACE_ERROR:
        return "gcvSTATUS_INTERFACE_ERROR";
    case gcvSTATUS_NOT_SUPPORTED:
        return "gcvSTATUS_NOT_SUPPORTED";
    case gcvSTATUS_MORE_DATA:
        return "gcvSTATUS_MORE_DATA";
    case gcvSTATUS_TIMEOUT:
        return "gcvSTATUS_TIMEOUT";
    case gcvSTATUS_OUT_OF_RESOURCES:
        return "gcvSTATUS_OUT_OF_RESOURCES";
    case gcvSTATUS_INVALID_DATA:
        return "gcvSTATUS_INVALID_DATA";
    case gcvSTATUS_INVALID_MIPMAP:
        return "gcvSTATUS_INVALID_MIPMAP";
    case gcvSTATUS_NOT_FOUND:
        return "gcvSTATUS_NOT_FOUND";
    case gcvSTATUS_NOT_ALIGNED:
        return "gcvSTATUS_NOT_ALIGNED";
    case gcvSTATUS_INVALID_REQUEST:
        return "gcvSTATUS_INVALID_REQUEST";
    case gcvSTATUS_GPU_NOT_RESPONDING:
        return "gcvSTATUS_GPU_NOT_RESPONDING";
    case gcvSTATUS_TIMER_OVERFLOW:
        return "gcvSTATUS_TIMER_OVERFLOW";
    case gcvSTATUS_VERSION_MISMATCH:
        return "gcvSTATUS_VERSION_MISMATCH";
    case gcvSTATUS_LOCKED:
        return "gcvSTATUS_LOCKED";
    case gcvSTATUS_INTERRUPTED:
        return "gcvSTATUS_INTERRUPTED";
    case gcvSTATUS_DEVICE:
        return "gcvSTATUS_DEVICE";
    case gcvSTATUS_NOT_MULTI_PIPE_ALIGNED:
        return "gcvSTATUS_NOT_MULTI_PIPE_ALIGNED";

    /* Linker errors. */
    case gcvSTATUS_GLOBAL_TYPE_MISMATCH:
        return "gcvSTATUS_GLOBAL_TYPE_MISMATCH";
    case gcvSTATUS_TOO_MANY_ATTRIBUTES:
        return "gcvSTATUS_TOO_MANY_ATTRIBUTES";
    case gcvSTATUS_TOO_MANY_UNIFORMS:
        return "gcvSTATUS_TOO_MANY_UNIFORMS";
    case gcvSTATUS_TOO_MANY_VARYINGS:
        return "gcvSTATUS_TOO_MANY_VARYINGS";
    case gcvSTATUS_UNDECLARED_VARYING:
        return "gcvSTATUS_UNDECLARED_VARYING";
    case gcvSTATUS_VARYING_TYPE_MISMATCH:
        return "gcvSTATUS_VARYING_TYPE_MISMATCH";
    case gcvSTATUS_MISSING_MAIN:
        return "gcvSTATUS_MISSING_MAIN";
    case gcvSTATUS_NAME_MISMATCH:
        return "gcvSTATUS_NAME_MISMATCH";
    case gcvSTATUS_INVALID_INDEX:
        return "gcvSTATUS_INVALID_INDEX";
    case gcvSTATUS_UNIFORM_MISMATCH:
        return "gcvSTATUS_UNIFORM_MISMATCH";
    case gcvSTATUS_UNSAT_LIB_SYMBOL:
        return "gcvSTATUS_UNSAT_LIB_SYMBOL";
    case gcvSTATUS_TOO_MANY_SHADERS:
        return "gcvSTATUS_TOO_MANY_SHADERS";
    case gcvSTATUS_LINK_INVALID_SHADERS:
        return "gcvSTATUS_LINK_INVALID_SHADERS";
    case gcvSTATUS_CS_NO_WORKGROUP_SIZE:
        return "gcvSTATUS_CS_NO_WORKGROUP_SIZE";
    case gcvSTATUS_LINK_LIB_ERROR:
        return "gcvSTATUS_LINK_LIB_ERROR";
    case gcvSTATUS_SHADER_VERSION_MISMATCH:
        return "gcvSTATUS_SHADER_VERSION_MISMATCH";
    case gcvSTATUS_TOO_MANY_INSTRUCTION:
        return "gcvSTATUS_TOO_MANY_INSTRUCTION";
    case gcvSTATUS_SSBO_MISMATCH:
        return "gcvSTATUS_SSBO_MISMATCH";
    case gcvSTATUS_TOO_MANY_OUTPUT:
        return "gcvSTATUS_TOO_MANY_OUTPUT";
    case gcvSTATUS_TOO_MANY_INPUT:
        return "gcvSTATUS_TOO_MANY_INPUT";
    case gcvSTATUS_NOT_SUPPORT_CL:
        return "gcvSTATUS_NOT_SUPPORT_CL";
    case gcvSTATUS_NOT_SUPPORT_INTEGER:
        return "gcvSTATUS_NOT_SUPPORT_INTEGER";
    case gcvSTATUS_UNIFORM_TYPE_MISMATCH:
        return "gcvSTATUS_UNIFORM_TYPE_MISMATCH";
    case gcvSTATUS_MISSING_PRIMITIVE_TYPE:
        return "gcvSTATUS_MISSING_PRIMITIVE_TYPE";
    case gcvSTATUS_MISSING_OUTPUT_VERTEX_COUNT:
        return "gcvSTATUS_MISSING_OUTPUT_VERTEX_COUNT";
    case gcvSTATUS_NON_INVOCATION_ID_AS_INDEX:
        return "gcvSTATUS_NON_INVOCATION_ID_AS_INDEX";
    case gcvSTATUS_INPUT_ARRAY_SIZE_MISMATCH:
        return "gcvSTATUS_INPUT_ARRAY_SIZE_MISMATCH";
    case gcvSTATUS_OUTPUT_ARRAY_SIZE_MISMATCH:
        return "gcvSTATUS_OUTPUT_ARRAY_SIZE_MISMATCH";

    /* Compiler errors. */
    case gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR:
        return "gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR";
    case gcvSTATUS_COMPILER_FE_PARSER_ERROR:
        return "gcvSTATUS_COMPILER_FE_PARSER_ERROR";
    default:
        return "nil";
    }
}

/******************************************************************************
 *******************************Kernel Dump************************************
 ******************************************************************************/

#ifndef gcmkDUMP_STRING
# define gcmkDUMP_STRING(os, s) gcmkOUTPUT_STRING((s))
#endif

static gcmkDECLARE_MUTEX(_dumpMutex);
static gctCHAR _dumpStorage[512];

/*******************************************************************************
 **
 **  gckOS_Dump
 **
 **  Formated print string to dump pool.
 **
 **  INPUT:
 **
 **      gctCONST_STRING Format
 **          String format.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
void
gckOS_Dump(gckOS Os, gctCONST_STRING Format, ...)
{
    char buffer[256];
    gctINT len;
    gctARGUMENTS args;

    gcmkARGUMENTS_START(args, Format);
    len = gcmkVSPRINTF(buffer, gcmSIZEOF(buffer) - 2, Format, &args);
    gcmkARGUMENTS_END(args);

    if (len > 0) {
        if (buffer[len - 1] != '\n') {
            buffer[len] = '\n';
            buffer[len + 1] = '\0';
        }

        gcmkMUTEX_LOCK(_dumpMutex);
        gcmkDUMP_STRING(Os, buffer);
        gcmkMUTEX_UNLOCK(_dumpMutex);
    }
}

static void
_DumpUserString(gckOS Os, gctPOINTER UserStr, gctSIZE_T Size)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T offset = 0;
    gctSIZE_T length = 0;
    gctBOOL needCopy = gcvTRUE;
    const gctSIZE_T maxLength = gcmSIZEOF(_dumpStorage) - 1;

    gcmkVERIFY_OK(gckOS_QueryNeedCopy(Os, 0, &needCopy));

    gcmkMUTEX_LOCK(_dumpMutex);

    while (offset < Size) {
        length = maxLength < (Size - offset) ? maxLength : (Size - offset);

        /* Copy or map from user. */
        if (needCopy) {
            gcmkONERROR(gckOS_CopyFromUserData(Os, _dumpStorage, UserStr, length));
        } else {
            gctPOINTER ptr = gcvNULL;

            gcmkONERROR(gckOS_MapUserPointer(Os, UserStr, length, (gctPOINTER *)&ptr));

            gckOS_MemCopy(_dumpStorage, ptr, length);
            gckOS_UnmapUserPointer(Os, UserStr, length, ptr);
        }

        _dumpStorage[length] = '\0';
        gcmkDUMP_STRING(Os, _dumpStorage);

        UserStr = (gctUINT8_PTR)UserStr + length;
        offset += length;
    }

    gcmkDUMP_STRING(Os, "\n");

OnError:
    gcmkMUTEX_UNLOCK(_dumpMutex);
}

static void
_DumpDataBuffer(gckOS Os, gceDUMP_BUFFER_TYPE Type,
                gctPOINTER Data, gctUINT64 Address, gctSIZE_T Size)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T offset = 0;
    gctSIZE_T length = 0;
    gctBOOL needCopy = gcvTRUE;
    gctCONST_STRING dumpTag;

    char buffer[256];
    const gctSIZE_T maxLength = gcmSIZEOF(_dumpStorage);

    switch (Type) {
    case gcvDUMP_BUFFER_VERIFY:
        dumpTag = "verify";
        break;
    case gcvDUMP_BUFFER_PHYSICAL_MEMORY:
        dumpTag = "physical";
        break;
    default:
        dumpTag = "memory";
        break;
    }

    if (Type <= gcvDUMP_BUFFER_USER_TYPE_LAST)
        gcmkVERIFY_OK(gckOS_QueryNeedCopy(Os, 0, &needCopy));

    gcmkMUTEX_LOCK(_dumpMutex);

    /* Form and print the opening string. */
    if (Type == gcvDUMP_BUFFER_PHYSICAL_MEMORY) {
        gcmkSPRINTF(buffer, gcmSIZEOF(buffer) - 1,
                    "@[%s 0x%llx 0x%08X\n",
                    dumpTag, Address, (gctUINT32)Size);
    } else {
        gcmkSPRINTF(buffer, gcmSIZEOF(buffer) - 1,
                    "@[%s 0x%llx 0x%08X\n",
                    dumpTag, Address, (gctUINT32)Size);
    }

    gcmkDUMP_STRING(Os, buffer);

    while (offset < Size) {
        gctPOINTER data = gcvNULL;
        gctUINT32_PTR ptr;
        gctUINT8_PTR bytePtr;
        gctSIZE_T count, tailByteCount;

        length = maxLength < (Size - offset) ? maxLength : (Size - offset);

        count = length / 4;
        tailByteCount = length % 4;

        ptr = (gctUINT32_PTR)Data;

        if (Type <= gcvDUMP_BUFFER_USER_TYPE_LAST) {
            /* Copy or map from user. */
            if (needCopy) {
                gcmkONERROR(gckOS_CopyFromUserData(Os, _dumpStorage, Data, length));

                ptr = (gctUINT32_PTR)_dumpStorage;
            } else {
                gcmkONERROR(gckOS_MapUserPointer(Os, Data, length, (gctPOINTER *)&data));

                ptr = (gctUINT32_PTR)data;
            }
        }

        while (count >= 4) {
            gcmkSPRINTF(buffer, gcmSIZEOF(buffer) - 1,
                        "  0x%08X 0x%08X 0x%08X 0x%08X\n",
                        ptr[0], ptr[1], ptr[2], ptr[3]);

            ptr += 4;
            count -= 4;

            gcmkDUMP_STRING(Os, buffer);
        }

        switch (count) {
        case 3:
            gcmkSPRINTF(buffer, gcmSIZEOF(buffer) - 1,
                        "  0x%08X 0x%08X 0x%08X",
                        ptr[0], ptr[1], ptr[2]);
            break;
        case 2:
            gcmkSPRINTF(buffer, gcmSIZEOF(buffer) - 1,
                        "  0x%08X 0x%08X", ptr[0], ptr[1]);
            break;
        case 1:
            gcmkSPRINTF(buffer, gcmSIZEOF(buffer) - 1,
                        "  0x%08X", ptr[0]);
            break;
        }

        if (count > 0)
            gcmkDUMP_STRING(Os, buffer);

        bytePtr = (gctUINT8_PTR)(ptr + count);

        if (!count && tailByteCount) {
            /* There is an extra space for the new line. */
            gcmkDUMP_STRING(Os, " ");
        }

        switch (tailByteCount) {
        case 3:
            gcmkSPRINTF(buffer, gcmSIZEOF(buffer) - 1,
                        " 0x00%02X%02X%02X",
                        bytePtr[2], bytePtr[1], bytePtr[0]);
            break;
        case 2:
            gcmkSPRINTF(buffer, gcmSIZEOF(buffer) - 1,
                        " 0x0000%02X%02X", bytePtr[1], bytePtr[0]);
            break;
        case 1:
            gcmkSPRINTF(buffer, gcmSIZEOF(buffer) - 1,
                        " 0x000000%02X", bytePtr[0]);
            break;
        }

        if (tailByteCount)
            gcmkDUMP_STRING(Os, buffer);

        if (count || tailByteCount)
            gcmkDUMP_STRING(Os, "\n");

        if (Type <= gcvDUMP_BUFFER_USER_TYPE_LAST && !needCopy)
            gckOS_UnmapUserPointer(Os, Data, length, data);

        /* advance to next batch. */
        Data = (gctUINT8_PTR)Data + length;
        offset += length;
    }

OnError:
    gcmkSPRINTF(buffer, gcmSIZEOF(buffer) - 1, "] -- %s\n", dumpTag);
    gcmkDUMP_STRING(Os, buffer);

    gcmkMUTEX_UNLOCK(_dumpMutex);
}

/*******************************************************************************
 **
 **  gckOS_DumpBuffer
 **
 **  Print the contents of the specified buffer.
 **
 **  INPUT:
 **
 **      gckOS Os
 **          Pointer to gckOS object.
 **
 **      gceDUMP_BUFFER_TYPE Type
 **          Buffer type.
 **
 **      gctPOINTER Buffer
 **          Pointer to the buffer to print.
 **
 **      gctUINT64 Address
 **          Address.
 **
 **      gctUINT Size
 **          Size of the buffer.
 **
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
void
gckOS_DumpBuffer(gckOS Os, gceDUMP_BUFFER_TYPE Type,
                 gctPOINTER Buffer, gctUINT64 Address, gctSIZE_T Size)
{
    if (!Buffer)
        return;

    /* memory dump below. */
    if (Type >= gcvDUMP_BUFFER_TYPE_COUNT) {
        gcmkPRINT("#[ERROR: invalid buffer type]\n");
        return;
    }

    if (Type == gcvDUMP_BUFFER_USER_STRING)
        _DumpUserString(Os, Buffer, Size);
    else
        _DumpDataBuffer(Os, Type, Buffer, Address, Size);
}
