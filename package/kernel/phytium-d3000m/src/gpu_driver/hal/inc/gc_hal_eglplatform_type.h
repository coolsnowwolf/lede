/****************************************************************************
*
*    Copyright 2012 - 2024 Phytium Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL PHYTIUM AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#ifndef __gc_hal_eglplatform_type_h_
#define __gc_hal_eglplatform_type_h_

#ifdef __cplusplus
extern "C" {
#endif

/* Structure that defined keyboard mapping. */
typedef struct _halKeyMap {
    /* Normal key. */
    halKeys normal;

    /* Extended key. */
    halKeys extended;
} halKeyMap;

/* Event structure. */
typedef struct _halEvent {
    /* Event type. */
    halEventType type;

    /* Event data union. */
    union _halEventData {
        /* Event data for keyboard. */
        struct _halKeyboard {
            /* Scancode. */
            halKeys scancode;

            /* ASCII characte of the key pressed. */
            char    key;

            /* Flag whether the key was pressed (1) or released (0). */
            char    pressed;
        } keyboard;

        /* Event data for pointer. */
        struct _halPointer {
            /* Current pointer coordinate. */
            int     x;
            int     y;
        } pointer;

        /* Event data for mouse buttons. */
        struct _halButton {
            /* Left button state. */
            int     left;

            /* Middle button state. */
            int     middle;

            /* Right button state. */
            int     right;

            /* Current pointer coordinate. */
            int     x;
            int     y;
        } button;
    } data;
} halEvent;

/* VFK_DISPLAY_INFO structure defining information returned by
 * vdkGetDisplayInfoEx.
 */
typedef struct _halDISPLAY_INFO {
    /* The size of the display in pixels. */
    int                 width;
    int                 height;

    /* The stride of the dispay. -1 is returned if the stride is not known
     * for the specified display.
     */
    int                 stride;

    /* The color depth of the display in bits per pixel. */
    int                 bitsPerPixel;

    /* The logical pointer to the display memory buffer. NULL is returned
     * if the pointer is not known for the specified display.
     */
    void                *logical;

    /* The physical address of the display memory buffer. ~0 is returned
     * if the address is not known for the specified display.
     */
    unsigned long       physical;

    /* Can be wraped as surface. */
    int                 wrapFB;

    /* FB_MULTI_BUFFER support */
    int                 multiBuffer;
    int                 backBufferY;

    /* Tiled buffer / tile status support. */
    int                 tiledBuffer;
    int                 tileStatus;
    int                 compression;

    /* The color info of the display. */
    unsigned int        alphaLength;
    unsigned int        alphaOffset;
    unsigned int        redLength;
    unsigned int        redOffset;
    unsigned int        greenLength;
    unsigned int        greenOffset;
    unsigned int        blueLength;
    unsigned int        blueOffset;

    /* Display flip support. */
    int flip;
} halDISPLAY_INFO;

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_eglplatform_type_h_ */
