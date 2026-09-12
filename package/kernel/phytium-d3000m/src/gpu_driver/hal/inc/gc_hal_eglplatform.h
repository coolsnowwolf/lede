/****************************************************************************
*
*    Copyright (c) 2025, Phytium Technology Co., Ltd.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Phytium Corporation. This is proprietary information owned by
*    Phytium Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Phytium Corporation.
*
*****************************************************************************/


#ifndef __gc_hal_eglplatform_h_
#define __gc_hal_eglplatform_h_

#include "gc_hal_types.h"
#include "gc_hal_base.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#include <windows.h>

typedef HDC     HALNativeDisplayType;
typedef HWND    HALNativeWindowType;
typedef HBITMAP HALNativePixmapType;

typedef struct __BITFIELDINFO {
    BITMAPINFO  bmi;
    RGBQUAD     bmiColors[2];
} BITFIELDINFO;

#include "gc_hal_eglplatform_type.h"

/*******************************************************************************
 ** Display. *******************************************************************
 */

gceSTATUS
gcoOS_GetDisplay(OUT HALNativeDisplayType *Display,
                 IN gctPOINTER Context);

gceSTATUS
gcoOS_GetDisplayByIndex(IN gctINT DisplayIndex,
                        OUT HALNativeDisplayType *Display,
                        IN gctPOINTER Context);

gceSTATUS
gcoOS_GetDisplayInfo(IN HALNativeDisplayType Display,
                     OUT gctINT *Width,
                     OUT gctINT *Height,
                     OUT gctSIZE_T *Physical,
                     OUT gctINT *Stride,
                     OUT gctINT *BitsPerPixel);

gceSTATUS
gcoOS_GetDisplayInfoEx(IN HALNativeDisplayType Display,
                       IN HALNativeWindowType Window,
                       IN gctUINT DisplayInfoSize,
                       OUT halDISPLAY_INFO *DisplayInfo);

gceSTATUS
gcoOS_GetDisplayVirtual(IN HALNativeDisplayType Display,
                        OUT gctINT *Width,
                        OUT gctINT *Height);

gceSTATUS
gcoOS_GetDisplayBackbuffer(IN HALNativeDisplayType Display,
                           IN HALNativeWindowType Window,
                           OUT gctPOINTER *context,
                           OUT gcoSURF *surface,
                           OUT gctUINT *Offset,
                           OUT gctINT *X,
                           OUT gctINT *Y);

gceSTATUS
gcoOS_SetDisplayVirtual(IN HALNativeDisplayType Display,
                        IN HALNativeWindowType Window,
                        IN gctUINT Offset,
                        IN gctINT X,
                        IN gctINT Y);

gceSTATUS
gcoOS_SetDisplayVirtualEx(IN HALNativeDisplayType Display,
                          IN HALNativeWindowType Window,
                          IN gctPOINTER Context,
                          IN gcoSURF Surface,
                          IN gctUINT Offset,
                          IN gctINT X,
                          IN gctINT Y);

gceSTATUS
gcoOS_CancelDisplayBackbuffer(IN HALNativeDisplayType Display,
                              IN HALNativeWindowType Window,
                              IN gctPOINTER Context,
                              IN gcoSURF Surface,
                              IN gctUINT Offset,
                              IN gctINT X,
                              IN gctINT Y);

gceSTATUS
gcoOS_SetSwapInterval(IN HALNativeDisplayType Display, IN gctINT Interval);

gceSTATUS
gcoOS_SetSwapIntervalEx(IN HALNativeDisplayType Display,
                        IN gctINT Interval,
                        IN gctPOINTER localDisplay);

gceSTATUS
gcoOS_GetSwapInterval(IN HALNativeDisplayType Display,
                      IN gctINT_PTR Min,
                      IN gctINT_PTR Max);

gceSTATUS
gcoOS_DisplayBufferRegions(IN HALNativeDisplayType Display,
                           IN HALNativeWindowType Window,
                           IN gctINT NumRects,
                           IN gctINT_PTR Rects);

gceSTATUS
gcoOS_DestroyDisplay(IN HALNativeDisplayType Display);

gceSTATUS
gcoOS_InitLocalDisplayInfo(IN HALNativeDisplayType Display,
                           IN OUT gctPOINTER *localDisplay);

gceSTATUS
gcoOS_DeinitLocalDisplayInfo(IN HALNativeDisplayType Display,
                             IN OUT gctPOINTER *localDisplay);

gceSTATUS
gcoOS_GetDisplayInfoEx2(IN HALNativeDisplayType Display,
                        IN HALNativeWindowType  Window,
                        IN gctPOINTER localDisplay,
                        IN gctUINT DisplayInfoSize,
                        OUT halDISPLAY_INFO *DisplayInfo);

gceSTATUS
gcoOS_GetDisplayBackbufferEx(IN HALNativeDisplayType Display,
                             IN HALNativeWindowType Window,
                             IN gctPOINTER localDisplay,
                             OUT gctPOINTER *context,
                             OUT gcoSURF *surface,
                             OUT gctUINT *Offset,
                             OUT gctINT *X,
                             OUT gctINT *Y);

gceSTATUS
gcoOS_IsValidDisplay(IN HALNativeDisplayType Display);

gceSTATUS
gcoOS_GetNativeVisualId(IN HALNativeDisplayType Display,
                        OUT gctINT *nativeVisualId);

gctBOOL
gcoOS_SynchronousFlip(IN HALNativeDisplayType Display);

/*******************************************************************************
 ** Windows. ********************************************************************
 */

gceSTATUS
gcoOS_CreateWindow(IN HALNativeDisplayType Display,
                   IN gctINT X,
                   IN gctINT Y,
                   IN gctINT Width,
                   IN gctINT Height,
                   OUT HALNativeWindowType *Window);

gceSTATUS
gcoOS_GetWindowInfo(IN HALNativeDisplayType Display,
                    IN HALNativeWindowType Window,
                    OUT gctINT *X,
                    OUT gctINT *Y,
                    OUT gctINT *Width,
                    OUT gctINT *Height,
                    OUT gctINT *BitsPerPixel,
                    OUT gctUINT *Offset);

gceSTATUS
gcoOS_DestroyWindow(IN HALNativeDisplayType Display,
                    IN HALNativeWindowType Window);

gceSTATUS
gcoOS_DrawImage(IN HALNativeDisplayType Display,
                IN HALNativeWindowType Window,
                IN gctINT Left,
                IN gctINT Top,
                IN gctINT Right,
                IN gctINT Bottom,
                IN gctINT Width,
                IN gctINT Height,
                IN gctINT BitsPerPixel,
                IN gctPOINTER Bits);

gceSTATUS
gcoOS_GetImage(IN HALNativeWindowType Window,
               IN gctINT Left,
               IN gctINT Top,
               IN gctINT Right,
               IN gctINT Bottom,
               OUT gctINT *BitsPerPixel,
               OUT gctPOINTER *Bits);

gceSTATUS
gcoOS_GetWindowInfoEx(IN HALNativeDisplayType Display,
                      IN HALNativeWindowType Window,
                      OUT gctINT *X,
                      OUT gctINT *Y,
                      OUT gctINT *Width,
                      OUT gctINT *Height,
                      OUT gctINT *BitsPerPixel,
                      OUT gctUINT *Offset,
                      OUT gceSURF_FORMAT *Format,
                      OUT gceSURF_TYPE *Type);

gceSTATUS
gcoOS_DrawImageEx(IN HALNativeDisplayType Display,
                  IN HALNativeWindowType Window,
                  IN gctINT Left,
                  IN gctINT Top,
                  IN gctINT Right,
                  IN gctINT Bottom,
                  IN gctINT Width,
                  IN gctINT Height,
                  IN gctINT BitsPerPixel,
                  IN gctPOINTER Bits,
                  IN gceSURF_FORMAT Format);

/*
 * Possiable types:
 *   gcvSURF_BITMAP
 *   gcvSURF_RENDER_TARGET
 *   gcvSURF_RENDER_TARGET_NO_COMPRESSION
 *   gcvSURF_RENDER_TARGET_NO_TILE_STATUS
 */
gceSTATUS
gcoOS_SetWindowFormat(IN HALNativeDisplayType Display,
                      IN HALNativeWindowType Window,
                      IN gceSURF_TYPE Type,
                      IN gceSURF_FORMAT Format);

/*******************************************************************************
 ** Pixmaps. *******************************************************************
 */

gceSTATUS
gcoOS_CreatePixmap(IN HALNativeDisplayType Display,
                   IN gctINT Width,
                   IN gctINT Height,
                   IN gctINT BitsPerPixel,
                   OUT HALNativePixmapType *Pixmap);

gceSTATUS
gcoOS_GetPixmapInfo(IN HALNativeDisplayType Display,
                    IN HALNativePixmapType Pixmap,
                    OUT gctINT *Width,
                    OUT gctINT *Height,
                    OUT gctINT *BitsPerPixel,
                    OUT gctINT *Stride,
                    OUT gctPOINTER *Bits);

gceSTATUS
gcoOS_DrawPixmap(IN HALNativeDisplayType Display,
                 IN HALNativePixmapType Pixmap,
                 IN gctINT Left,
                 IN gctINT Top,
                 IN gctINT Right,
                 IN gctINT Bottom,
                 IN gctINT Width,
                 IN gctINT Height,
                 IN gctINT BitsPerPixel,
                 IN gctPOINTER Bits);

gceSTATUS
gcoOS_DestroyPixmap(IN HALNativeDisplayType Display,
                    IN HALNativePixmapType Pixmap);

gceSTATUS
gcoOS_GetPixmapInfoEx(IN HALNativeDisplayType Display,
                      IN HALNativePixmapType Pixmap,
                      OUT gctINT *Width,
                      OUT gctINT *Height,
                      OUT gctINT *BitsPerPixel,
                      OUT gctINT *Stride,
                      OUT gctPOINTER *Bits,
                      OUT gceSURF_FORMAT *Format);

gceSTATUS
gcoOS_CopyPixmapBits(IN HALNativeDisplayType Display,
                     IN HALNativePixmapType Pixmap,
                     IN gctUINT DstWidth,
                     IN gctUINT DstHeight,
                     IN gctINT DstStride,
                     IN gceSURF_FORMAT DstFormat,
                     OUT gctPOINTER DstBits);

/*******************************************************************************
 ** OS relative. ****************************************************************
 */
gceSTATUS
gcoOS_LoadEGLLibrary(OUT gctHANDLE *Handle);

gceSTATUS
gcoOS_FreeEGLLibrary(IN gctHANDLE Handle);

gceSTATUS
gcoOS_ShowWindow(IN HALNativeDisplayType Display, IN HALNativeWindowType Window);

gceSTATUS
gcoOS_HideWindow(IN HALNativeDisplayType Display, IN HALNativeWindowType Window);

gceSTATUS
gcoOS_SetWindowTitle(IN HALNativeDisplayType Display,
                     IN HALNativeWindowType Window,
                     IN gctCONST_STRING Title);

gceSTATUS
gcoOS_CapturePointer(IN HALNativeDisplayType Display, IN HALNativeWindowType Window);

gceSTATUS
gcoOS_GetEvent(IN HALNativeDisplayType Display,
               IN HALNativeWindowType Window,
               OUT halEvent *Event);

gceSTATUS
gcoOS_CreateClientBuffer(IN gctINT Width,
                         IN gctINT Height,
                         IN gctINT Format,
                         IN gctINT Type,
                         OUT gctPOINTER *ClientBuffer);

gceSTATUS
gcoOS_GetClientBufferInfo(IN gctPOINTER  ClientBuffer,
                          OUT gctINT *Width,
                          OUT gctINT *Height,
                          OUT gctINT *Stride,
                          OUT gctPOINTER *Bits);

gceSTATUS
gcoOS_DestroyClientBuffer(IN gctPOINTER ClientBuffer);

gceSTATUS
gcoOS_DestroyContext(IN gctPOINTER Display, IN gctPOINTER Context);

gceSTATUS
gcoOS_CreateContext(IN gctPOINTER LocalDisplay, IN gctPOINTER Context);

gceSTATUS
gcoOS_MakeCurrent(IN gctPOINTER LocalDisplay,
                  IN HALNativeWindowType DrawDrawable,
                  IN HALNativeWindowType ReadDrawable,
                  IN gctPOINTER Context,
                  IN gcoSURF ResolveTarget);

gceSTATUS
gcoOS_CreateDrawable(IN gctPOINTER LocalDisplay, IN HALNativeWindowType Drawable);

gceSTATUS
gcoOS_DestroyDrawable(IN gctPOINTER LocalDisplay, IN HALNativeWindowType Drawable);
gceSTATUS
gcoOS_SwapBuffers(IN gctPOINTER LocalDisplay,
                  IN HALNativeWindowType Drawable,
                  IN gcoSURF RenderTarget,
                  IN gcoSURF ResolveTarget,
                  IN gctPOINTER ResolveBits,
                  OUT gctUINT *Width,
                  OUT gctUINT *Height);

gceSTATUS
gcoOS_ResizeWindow(IN gctPOINTER localDisplay,
                   IN HALNativeWindowType Drawable,
                   IN gctUINT Width,
                   IN gctUINT Height);

gceSTATUS
gcoOS_RSForSwap(IN gctPOINTER localDisplay,
                IN HALNativeWindowType Drawable,
                IN gctPOINTER resolve);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_eglplatform_h_ */
