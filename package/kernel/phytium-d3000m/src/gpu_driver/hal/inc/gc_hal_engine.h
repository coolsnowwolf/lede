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


#ifndef __gc_hal_engine_h_
#define __gc_hal_engine_h_

#include "gc_hal_types.h"
#include "gc_hal_enum.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _gcsSURF_RESOLVE_ARGS {
    gceHAL_ARG_VERSION version;

    union _gcsSURF_RESOLVE_ARGS_UNION {
        struct _gcsSURF_RESOLVE_ARG_v2 {
            gctBOOL   yInverted;
            gctBOOL   directCopy;
            gctBOOL   resample;
            gctBOOL   bUploadTex;     /* used for upload tex.*/
            gctBOOL   bSwap;          /* used for swap.*/
            gctBOOL   visualizeDepth; /* convert depth to visible color */
            gcsPOINT  srcOrigin;
            gcsPOINT  dstOrigin;
            gcsPOINT  rectSize;
            gctUINT   numSlices;
            gceENGINE engine;        /* 3DBlit engine */
            gctBOOL   gpuOnly;       /* need only try HW path.*/
            gctBOOL   cpuOnly;       /* need only try SW path */

            gctBOOL   dump;          /* need dump for verify */
            gctBOOL   srcSwizzle;    /* src surface format swizzle information */
            gctBOOL   dstSwizzle;    /* dst surface format swizzle information */
            gctBOOL   srcCompressed; /* src compressed format*/
            gctBOOL   dstCompressed; /* dst compressed format*/
            gctUINT   blitToSelf;

            gctUINT   rotate;        /* rotate of 3DBlt engine */
            gctBOOL   onlyDepth;
            gctBOOL   onlyStencil;
        } v2;
    } uArgs;
} gcsSURF_RESOLVE_ARGS;

typedef struct _gscBUFFER_VIEW {
    gctUINT32 cmd;
} gcsBUFFER_VIEW, *gcsBUFFER_VIEW_PTR;

typedef struct _gcsIMAGE_VIEW {
    gctUINT32 cmd;
} gcsIMAGE_VIEW, *gcsIMAGE_VIEW_PTR;

#if gcdENABLE_3D
/******************************************************************************
 ***************************** Object Declarations ****************************
 ******************************************************************************/

typedef struct _gcoSTREAM            *gcoSTREAM;
typedef struct _gcoVERTEX            *gcoVERTEX;
typedef struct _gcoTEXTURE           *gcoTEXTURE;
typedef struct _gcoINDEX             *gcoINDEX;
typedef struct _gcsVERTEX_ATTRIBUTES *gcsVERTEX_ATTRIBUTES_PTR;
typedef struct _gcoVERTEXARRAY       *gcoVERTEXARRAY;
typedef struct _gcoBUFOBJ            *gcoBUFOBJ;

# define gcdATTRIBUTE_COUNT              32
# define gcdVERTEXARRAY_POOL_CAPACITY    32

# define gcdSTREAM_POOL_SIZE             128
# define gcdSTREAM_GROUP_SIZE            16
# define gcdSTREAM_SIGNAL_NUM  \
    ((gcdSTREAM_POOL_SIZE + gcdSTREAM_GROUP_SIZE - 1) / gcdSTREAM_GROUP_SIZE)

# define gcvPORGRAM_STAGE_GPIPE (gcvPROGRAM_STAGE_VERTEX_BIT | \
                                 gcvPROGRAM_STAGE_TCS_BIT    | \
                                 gcvPROGRAM_STAGE_TES_BIT    | \
                                 gcvPROGRAM_STAGE_GEOMETRY_BIT)

# define gcdSHADER_COMPONENTS_MAX_SIZE   128

/******************************************************************************
 ******************************** gcoHAL Object *******************************
 ******************************************************************************/

gceSTATUS
gcoHAL_QueryShaderCaps(IN gcoHAL   Hal,
                       OUT gctUINT *UnifiedUniforms,
                       OUT gctUINT *VertUniforms,
                       OUT gctUINT *FragUniforms,
                       OUT gctUINT *Varyings,
                       OUT gctUINT *ShaderCoreCount,
                       OUT gctUINT *ThreadCount,
                       OUT gctUINT *VertInstructionCount,
                       OUT gctUINT *FragInstructionCount);

gceSTATUS
gcoHAL_QuerySamplerBase(IN gcoHAL Hal,
                        OUT gctUINT32 *VertexCount,
                        OUT gctINT_PTR VertexBase,
                        OUT gctUINT32 *FragmentCount,
                        OUT gctINT_PTR FragmentBase);

gceSTATUS
gcoHAL_QueryUniformBase(IN gcoHAL Hal, OUT gctUINT32 *VertexBase,
                        OUT gctUINT32 *FragmentBase);

gceSTATUS
gcoHAL_QueryTextureCaps(IN gcoHAL Hal,
                        OUT gctUINT *MaxWidth,
                        OUT gctUINT *MaxHeight,
                        OUT gctUINT *MaxDepth,
                        OUT gctBOOL *Cubic,
                        OUT gctBOOL *NonPowerOfTwo,
                        OUT gctUINT *VertexSamplers,
                        OUT gctUINT *PixelSamplers);

gceSTATUS
gcoHAL_QueryTextureMaxAniso(IN gcoHAL Hal, OUT gctUINT *MaxAnisoValue);

gceSTATUS
gcoHAL_QueryStreamCaps(IN gcoHAL Hal,
                       OUT gctUINT32 *MaxAttributes,
                       OUT gctUINT32 *MaxStreamStride,
                       OUT gctUINT32 *NumberOfStreams,
                       OUT gctUINT32 *Alignment,
                       OUT gctUINT32 *MaxAttribOffset);

/******************************************************************************
 ******************************** gcoSURF Object ******************************
 ******************************************************************************/

/*----------------------------------------------------------------------------*/
/*--------------------------------- gcoSURF 3D --------------------------------*/
typedef struct _gcsSURF_BLIT_ARGS {
    gcoSURF     srcSurface;
    gctINT      srcX, srcY, srcZ;
    gctINT      srcWidth, srcHeight, srcDepth;
    gcoSURF     dstSurface;
    gctINT      dstX, dstY, dstZ;
    gctINT      dstWidth, dstHeight, dstDepth;
    gctBOOL     xReverse;
    gctBOOL     yReverse;
    gctBOOL     scissorTest;
    gcsRECT     scissor;
    gctUINT     flags;
    gctUINT     srcNumSlice, dstNumSlice;
    gctBOOL     needDecode;
    gctBOOL     readSwap;
    gctBOOL     writeSwap;
} gcsSURF_BLIT_ARGS;

typedef struct _gcsSURF_CLEAR_ARGS {
    /*
     ** Color to fill the color portion of the framebuffer when clear
     ** is called.
     */
    struct {
        gcuVALUE r;
        gcuVALUE g;
        gcuVALUE b;
        gcuVALUE a;
        /* Color has multiple value type so we must specify it. */
        gceVALUE_TYPE valueType;
    } color;

    gcuVALUE        depth;
    gctUINT         stencil;

    gctUINT8        stencilMask; /* stencil bit-wise mask */
    gctBOOL         depthMask;   /* Depth Write Mask */
    gctUINT8        colorMask;   /* 4-bit channel Mask: ABGR:MSB->LSB */
    gcsRECT_PTR     clearRect;   /* NULL means full clear */
    gceCLEAR        flags;       /* clear flags */

    gctUINT32       offset; /* Offset in surface to cube/array/3D, obsolete in v2 version */
    gctUINT32       depthNum;

    gctBOOL         framebufferSRGB; /* Should always true for OES and default false for OGL */

} gcsSURF_CLEAR_ARGS, *gcsSURF_CLEAR_ARGS_PTR;

typedef struct _gscSURF_BLITDRAW_BLIT {
    gcoSURF             srcSurface;
    gcoSURF             dstSurface;
    gcsRECT             srcRect;
    gcsRECT             dstRect;
    gceTEXTURE_FILTER   filterMode;
    gctBOOL             xReverse;
    gctBOOL             yReverse;
    gctBOOL             scissorEnabled;
    gcsRECT             scissor;
    gctBOOL             sRGBEnabled;
} gscSURF_BLITDRAW_BLIT;

typedef gceSTATUS (*gctSPLIT_DRAW_FUNC_PTR)(IN gctPOINTER gc,
                                            IN gctPOINTER instantDraw,
                                            IN gctPOINTER splitDrawInfo);

typedef struct _gcsSPLIT_DRAW_INFO {
    gceSPLIT_DRAW_TYPE     splitDrawType;
    gctSPLIT_DRAW_FUNC_PTR splitDrawFunc;

    union _gcsSPLIT_DRAW_UNION {
        /* This path will split many draw.*/
        struct __gcsSPLIT_DRAW_INFO_TCS {
            gctPOINTER      indexPtr;
            gctUINT         indexPerPatch;
        } info_tcs;

        /* This path split into two draw at most.
         ** es11 path follow the old code, es30 path
         ** add more info parameter to record
         */
        struct __gcsSPLIT_DRAW_INFO_INDEX_FETCH {
            gctSIZE_T       instanceCount;
            gctSIZE_T       splitCount;
            gcePRIMITIVE    splitPrimMode;
            gctSIZE_T       splitPrimCount;
        } info_index_fetch;

        struct __gcsSPLIT_DRAW_INFO_LINES_PATCH {
            gctUINT         splitPrimCount;
        } info_lines;
    } u;
} gcsSPLIT_DRAW_INFO, *gcsSPLIT_DRAW_INFO_PTR;

typedef struct _gscSURF_BLITDRAW_ARGS {
    /* always the fist member */
    gceHAL_ARG_VERSION version;

    union _gcsSURF_BLITDRAW_ARGS_UNION {
        struct _gscSURF_BLITDRAW_ARG_v1 {
            /* Whether it's clear or blit operation, can be extended. */
            gceBLIT_TYPE type;

            union _gscSURF_BLITDRAW_UNION {
                gscSURF_BLITDRAW_BLIT blit;

                struct _gscSURF_BLITDRAW_CLEAR {
                    gcsSURF_CLEAR_ARGS clearArgs;
                    gcoSURF            rtSurface;
                    gcoSURF            dsSurface;
                } clear;
            } u;
        } v1;
    } uArgs;
} gcsSURF_BLITDRAW_ARGS;

typedef struct _gcsSURF_BLITBLT_ARGS {
    gctCONST_POINTER    buf;
    gceSURF_FORMAT      format;
    gctUINT32           stride;
    gcoSURF             dstSurf;
    gcsPOINT            dstOrigin;
    gcsPOINT            rectSize;
    gctUINT32           dstOffset;
} gcsSURF_BLITBLT_ARGS;

/* CPU Blit with format (including linear <-> tile) conversion*/
gceSTATUS
gcoSURF_BlitCPU(gcsSURF_BLIT_ARGS *args);

/* Copy a rectangular area with format conversion. */
gceSTATUS
gcoSURF_CopyPixels(IN gcsSURF_VIEW *SrcView,
                   IN gcsSURF_VIEW *DstView,
                   IN gcsSURF_RESOLVE_ARGS *Args);

/* Clear surface function. */
gceSTATUS
gcoSURF_Clear(IN gcsSURF_VIEW *SurfView,
              IN gcsSURF_CLEAR_ARGS_PTR ClearArgs);

/* Preserve pixels from source. */
gceSTATUS
gcoSURF_Preserve(IN gcoSURF SrcSurf,
                 IN gcoSURF DstSurf,
                 IN gcsRECT_PTR MaskRect);

/* TO BE REMOVED */
gceSTATUS
depr_gcoSURF_Resolve(IN gcoSURF SrcSurface,
                     IN gcoSURF DestSurface,
                     IN gctADDRESS DestAddress,
                     IN gctPOINTER DestBits,
                     IN gctINT DestStride,
                     IN gceSURF_TYPE DestType,
                     IN gceSURF_FORMAT DestFormat,
                     IN gctUINT DestWidth,
                     IN gctUINT DestHeight);

gceSTATUS
depr_gcoSURF_ResolveRect(IN gcoSURF SrcSurface,
                         IN gcoSURF DstSurface,
                         IN gctADDRESS DstAddress,
                         IN gctPOINTER DstBits,
                         IN gctINT DstStride,
                         IN gceSURF_TYPE DstType,
                         IN gceSURF_FORMAT DstFormat,
                         IN gctUINT DstWidth,
                         IN gctUINT DstHeight,
                         IN gcsPOINT_PTR SrcOrigin,
                         IN gcsPOINT_PTR gcoSURF,
                         IN gcsPOINT_PTR RectSize);

/* Resample surface. */
gceSTATUS
gcoSURF_Resample(IN gcoSURF SrcSurf,
                 IN gcoSURF DstSurf,
                 IN gctBOOL sRGBDecode);

/* Resolve rectangular area of a surface. */
gceSTATUS
gcoSURF_ResolveRect(IN gcsSURF_VIEW *SrcView,
                    IN gcsSURF_VIEW *DstView,
                    IN gcsSURF_RESOLVE_ARGS *Args);

gceSTATUS
gcoSURF_GetResolveAlignment(IN gcoSURF Surface,
                            OUT gctUINT *originX,
                            OUT gctUINT *originY,
                            OUT gctUINT *sizeX,
                            OUT gctUINT *sizeY);

gceSTATUS
gcoSURF_IsHWResolveable(IN gcoSURF SrcSurf,
                        IN gcoSURF DstSurf,
                        IN gcsPOINT_PTR SrcOrigin,
                        IN gcsPOINT_PTR DstOrigin,
                        IN gcsPOINT_PTR RectSize);

/* Set surface resolvability. */
gceSTATUS
gcoSURF_SetResolvability(IN gcoSURF Surface, IN gctBOOL Resolvable);

gceSTATUS
gcoSURF_IsRenderable(IN gcoSURF Surface);

gceSTATUS
gcoSURF_IsFormatRenderableAsRT(IN gcoSURF Surface);

gceSTATUS
gcoBUFOBJ_SetBindingTarget(IN gcoHAL Hal, IN gctUINT TargetIndex, IN gcoBUFOBJ BufObj);

gceSTATUS
gcoBUFOBJ_GetFence(IN gcoBUFOBJ BufObj, IN gceFENCE_TYPE Type);

gceSTATUS
gcoBUFOBJ_WaitFence(IN gcoBUFOBJ BufObj, IN gceFENCE_TYPE Type);

gceSTATUS
gcoBUFOBJ_IsFenceEnabled(IN gcoBUFOBJ BufObj);

gceSTATUS
gcoSURF_GetFence(IN gcoSURF Surface, IN gceFENCE_TYPE Type);

gceSTATUS
gcoSURF_WaitFence(IN gcoSURF Surface);

gceSTATUS
gcoSTREAM_GetFence(IN gcoSTREAM stream);

gceSTATUS
gcoSTREAM_WaitFence(IN gcoSTREAM stream);

gceSTATUS
gcoINDEX_GetFence(IN gcoINDEX Index);

gceSTATUS
gcoINDEX_WaitFence(IN gcoINDEX Index, IN gceFENCE_TYPE Type);

gceSTATUS
gcoSURF_DrawBlit(gcsSURF_VIEW *SrcView,
                 gcsSURF_VIEW *DstView,
                 gscSURF_BLITDRAW_BLIT *Args);

gceSTATUS
gcoSURF_DrawBlitDepth(gcsSURF_VIEW *SrcView,
                      gcsSURF_VIEW *DstView,
                      gscSURF_BLITDRAW_BLIT *Args);

gceSTATUS
gcoSURF_ComputeBlit(gcsSURF_VIEW *SrcView,
                    gcsSURF_VIEW *DstView,
                    gscSURF_BLITDRAW_BLIT *Args);

/******************************************************************************
 ******************************* gcoINDEX Object ******************************
 ******************************************************************************/

/* Construct a new gcoINDEX object. */
gceSTATUS
gcoINDEX_Construct(IN gcoHAL Hal, OUT gcoINDEX *Index);

/* Destroy a gcoINDEX object. */
gceSTATUS
gcoINDEX_Destroy(IN gcoINDEX Index);

/* Lock index in memory. */
gceSTATUS
gcoINDEX_Lock(IN gcoINDEX Index,
              OUT gctADDRESS *Address,
              OUT gctPOINTER *Memory);

/* Unlock index that was previously locked with gcoINDEX_Lock. */
gceSTATUS
gcoINDEX_Unlock(IN gcoINDEX Index);

/* Upload index data into the memory. */
gceSTATUS
gcoINDEX_Load(IN gcoINDEX Index,
              IN gceINDEX_TYPE IndexType,
              IN gctUINT32 IndexCount,
              IN gctPOINTER IndexBuffer);

/* Bind an index object to the hardware. */
gceSTATUS
gcoINDEX_Bind(IN gcoINDEX Index, IN gceINDEX_TYPE Type);

/* Bind an index object to the hardware. */
gceSTATUS
gcoINDEX_BindOffset(IN gcoINDEX Index,
                    IN gceINDEX_TYPE Type,
                    IN gctSIZE_T Offset);

/* Free existing index buffer. */
gceSTATUS
gcoINDEX_Free(IN gcoINDEX Index);

/* Upload data into an index buffer. */
gceSTATUS
gcoINDEX_Upload(IN gcoINDEX Index,
                IN gctCONST_POINTER Buffer,
                IN gctSIZE_T Bytes);

/* Upload data into an index buffer starting at an offset. */
gceSTATUS
gcoINDEX_UploadOffset(IN gcoINDEX Index,
                      IN gctSIZE_T Offset,
                      IN gctCONST_POINTER Buffer,
                      IN gctSIZE_T Bytes);

/*Merge index2 to index1 from 0, index2 must subset of inex1*/
gceSTATUS
gcoINDEX_Merge(IN gcoINDEX Index1, IN gcoINDEX Index2);

/*check if index buffer is enough for this draw*/
gctBOOL
gcoINDEX_CheckRange(IN gcoINDEX Index,
                    IN gceINDEX_TYPE Type,
                    IN gctINT Count,
                    IN gctUINT32 Indices);

/* Query the index capabilities. */
gceSTATUS
gcoINDEX_QueryCaps(OUT gctBOOL *Index8,
                   OUT gctBOOL *Index16,
                   OUT gctBOOL *Index32,
                   OUT gctUINT64 *MaxIndex);

/* Determine the index range in the current index buffer. */
gceSTATUS
gcoINDEX_GetIndexRange(IN gcoINDEX Index,
                       IN gceINDEX_TYPE Type,
                       IN gctSIZE_T Offset,
                       IN gctUINT32 Count,
                       OUT gctUINT32 *MinimumIndex,
                       OUT gctUINT32 *MaximumIndex);

/* Dynamic buffer management. */
gceSTATUS
gcoINDEX_SetDynamic(IN gcoINDEX Index,
                    IN gctSIZE_T Bytes,
                    IN gctUINT Buffers);

gceSTATUS
gcoCLHardware_Construct(void);
/******************************************************************************
 ********************************* gco3D Object *******************************
 ******************************************************************************/

/* Construct a new gco3D object. */
gceSTATUS
gco3D_Construct(IN gcoHAL Hal, IN gctBOOL Robust, OUT gco3D *Engine);

/* Destroy an gco3D object. */
gceSTATUS
gco3D_Destroy(IN gco3D Engine);

/* Set 3D API type. */
gceSTATUS
gco3D_SetAPI(IN gco3D Engine, IN gceAPI ApiType);

/* Get 3D API type. */
gceSTATUS
gco3D_GetAPI(IN gco3D Engine, OUT gceAPI *ApiType);

gceSTATUS
gco3D_SetTarget(IN gco3D Engine,
                IN gctUINT32 TargetIndex,
                IN gcsSURF_VIEW *SurfView,
                IN gctUINT32 LayerIndex);

gceSTATUS
gco3D_UnsetTarget(IN gco3D Engine, IN gctUINT32 TargetIndex, IN gcoSURF Surface);

gceSTATUS
gco3D_SetPSOutputMapping(IN gco3D Engine, IN gctINT32 *psOutputMapping);

gceSTATUS
gco3D_SetDualSourcePSOutputExchange(IN gco3D Engine, IN gctBOOL ExchangeDualSourcePsOutput);

gceSTATUS
gco3D_SetRenderLayered(IN gco3D Engine, IN gctBOOL Enable, IN gctUINT MaxLayers);

gceSTATUS
gco3D_SetShaderLayered(IN gco3D Engine, IN gctBOOL Enable);

gceSTATUS
gco3D_IsProgramSwitched(IN gco3D Engine);

/* Set depth buffer. */
gceSTATUS
gco3D_SetDepth(IN gco3D Engine, IN gcsSURF_VIEW *SurfView);

/* Unset depth buffer. */
gceSTATUS
gco3D_UnsetDepth(IN gco3D Engine, IN gcoSURF Surface);

/* Set clip control;. */
gceSTATUS
gco3D_SetClipControl(IN gco3D Engine,
                     IN gctBOOL DefaultClipOrigin,
                     IN gctBOOL DefaultClipDepthMode);

/* Set viewports. */
gceSTATUS
gco3D_SetViewport(IN gco3D Engine,
                  IN gctUINT  Index,
                  IN gctFLOAT Left,
                  IN gctFLOAT Top,
                  IN gctFLOAT Right,
                  IN gctFLOAT Bottom);

/* Set scissors. */
gceSTATUS
gco3D_SetScissors(IN gco3D Engine,
                  IN gctUINT  Index,
                  IN gctINT32 Left,
                  IN gctINT32 Top,
                  IN gctINT32 Right,
                  IN gctINT32 Bottom);

/* Set clear color. */
gceSTATUS
gco3D_SetClearColor(IN gco3D Engine,
                    IN gctUINT8 Red,
                    IN gctUINT8 Green,
                    IN gctUINT8 Blue,
                    IN gctUINT8 Alpha);

/* Set fixed point clear color. */
gceSTATUS
gco3D_SetClearColorX(IN gco3D Engine,
                     IN gctFIXED_POINT Red,
                     IN gctFIXED_POINT Green,
                     IN gctFIXED_POINT Blue,
                     IN gctFIXED_POINT Alpha);

/* Set floating point clear color. */
gceSTATUS
gco3D_SetClearColorF(IN gco3D Engine,
                     IN gctFLOAT Red,
                     IN gctFLOAT Green,
                     IN gctFLOAT Blue,
                     IN gctFLOAT Alpha);

/* Set fixed point clear depth. */
gceSTATUS
gco3D_SetClearDepthX(IN gco3D Engine, IN gctFIXED_POINT Depth);

/* Set floating point clear depth. */
gceSTATUS
gco3D_SetClearDepthF(IN gco3D Engine, IN gctFLOAT Depth);

/* Set clear stencil. */
gceSTATUS
gco3D_SetClearStencil(IN gco3D Engine, IN gctUINT32 Stencil);

/* Set shading mode. */
gceSTATUS
gco3D_SetShading(IN gco3D Engine, IN gceSHADING Shading);

/* Set blending mode. */
gceSTATUS
gco3D_EnableBlending(IN gco3D Engine, IN gctBOOL Enable);

/* Set blending function. */
gceSTATUS
gco3D_SetBlendFunction(IN gco3D Engine,
                       IN gceBLEND_UNIT Unit,
                       IN gceBLEND_FUNCTION FunctionRGB,
                       IN gceBLEND_FUNCTION FunctionAlpha);

/* Set blending mode. */
gceSTATUS
gco3D_SetBlendMode(IN gco3D Engine, IN gceBLEND_MODE ModeRGB, IN gceBLEND_MODE ModeAlpha);

/* Set blending mode for separate rt control */
gceSTATUS
gco3D_EnableBlendingIndexed(IN gco3D Engine, IN gctUINT Index, IN gctBOOL Enable);

/* Set blending function for separate rt control */
gceSTATUS
gco3D_SetBlendFunctionIndexed(IN gco3D Engine,
                              IN gctUINT Index,
                              IN gceBLEND_UNIT Unit,
                              IN gceBLEND_FUNCTION FunctionRGB,
                              IN gceBLEND_FUNCTION FunctionAlpha);

/* Set blending mode for separate rt control*/
gceSTATUS
gco3D_SetBlendModeIndexed(IN gco3D Engine,
                          IN gctUINT Index,
                          IN gceBLEND_MODE ModeRGB,
                          IN gceBLEND_MODE ModeAlpha);

/* Set blending color. */
gceSTATUS
gco3D_SetBlendColor(IN gco3D Engine,
                    IN gctUINT Red,
                    IN gctUINT Green,
                    IN gctUINT Blue,
                    IN gctUINT Alpha);

/* Set fixed point blending color. */
gceSTATUS
gco3D_SetBlendColorX(IN gco3D Engine,
                     IN gctFIXED_POINT Red,
                     IN gctFIXED_POINT Green,
                     IN gctFIXED_POINT Blue,
                     IN gctFIXED_POINT Alpha);

/* Set floating point blending color. */
gceSTATUS
gco3D_SetBlendColorF(IN gco3D Engine,
                     IN gctFLOAT Red,
                     IN gctFLOAT Green,
                     IN gctFLOAT Blue,
                     IN gctFLOAT Alpha);

/* Set culling mode. */
gceSTATUS
gco3D_SetCulling(IN gco3D Engine, IN gceCULL Mode);

/* Set point size */
gceSTATUS
gco3D_SetPointSize(IN gco3D Engine, IN gctFLOAT Size);

/* Enable point size */
gceSTATUS
gco3D_SetPointSizeEnable(IN gco3D Engine, IN gctBOOL Enable);

/* Set point sprite */
gceSTATUS
gco3D_SetPointSprite(IN gco3D Engine, IN gctBOOL Enable);

/* Enable/Disable primitive-id. */
gceSTATUS
gco3D_SetPrimitiveIdEnable(IN gco3D Engine, IN gctBOOL Enable);

/* Set fill mode. */
gceSTATUS
gco3D_SetFill(IN gco3D Engine, IN gceFILL Mode, IN gceFILL backMode);

/* Set front face mode. */
gceSTATUS
gco3D_SetFrontFace(IN gco3D Engine, IN gceFRONTFACE Mode);

/* Set line prim type is back face. */
gceSTATUS
gco3D_SetLineFaceIsBack(IN gco3D Engine, IN gctBOOL IsBack);

/* Set depth compare mode. */
gceSTATUS
gco3D_SetDepthCompare(IN gco3D Engine, IN gceCOMPARE Compare);

/* Enable depth writing. */
gceSTATUS
gco3D_EnableDepthWrite(IN gco3D Engine, IN gctBOOL Enable);

/* Enable depth clamp. */
gceSTATUS
gco3D_EnableDepthClamp(IN gco3D Engine, IN gctBOOL Enable);

/* Set depth mode. */
gceSTATUS
gco3D_SetDepthMode(IN gco3D Engine, IN gceDEPTH_MODE Mode);

/* Set depth range. */
gceSTATUS
gco3D_SetDepthRangeX(IN gco3D Engine,
                     IN gceDEPTH_MODE Mode,
                     IN gctFIXED_POINT Near,
                     IN gctFIXED_POINT Far);

/* Set depth range. */
gceSTATUS
gco3D_SetDepthRangeF(IN gco3D Engine,  IN gceDEPTH_MODE Mode, IN gctUINT Index,
                     IN gctFLOAT Near, IN gctFLOAT Far);

/* Set last pixel enable */
gceSTATUS
gco3D_SetLastPixelEnable(IN gco3D Engine, IN gctBOOL Enable);

/* Set depth Bias and Scale */
gceSTATUS
gco3D_SetDepthScaleBiasX(IN gco3D Engine,
                         IN gctFIXED_POINT DepthScale,
                         IN gctFIXED_POINT DepthBias);

gceSTATUS
gco3D_SetDepthScaleBiasF(IN gco3D Engine, IN gctFLOAT DepthScale, IN gctFLOAT DepthBias, IN gctFLOAT DepthClamp);

gceSTATUS
gco3D_SetPolygonOffsetMode(IN gco3D Engine, IN gcePOLYGONOFFSETMODE Mode);

/* Set depth near and far clipping plane. */
gceSTATUS
gco3D_SetDepthPlaneF(IN gco3D Engine, IN gctFLOAT Near, IN gctFLOAT Far);

/* Enable or disable dithering. */
gceSTATUS
gco3D_EnableDither(IN gco3D Engine, IN gctBOOL Enable);

/* Enable or disable robust. */
gceSTATUS
gco3D_EnableRobust(IN gco3D Engine, IN gctBOOL Enable);

/* Set color write enable bits. */
gceSTATUS
gco3D_SetColorWrite(IN gco3D Engine, IN gctUINT8 Enable);

/* Set color write enable bits for separate rt control */
gceSTATUS
gco3D_SetColorWriteIndexed(IN gco3D Engine, IN gctUINT Index, IN gctUINT8 Enable);

/* Enable or disable early depth. */
gceSTATUS
gco3D_SetEarlyDepth(IN gco3D Engine, IN gctBOOL Enable);

/* Deprecated: Enable or disable all early depth operations. */
gceSTATUS
gco3D_SetAllEarlyDepthModes(IN gco3D Engine, IN gctBOOL Disable);

gceSTATUS
gco3D_SetEarlyDepthFromAPP(IN gco3D Engine, IN gctBOOL EarlyDepthFromAPP);

gceSTATUS
gco3D_SetRADepthWrite(IN gco3D Engine, IN gctBOOL Disable,
                      IN gctBOOL psReadZ, IN gctBOOL psReadW);

gceSTATUS
gco3D_SetPatchVertices(IN gco3D Engine, IN gctINT PatchVertices);

/* Switch dynamic early mode */
gceSTATUS
gco3D_SwitchDynamicEarlyDepthMode(IN gco3D Engine);

/* Set dynamic early mode */
gceSTATUS
gco3D_DisableDynamicEarlyDepthMode(IN gco3D Engine, IN gctBOOL Disable);

/* Enable or disable depth-only mode. */
gceSTATUS
gco3D_SetDepthOnly(IN gco3D Engine, IN gctBOOL Enable);

typedef struct _gcsSTENCIL_INFO *gcsSTENCIL_INFO_PTR;
typedef struct _gcsSTENCIL_INFO {
    gceSTENCIL_MODE         mode;

    gctUINT8                maskFront;
    gctUINT8                maskBack;
    gctUINT8                writeMaskFront;
    gctUINT8                writeMaskBack;

    gctUINT8                referenceFront;

    gceCOMPARE              compareFront;
    gceSTENCIL_OPERATION    passFront;
    gceSTENCIL_OPERATION    failFront;
    gceSTENCIL_OPERATION    depthFailFront;

    gctUINT8                referenceBack;
    gceCOMPARE              compareBack;
    gceSTENCIL_OPERATION    passBack;
    gceSTENCIL_OPERATION    failBack;
    gceSTENCIL_OPERATION    depthFailBack;
} gcsSTENCIL_INFO;

/* Set stencil mode. */
gceSTATUS
gco3D_SetStencilMode(IN gco3D Engine, IN gceSTENCIL_MODE Mode);

/* Set stencil mask. */
gceSTATUS
gco3D_SetStencilMask(IN gco3D Engine, IN gctUINT8 Mask);

/* Set stencil back mask. */
gceSTATUS
gco3D_SetStencilMaskBack(IN gco3D Engine, IN gctUINT8 Mask);

/* Set stencil write mask. */
gceSTATUS
gco3D_SetStencilWriteMask(IN gco3D Engine, IN gctUINT8 Mask);

/* Set stencil back write mask. */
gceSTATUS
gco3D_SetStencilWriteMaskBack(IN gco3D Engine, IN gctUINT8 Mask);

/* Set stencil reference. */
gceSTATUS
gco3D_SetStencilReference(IN gco3D Engine,
                          IN gctUINT8 Reference,
                          IN gctBOOL Front);

/* Set stencil compare. */
gceSTATUS
gco3D_SetStencilCompare(IN gco3D Engine,
                        IN gceSTENCIL_WHERE Where,
                        IN gceCOMPARE Compare);

/* Set stencil operation on pass. */
gceSTATUS
gco3D_SetStencilPass(IN gco3D Engine,
                     IN gceSTENCIL_WHERE Where,
                     IN gceSTENCIL_OPERATION Operation);

/* Set stencil operation on fail. */
gceSTATUS
gco3D_SetStencilFail(IN gco3D Engine,
                     IN gceSTENCIL_WHERE Where,
                     IN gceSTENCIL_OPERATION Operation);

/* Set stencil operation on depth fail. */
gceSTATUS
gco3D_SetStencilDepthFail(IN gco3D Engine,
                          IN gceSTENCIL_WHERE Where,
                          IN gceSTENCIL_OPERATION Operation);

/* Set all stencil states in one blow. */
gceSTATUS
gco3D_SetStencilAll(IN gco3D Engine, IN gcsSTENCIL_INFO_PTR Info);

typedef struct _gcsALPHA_INFO *gcsALPHA_INFO_PTR;
typedef struct _gcsALPHA_INFO {
    /* Alpha test states. */
    gctBOOL                 test;
    gceCOMPARE              compare;
    gctUINT8                reference;
    gctFLOAT                floatReference;

    /* Alpha blending states. */
    gctBOOL                 blend[gcdMAX_DRAW_BUFFERS];

    gceBLEND_FUNCTION       srcFuncColor[gcdMAX_DRAW_BUFFERS];
    gceBLEND_FUNCTION       srcFuncAlpha[gcdMAX_DRAW_BUFFERS];
    gceBLEND_FUNCTION       trgFuncColor[gcdMAX_DRAW_BUFFERS];
    gceBLEND_FUNCTION       trgFuncAlpha[gcdMAX_DRAW_BUFFERS];

    gceBLEND_MODE           modeColor[gcdMAX_DRAW_BUFFERS];
    gceBLEND_MODE           modeAlpha[gcdMAX_DRAW_BUFFERS];

    gctUINT32               factorR;
    gctUINT32               factorG;
    gctUINT32               factorB;
    gctUINT32               factorA;
    gctUINT32               color;

    gctBOOL                 anyBlendEnabled;
} gcsALPHA_INFO;

/* Enable or disable alpha test. */
gceSTATUS
gco3D_SetAlphaTest(IN gco3D Engine, IN gctBOOL Enable);

/* Set alpha test compare. */
gceSTATUS
gco3D_SetAlphaCompare(IN gco3D Engine, IN gceCOMPARE Compare);

/* Set alpha test reference in unsigned integer. */
gceSTATUS
gco3D_SetAlphaReference(IN gco3D Engine, IN gctUINT8 Reference,
                        IN gctFLOAT FloatReference);

/* Set alpha test reference in fixed point. */
gceSTATUS
gco3D_SetAlphaReferenceX(IN gco3D Engine, IN gctFIXED_POINT Reference);

/* Set alpha test reference in floating point. */
gceSTATUS
gco3D_SetAlphaReferenceF(IN gco3D Engine, IN gctFLOAT Reference);

#if gcdALPHA_KILL_IN_SHADER
gceSTATUS
gco3D_SetAlphaKill(IN gco3D Engine, IN gctBOOL AlphaKill,
                   IN gctBOOL ColorKill);
# endif

/* Enable/Disable anti-alias line. */
gceSTATUS
gco3D_SetAntiAliasLine(IN gco3D Engine, IN gctBOOL Enable);

/* Set texture slot for anti-alias line. */
gceSTATUS
gco3D_SetAALineTexSlot(IN gco3D Engine, IN gctUINT TexSlot);

/* Set anti-alias line width scale. */
gceSTATUS
gco3D_SetAALineWidth(IN gco3D Engine, IN gctFLOAT Width);

/* Draw a number of primitives. */
gceSTATUS
gco3D_DrawPrimitives(IN gco3D Engine,
                     IN gcePRIMITIVE Type,
                     IN gctSIZE_T StartVertex,
                     IN gctSIZE_T PrimitiveCount);

gceSTATUS
gco3D_DrawIndirectPrimitives(IN gco3D Engine,
                             IN gcePRIMITIVE Type,
                             IN gctBOOL DrawIndex,
                             IN gctINT BaseOffset,
                             IN gcoBUFOBJ BufObj);

gceSTATUS
gco3D_MultiDrawIndirectPrimitives(IN gco3D Engine,
                                  IN gcePRIMITIVE Type,
                                  IN gctBOOL DrawIndex,
                                  IN gctBOOL useParameterBuf,
                                  IN gctINT parameterOffset,
                                  IN gctINT indirectOffset,
                                  IN gctINT DrawCount,
                                  IN gctINT Stride,
                                  IN gcoBUFOBJ parameterBufObj,
                                  IN gcoBUFOBJ indirectBufObj);

gceSTATUS
gco3D_DrawInstancedPrimitives(IN gco3D Engine,
                              IN gcePRIMITIVE Type,
                              IN gctBOOL DrawIndex,
                              IN gctINT StartVertex,
                              IN gctSIZE_T StartIndex,
                              IN gctSIZE_T StartInstance,
                              IN gctSIZE_T PrimitiveCount,
                              IN gctSIZE_T VertexCount,
                              IN gctSIZE_T InstanceCount);

gceSTATUS
gco3D_DrawAutoPrimitives(IN gco3D Engine,
                         IN gcePRIMITIVE Type,
                         IN gctUINT StreamId,
                         IN gctSIZE_T InstanceCount);

gceSTATUS
gco3D_DrawNullPrimitives(IN gco3D Engine);

gceSTATUS
gco3D_DrawPrimitivesCount(IN gco3D Engine,
                          IN gcePRIMITIVE Type,
                          IN gctINT *StartVertex,
                          IN gctSIZE_T *VertexCount,
                          IN gctSIZE_T PrimitiveCount);

/* Draw a number of primitives using offsets. */
gceSTATUS
gco3D_DrawPrimitivesOffset(IN gco3D Engine,
                           IN gcePRIMITIVE Type,
                           IN gctINT32 StartOffset,
                           IN gctSIZE_T PrimitiveCount);

/* Draw a number of indexed primitives. */
gceSTATUS
gco3D_DrawIndexedPrimitives(IN gco3D Engine,
                            IN gcePRIMITIVE Type,
                            IN gctSIZE_T BaseVertex,
                            IN gctSIZE_T StartIndex,
                            IN gctSIZE_T PrimitiveCount);

/* Draw a number of indexed primitives using offsets. */
gceSTATUS
gco3D_DrawIndexedPrimitivesOffset(IN gco3D Engine,
                                  IN gcePRIMITIVE Type,
                                  IN gctINT32 BaseOffset,
                                  IN gctINT32 StartOffset,
                                  IN gctSIZE_T PrimitiveCount);

/* Draw a element from pattern */
gceSTATUS
gco3D_DrawPattern(IN gco3D Engine, IN gcsFAST_FLUSH_PTR FastFlushInfo);

/* Enable or disable anti-aliasing. */
gceSTATUS
gco3D_SetAntiAlias(IN gco3D Engine, IN gctBOOL Enable);

/* Set msaa samples */
gceSTATUS
gco3D_SetSamples(IN gco3D Engine, IN gctUINT32 Samples);

/* Write data into the command buffer. */
gceSTATUS
gco3D_WriteBuffer(IN gco3D Engine, IN gctCONST_POINTER Data,
                  IN gctSIZE_T Bytes, IN gctBOOL Aligned);

/* Send sempahore and stall until sempahore is signalled. */
gceSTATUS
gco3D_Semaphore(IN gco3D Engine, IN gceWHERE From,
                IN gceWHERE To, IN gceHOW How);

/* Sync for multi gpu core */
gceSTATUS
gco3D_MultiGPUSync(void);

/* Explicitly flush pipeline */
gceSTATUS
gco3D_FlushPipe(IN gco3D Engine);

/* Explicitly flush shader L1 cache */
gceSTATUS
gco3D_FlushSHL1Cache(IN gco3D Engine);

/* Set the subpixels center. */
gceSTATUS
gco3D_SetCentroids(IN gco3D Engine, IN gctUINT32 Index, IN gctPOINTER Centroids);

/* query msaa sample coordinates */
gceSTATUS
gco3D_GetSampleCoords(IN gco3D Engine,
                      IN gctUINT32 SampleIndex,
                      IN gctBOOL yInverted,
                      OUT gctFLOAT_PTR Coords);

gceSTATUS
gco3D_SetLogicOp(IN gco3D Engine, IN gctUINT8 Rop);

gceSTATUS
gco3D_SetFramebufferSRGB(IN gco3D Engine, IN gctBOOL framebufferSRGB);

gceSTATUS
gco3D_SetQuery(IN gco3D Engine,
               IN gctADDRESS QueryHeader,
               IN gceQueryType Type,
               IN gctBOOL Enable,
               IN gctUINT32 Index);

gceSTATUS
gco3D_GetQuery(IN gco3D Engine,
               IN gceQueryType Type,
               IN gcsSURF_NODE_PTR Node,
               IN gctUINT32 Size,
               IN gctPOINTER Locked,
               IN gctUINT32 IndexedId,
               OUT gctINT32 *Index);

gceSTATUS
gco3D_SetXfbHeader(IN gco3D Engine, IN gctUINT32 Index, IN gctADDRESS Physical);

gceSTATUS
gco3D_SetDrawXfbHeader(IN gco3D Engine, IN gctADDRESS Physical);

gceSTATUS
gco3D_SetXfbBuffer(IN gco3D Engine,
                   IN gctUINT32 Index,
                   IN gctADDRESS BufferAddr,
                   IN gctUINT32 BufferStride,
                   IN gctUINT32 BufferSize);

gceSTATUS
gco3D_SetXfbCmd(IN gco3D Engine, IN gceXfbCmd Cmd);

gceSTATUS
gco3D_SetRasterDiscard(IN gco3D Engine, IN gctBOOL Enable);

gceSTATUS
gco3D_SetColorOutCount(IN gco3D Engine, IN gctUINT32 ColorOutCount);

gceSTATUS
gco3D_SetColorCacheMode(IN gco3D Engine);

gceSTATUS
gco3D_Set3DEngine(IN gco3D Engine);

gceSTATUS
gco3D_UnSet3DEngine(IN gco3D Engine);

gceSTATUS
gco3D_Get3DEngine(OUT gco3D *Engine);

gceSTATUS
gco3D_QueryReset(IN gco3D Engine, OUT gctBOOL_PTR Innocent);

/* OCL thread walker information. */
typedef struct _gcsTHREAD_WALKER_INFO *gcsTHREAD_WALKER_INFO_PTR;
typedef struct _gcsTHREAD_WALKER_INFO {
    gctUINT32   dimensions;
    gctUINT32   traverseOrder;
    gctUINT32   enableSwathX;
    gctUINT32   enableSwathY;
    gctUINT32   enableSwathZ;
    gctUINT32   swathSizeX;
    gctUINT32   swathSizeY;
    gctUINT32   swathSizeZ;
    gctUINT32   valueOrder;

    gctUINT32   globalSizeX;
    gctUINT32   globalOffsetX;
    gctUINT32   globalSizeY;
    gctUINT32   globalOffsetY;
    gctUINT32   globalSizeZ;
    gctUINT32   globalOffsetZ;

    gctUINT32   globalScaleX;
    gctUINT32   globalScaleY;
    gctUINT32   globalScaleZ;

    gctUINT32   workGroupSizeX;
    gctUINT32   workGroupCountX;
    gctUINT32   workGroupSizeY;
    gctUINT32   workGroupCountY;
    gctUINT32   workGroupSizeZ;
    gctUINT32   workGroupCountZ;

    gctUINT32   threadAllocation;
    gctBOOL     barrierUsed;
    gctUINT32   memoryAccessFlag; /* same as gceMEMORY_ACCESS_FLAG */
    gctBOOL     indirect;
    gctUINT32   groupNumberUniformIdx;
    gctADDRESS  baseAddress;
    gctBOOL     bDual16;
    gctBOOL     bUsedTex;
    gctUINT32   coreCount;
    gctUINT32   coreIDs[gcdMAX_3DGPU_COUNT];
} gcsTHREAD_WALKER_INFO;

#if gcdENABLE_3D && gcdUSE_VX
/* VX thread walker parameters. */
typedef struct _gcsVX_THREAD_WALKER_PARAMETERS *gcsVX_THREAD_WALKER_PARAMETERS_PTR;

typedef struct _gcsVX_THREAD_WALKER_PARAMETERS {
    gctUINT32   valueOrder;
    gctUINT32   workDim;

    gctUINT32   workGroupSizeX;
    gctUINT32   workGroupCountX;

    gctUINT32   workGroupSizeY;
    gctUINT32   workGroupCountY;

    gctUINT32   globalOffsetX;
    gctUINT32   globalScaleX;

    gctUINT32   globalOffsetY;
    gctUINT32   globalScaleY;

#if gcdVX_OPTIMIZER > 1
    gctBOOL tileMode;
#  endif
} gcsVX_THREAD_WALKER_PARAMETERS;

typedef struct _gcsVX_IMAGE_INFO *gcsVX_IMAGE_INFO_PTR;

typedef struct _gcsVX_IMAGE_INFO {
    gctUINT32       format;
    gctUINT32       rect[4];
    gctSIZE_T       width;
    gctSIZE_T       height;

    /*arraySize, sliceSize is for imageArray / image3D */
    gctSIZE_T       arraySize;
    gctUINT64       sliceSize;

    gctUINT32       bpp;
    gctUINT32       planes;
    gctUINT32       componentCount;
    gctBOOL         isFloat;

    gctUINT32       uPixels;
    gctUINT32       vPixels;
    gceSURF_FORMAT  internalFormat;
    gctUINT32       border;

    /*vx_imagepatch_addressing_t == (gctUINT32 * 8) */
    void            *base_addr[3];

    gctUINT64       stride[3];

    gctPOINTER      logicals[3];
    gctADDRESS      physicals[3];
    gctUINT32       bytes;

    gcsSURF_NODE_PTR nodes[3];

    gctBOOL         isVXC;
#if gcdVX_OPTIMIZER
    gctUINT32       uniformData[3][4];
#  endif
    /* the uniform data type of save nbg */
    gctUINT32       uniformSaveDataType;
} gcsVX_IMAGE_INFO;
typedef struct _gcsVX_DISTRIBUTION_INFO *gcsVX_DISTRIBUTION_INFO_PTR;

typedef struct _gcsVX_DISTRIBUTION_INFO {
    gctUINT32       logical;
    gctADDRESS      physical;
    gctUINT32       bytes;

    gcsSURF_NODE_PTR node;
} gcsVX_DISTRIBUTION_INFO;
# endif

/* Start OCL thread walker. */
gceSTATUS
gco3D_InvokeThreadWalker(IN gco3D Engine, IN gcsTHREAD_WALKER_INFO_PTR Info);

gceSTATUS
gco3D_GetClosestRenderFormat(IN gco3D Engine,
                             IN gceSURF_FORMAT InFormat,
                             OUT gceSURF_FORMAT *OutFormat);

/* Set w clip and w plane limit value. */
gceSTATUS
gco3D_SetWClipEnable(IN gco3D Engine, IN gctBOOL Enable);

gceSTATUS
gco3D_GetWClipEnable(IN gco3D Engine, OUT gctBOOL *Enable);

gceSTATUS
gco3D_SetWPlaneLimitF(IN gco3D Engine, IN gctFLOAT Value);

gceSTATUS
gco3D_SetWPlaneLimitX(IN gco3D Engine, IN gctFIXED_POINT Value);

gceSTATUS
gco3D_SetWPlaneLimit(IN gco3D Engine, IN gctFLOAT Value);

gceSTATUS
gco3D_PrimitiveRestart(IN gco3D Engine, IN gctBOOL PrimitiveRestart, IN gctBOOL PrimitiveRestartFixedIndex);

gceSTATUS
gco3D_SetMultiDrawNumber(IN gco3D Engine, IN gctBOOL isCpuLoop, IN gctINT32 currentDrawID);

gceSTATUS
gco3D_LoadProgram(IN gco3D Engine,
                  IN gcePROGRAM_STAGE_BIT StageBits,
                  IN gctPOINTER ProgramState);

gceSTATUS
gco3D_EnableAlphaToCoverage(IN gco3D Engine, IN gctBOOL Enable);

gceSTATUS
gco3D_EnableSampleCoverage(IN gco3D Engine, IN gctBOOL Enable);

gceSTATUS
gco3D_SetSampleCoverageValue(IN gco3D Engine,
                             IN gctFLOAT CoverageValue,
                             IN gctBOOL Invert);

gceSTATUS
gco3D_EnableSampleMask(IN gco3D Engine, IN gctBOOL Enable);

gceSTATUS
gco3D_SetSampleMask(IN gco3D Engine, IN gctUINT32 SampleMask);

gceSTATUS
gco3D_EnableSampleShading(IN gco3D Engine, IN gctBOOL Enable);

gceSTATUS
gco3D_SetMinSampleShadingValue(IN gco3D Engine, IN gctFLOAT MinSampleShadingValue);

gceSTATUS
gco3D_SetSampleShading(IN gco3D Engine,
                       IN gctBOOL Enable,
                       IN gctBOOL IsSampleIn,
                       IN gctFLOAT SampleShadingValue);

gceSTATUS
gco3D_EnableSampleMaskOut(IN gco3D Engine, IN gctBOOL Enable, IN gctINT SampleMaskLoc);

/*----------------------------------------------------------------------------*/
/*-------------------------- gco3D Fragment Processor ------------------------*/

/* Set the fragment processor configuration. */
gceSTATUS
gco3D_SetFragmentConfiguration(IN gco3D Engine,
                               IN gctBOOL ColorFromStream,
                               IN gctBOOL EnableFog,
                               IN gctBOOL EnableSmoothPoint,
                               IN gctUINT32 ClipPlanes);

/* Enable/disable texture stage operation. */
gceSTATUS
gco3D_EnableTextureStage(IN gco3D Engine, IN gctINT Stage, IN gctBOOL Enable);

/* Program the channel enable masks for the color texture function. */
gceSTATUS
gco3D_SetTextureColorMask(IN gco3D Engine,
                          IN gctINT Stage,
                          IN gctBOOL ColorEnabled,
                          IN gctBOOL AlphaEnabled);

/* Program the channel enable masks for the alpha texture function. */
gceSTATUS
gco3D_SetTextureAlphaMask(IN gco3D Engine,
                          IN gctINT Stage,
                          IN gctBOOL ColorEnabled,
                          IN gctBOOL AlphaEnabled);

/* Program the constant fragment color. */
gceSTATUS
gco3D_SetFragmentColorX(IN gco3D Engine,
                        IN gctFIXED_POINT Red,
                        IN gctFIXED_POINT Green,
                        IN gctFIXED_POINT Blue,
                        IN gctFIXED_POINT Alpha);

gceSTATUS
gco3D_SetFragmentColorF(IN gco3D Engine,
                        IN gctFLOAT Red,
                        IN gctFLOAT Green,
                        IN gctFLOAT Blue,
                        IN gctFLOAT Alpha);

/* Program the constant fog color. */
gceSTATUS
gco3D_SetFogColorX(IN gco3D Engine,
                   IN gctFIXED_POINT Red,
                   IN gctFIXED_POINT Green,
                   IN gctFIXED_POINT Blue,
                   IN gctFIXED_POINT Alpha);

gceSTATUS
gco3D_SetFogColorF(IN gco3D Engine,
                   IN gctFLOAT Red,
                   IN gctFLOAT Green,
                   IN gctFLOAT Blue,
                   IN gctFLOAT Alpha);

/* Program the constant texture color. */
gceSTATUS
gco3D_SetTetxureColorX(IN gco3D Engine,
                       IN gctINT Stage,
                       IN gctFIXED_POINT Red,
                       IN gctFIXED_POINT Green,
                       IN gctFIXED_POINT Blue,
                       IN gctFIXED_POINT Alpha);

gceSTATUS
gco3D_SetTetxureColorF(IN gco3D Engine,
                       IN gctINT Stage,
                       IN gctFLOAT Red,
                       IN gctFLOAT Green,
                       IN gctFLOAT Blue,
                       IN gctFLOAT Alpha);

/* Configure color texture function. */
gceSTATUS
gco3D_SetColorTextureFunction(IN gco3D Engine,
                              IN gctINT Stage,
                              IN gceTEXTURE_FUNCTION Function,
                              IN gceTEXTURE_SOURCE Source0,
                              IN gceTEXTURE_CHANNEL Channel0,
                              IN gceTEXTURE_SOURCE Source1,
                              IN gceTEXTURE_CHANNEL Channel1,
                              IN gceTEXTURE_SOURCE Source2,
                              IN gceTEXTURE_CHANNEL Channel2,
                              IN gctINT Scale);

/* Configure alpha texture function. */
gceSTATUS
gco3D_SetAlphaTextureFunction(IN gco3D Engine,
                              IN gctINT Stage,
                              IN gceTEXTURE_FUNCTION Function,
                              IN gceTEXTURE_SOURCE Source0,
                              IN gceTEXTURE_CHANNEL Channel0,
                              IN gceTEXTURE_SOURCE Source1,
                              IN gceTEXTURE_CHANNEL Channel1,
                              IN gceTEXTURE_SOURCE Source2,
                              IN gceTEXTURE_CHANNEL Channel2,
                              IN gctINT Scale);

/******************************************************************************
 ****************************** gcoTEXTURE Object *****************************
 ******************************************************************************/

typedef struct _gcsTEXTURE {
    /* Addressing modes. */
    gceTEXTURE_ADDRESSING       s;
    gceTEXTURE_ADDRESSING       t;
    gceTEXTURE_ADDRESSING       r;

    gceTEXTURE_SWIZZLE          swizzle[gcvTEXTURE_COMPONENT_NUM];

    /* Border color. */
    gctUINT8                    border[gcvTEXTURE_COMPONENT_NUM];

    /* Filters. */
    gceTEXTURE_FILTER           minFilter;
    gceTEXTURE_FILTER           magFilter;
    gceTEXTURE_FILTER           mipFilter;
    gctUINT                     anisoFilter;

    /* Level of detail. */
    gctFLOAT                    lodBias;
    gctFLOAT                    lodMin;
    gctFLOAT                    lodMax;

    /* base/max level */
    gctINT32                    baseLevel;
    gctINT32                    maxLevel;

    /* depth texture comparison */
    gceTEXTURE_COMPARE_MODE     compareMode;
    gceCOMPARE                  compareFunc;

    gceTEXTURE_DS_TEX_MODE      dsTextureMode;

    gceTEXTURE_DS_MODE          dsMode;

    /* sRGB decode */
    gceTEXTURE_SRGBDECODE       sRGB;

    gcuVALUE                    borderColor[4];
    gctBOOL                     descDirty;

    /* texture stage */
    gctINT                      stage;
} gcsTEXTURE, *gcsTEXTURE_PTR;

typedef struct _gcsTEXTURE_BINDTEXTS_ARGS {
    /* must be the first member */
    gceHAL_ARG_VERSION          version;

} gcsTEXTURE_BINDTEXTS_ARGS;

/* Construct a new gcoTEXTURE object. */
gceSTATUS
gcoTEXTURE_Construct(IN gcoHAL Hal, OUT gcoTEXTURE *Texture);

/* Construct a new gcoTEXTURE object with type information. */
gceSTATUS
gcoTEXTURE_ConstructEx(IN gcoHAL Hal, IN gceTEXTURE_TYPE Type, OUT gcoTEXTURE *Texture);

/* Construct a new sized gcoTEXTURE object. */
gceSTATUS
gcoTEXTURE_ConstructSized(IN gcoHAL Hal,
                          IN gceSURF_FORMAT Format,
                          IN gceTILING Tiling,
                          IN gctUINT Width,
                          IN gctUINT Height,
                          IN gctUINT Depth,
                          IN gctUINT Faces,
                          IN gctUINT MipMapCount,
                          IN gcePOOL Pool,
                          OUT gcoTEXTURE *Texture);

/* Destroy an gcoTEXTURE object. */
gceSTATUS
gcoTEXTURE_Destroy(IN gcoTEXTURE Texture);

/* Check src and dst format support of 3D Blit. */
gceSTATUS
gcoTEXTURE_CheckHWSupportedImage(
    IN gceSURF_FORMAT srcFormat,
    IN gceSURF_FORMAT dstFormat
);

/* Upload data to an gcoTEXTURE object. */
gceSTATUS
gcoTEXTURE_Upload(IN gcoTEXTURE Texture,
                  IN gctINT MipMap,
                  IN gceTEXTURE_FACE Face,
                  IN gctSIZE_T Width,
                  IN gctSIZE_T Height,
                  IN gctUINT Slice,
                  IN gctCONST_POINTER Memory,
                  IN gctSIZE_T Stride,
                  IN gceSURF_FORMAT Format,
                  IN gceSURF_COLOR_SPACE SrcColorSpace);

/* Upload data to an gcoTEXTURE object. */
gceSTATUS
gcoTEXTURE_UploadSub(IN gcoTEXTURE Texture,
                     IN gctINT MipMap,
                     IN gceTEXTURE_FACE Face,
                     IN gctSIZE_T X,
                     IN gctSIZE_T Y,
                     IN gctSIZE_T Width,
                     IN gctSIZE_T Height,
                     IN gctUINT Slice,
                     IN gctCONST_POINTER Memory,
                     IN gctSIZE_T Stride,
                     IN gceSURF_FORMAT Format,
                     IN gceSURF_COLOR_SPACE SrcColorSpace,
                     IN gctADDRESS PhysicalAddress);

/* Upload YUV data to an gcoTEXTURE object. */
gceSTATUS
gcoTEXTURE_UploadYUV(IN gcoTEXTURE Texture,
                     IN gceTEXTURE_FACE Face,
                     IN gctUINT Width,
                     IN gctUINT Height,
                     IN gctUINT Slice,
                     IN gctPOINTER Memory[3],
                     IN gctINT Stride[3],
                     IN gceSURF_FORMAT Format);

/* Upload compressed data to an gcoTEXTURE object. */
gceSTATUS
gcoTEXTURE_UploadCompressed(IN gcoTEXTURE Texture,
                            IN gctINT MipMap,
                            IN gceTEXTURE_FACE Face,
                            IN gctSIZE_T Width,
                            IN gctSIZE_T Height,
                            IN gctUINT Slice,
                            IN gctCONST_POINTER Memory,
                            IN gctSIZE_T Bytes);

gceSTATUS
gcoTEXTURE_GetCompressedTexture(IN gcoTEXTURE Texture,
                                IN gctINT MipMap,
                                IN gceTEXTURE_FACE Face,
                                IN gctSIZE_T Xoffset,
                                IN gctSIZE_T Yoffset,
                                IN gctSIZE_T Width,
                                IN gctSIZE_T Height,
                                IN gctUINT Slice,
                                IN gctPOINTER Memory);

/* Upload compressed sub data to an gcoTEXTURE object. */
gceSTATUS
gcoTEXTURE_UploadCompressedSub(IN gcoTEXTURE Texture,
                               IN gctINT MipMap,
                               IN gceTEXTURE_FACE Face,
                               IN gctSIZE_T XOffset,
                               IN gctSIZE_T YOffset,
                               IN gctSIZE_T Width,
                               IN gctSIZE_T Height,
                               IN gctUINT Slice,
                               IN gctCONST_POINTER Memory,
                               IN gctSIZE_T Size);

/* Get gcoSURF object for a mipmap level. */
gceSTATUS
gcoTEXTURE_GetMipMap(IN gcoTEXTURE Texture, IN gctUINT MipMap, OUT gcoSURF *Surface);

/* Get gcoSURF object for a mipmap level and face offset. */
gceSTATUS
gcoTEXTURE_GetMipMapFace(IN gcoTEXTURE Texture,
                         IN gctUINT MipMap,
                         IN gceTEXTURE_FACE Face,
                         OUT gcoSURF *Surface,
                         OUT gctSIZE_T_PTR Offset);

gceSTATUS
gcoTEXTURE_GetMipMapSlice(IN gcoTEXTURE Texture,
                          IN gctUINT MipMap,
                          IN gctUINT Slice,
                          OUT gcoSURF *Surface,
                          OUT gctSIZE_T_PTR Offset);

gceSTATUS
gcoTEXTURE_AddMipMap(IN gcoTEXTURE Texture,
                     IN gctINT Level,
                     IN gctINT InternalFormat,
                     IN gceSURF_FORMAT Format,
                     IN gctSIZE_T Width,
                     IN gctSIZE_T Height,
                     IN gctSIZE_T Depth,
                     IN gctUINT Faces,
                     IN gcePOOL Pool,
                     IN gctBOOL Filterable,
                     OUT gcoSURF *Surface);

gceSTATUS
gcoTEXTURE_AddMipMapEx(IN gcoTEXTURE Texture,
                       IN gctINT Level,
                       IN gctINT InternalFormat,
                       IN gceSURF_FORMAT Format,
                       IN gctSIZE_T Width,
                       IN gctSIZE_T Height,
                       IN gctSIZE_T Depth,
                       IN gctUINT Faces,
                       IN gcePOOL Pool,
                       IN gctUINT32 Samples,
                       IN gctBOOL Protected,
                       IN gctBOOL Filterable,
                       IN gctBOOL isView,
                       OUT gcoSURF *Surface);

gceSTATUS
gcoTEXTURE_AddMipMapFromClient(IN gcoTEXTURE Texture,
                               IN gctINT Level,
                               IN gcoSURF Surface);

gceSTATUS
gcoTEXTURE_AddMipMapFromSurface(IN gcoTEXTURE Texture,
                                IN gctINT Level,
                                IN gcoSURF Surface);

gceSTATUS
gcoTEXTURE_LockMipMap(IN gcoTEXTURE Texture,
                      IN gctUINT MipMap,
                      OPTIONAL OUT gctADDRESS *Address,
                      OPTIONAL OUT gctPOINTER *Memory);

gceSTATUS
gcoTEXTURE_SetEndianHint(IN gcoTEXTURE Texture, IN gceENDIAN_HINT EndianHint);

gceSTATUS
gcoTEXTURE_Disable(IN gcoHAL Hal, IN gctINT Sampler, IN gctINT TextureIdx, IN gctBOOL DefaultInteger);

gceSTATUS
gcoTEXTURE_Clear(IN gcoTEXTURE Texture, IN gctINT MipMap);

gceSTATUS
gcoTEXTURE_Flush(IN gcoTEXTURE Texture);

gceSTATUS
gcoTEXTURE_FlushVS(IN gcoTEXTURE Texture);

gceSTATUS
gcoTEXTURE_QueryCaps(IN gcoHAL Hal,
                     OUT gctUINT *MaxWidth,
                     OUT gctUINT *MaxHeight,
                     OUT gctUINT *MaxDepth,
                     OUT gctBOOL *Cubic,
                     OUT gctBOOL *NonPowerOfTwo,
                     OUT gctUINT *VertexSamplers,
                     OUT gctUINT *PixelSamplers);

gceSTATUS
gcoTEXTURE_GetClosestFormat(IN gcoHAL Hal,
                            IN gceSURF_FORMAT InFormat,
                            OUT gceSURF_FORMAT *OutFormat);

gceSTATUS
gcoTEXTURE_GetClosestFormatEx(IN gcoHAL Hal,
                              IN gceSURF_FORMAT InFormat,
                              IN gceTEXTURE_TYPE TextureType,
                              OUT gceSURF_FORMAT *OutFormat);

gceSTATUS
gcoTEXTURE_GetFormatInfo(IN gcoTEXTURE Texture,
                         IN gctINT preferLevel,
                         OUT gcsSURF_FORMAT_INFO_PTR *TxFormatInfo);

gceSTATUS
gcoTEXTURE_GetTextureFormatName(IN gcsSURF_FORMAT_INFO_PTR TxFormatInfo,
                                OUT gctCONST_STRING *TxName);

gceSTATUS
gcoTEXTURE_RenderIntoMipMap(IN gcoTEXTURE Texture, IN gctINT Level);

gceSTATUS
gcoTEXTURE_RenderIntoMipMap2(IN gcoTEXTURE Texture, IN gctINT Level, IN gctUINT RenderFlag, IN gctBOOL Sync);

gceSTATUS
gcoTEXTURE_IsRenderable(IN gcoTEXTURE Texture, IN gctUINT Level);

gceSTATUS
gcoTEXTURE_IsComplete(IN gcoTEXTURE Texture,
                      IN gcsTEXTURE_PTR Info,
                      IN gctINT BaseLevel,
                      IN gctINT MaxLevel);

gceSTATUS
gcoTEXTURE_CheckTexLevel0Attrib(IN gcoTEXTURE Texture,
                                IN gctINT MaxLevel,
                                IN gctINT usedLevel);

gceSTATUS
gcoTEXTURE_BindTexture(IN gcoTEXTURE Texture,
                       IN gctINT Target,
                       IN gctINT Sampler,
                       IN gcsTEXTURE_PTR Info);

gceSTATUS
gcoTEXTURE_BindTextureEx(IN gcoTEXTURE Texture,
                         IN gctINT Target,
                         IN gctINT Sampler,
                         IN gcsTEXTURE_PTR Info,
                         IN gctINT textureLayer);

gceSTATUS
gcoTEXTURE_BindTextureDesc(IN gcoTEXTURE Texture,
                           IN gctINT Sampler,
                           IN gcsTEXTURE_PTR Info,
                           IN gctINT TextureLayer,
                           IN gctUINT TextureIdx);

gceSTATUS
gcoTEXTURE_SetDescDirty(IN gcoTEXTURE Texture);

gceSTATUS
gcoTEXTURE_InitParams(IN gcoHAL Hal, IN gcsTEXTURE_PTR TexParams);

gceSTATUS
gcoTEXTURE_SetDepthTextureFlag(IN gcoTEXTURE Texture, IN gctBOOL unsized);

gceSTATUS
gcoTEXTURE_SetSpecialSwap(IN gcoTEXTURE Texture, IN gctBOOL needSwap);

gceSTATUS
gcoTEXTURE_BindTextureTS(IN gcsTEXTURE_BINDTEXTS_ARGS *args);

gceSTATUS
gcoTEXTURE_GenerateMipMap(IN gcoTEXTURE Texture,
                          IN gctINT BaseLevel,
                          IN gctINT MaxLevel,
                          IN gctBOOL sRGBDecode);

gceSTATUS
gcoTEXTURE_SetCubeArrayShadow(IN gcoTEXTURE Texture,
                          IN gctBOOL Value);

/******************************************************************************
 ****************************** gcoSTREAM Object ******************************
 ******************************************************************************/

gceSTATUS
gcoSTREAM_Construct(IN gcoHAL Hal, OUT gcoSTREAM *Stream);

gceSTATUS
gcoSTREAM_Destroy(IN gcoSTREAM Stream);

gceSTATUS
gcoSTREAM_Upload(IN gcoSTREAM Stream,
                 IN gctCONST_POINTER Buffer,
                 IN gctSIZE_T Offset,
                 IN gctSIZE_T Bytes,
                 IN gctBOOL Dynamic);

gceSTATUS
gcoSTREAM_ReAllocBufNode(IN gcoSTREAM Stream);

gceSTATUS
gcoSTREAM_SetStride(IN gcoSTREAM Stream, IN gctUINT32 Stride);

gceSTATUS
gcoSTREAM_Node(IN gcoSTREAM Stream, OUT gcsSURF_NODE_PTR *Node);

gceSTATUS
gcoSTREAM_Lock(IN gcoSTREAM Stream,
               OUT gctPOINTER *Logical,
               OUT gctADDRESS *Physical);

gceSTATUS
gcoSTREAM_Unlock(IN gcoSTREAM Stream);

gceSTATUS
gcoSTREAM_Reserve(IN gcoSTREAM Stream, IN gctSIZE_T Bytes);

gceSTATUS
gcoSTREAM_Flush(IN gcoSTREAM Stream);

typedef struct _gcsSTREAM_INFO {
    gctUINT             index;
    gceVERTEX_FORMAT    format;
    gctBOOL             normalized;
    gctUINT             components;
    gctSIZE_T           size;
    gctCONST_POINTER    data;
    gctUINT             stride;
} gcsSTREAM_INFO, *gcsSTREAM_INFO_PTR;

gceSTATUS
gcoSTREAM_CPUCacheOperation(IN gcoSTREAM Stream, IN gceCACHEOPERATION Operation);

gceSTATUS
gcoSTREAM_CPUCacheOperation_Range(IN gcoSTREAM Stream,
                                  IN gctSIZE_T Offset,
                                  IN gctSIZE_T Length,
                                  IN gceCACHEOPERATION Operation);

/******************************************************************************
 ******************************* gcoVERTEX Object *****************************
 ******************************************************************************/

typedef struct _gcsVERTEX_ATTRIBUTES {
    gceVERTEX_FORMAT    format;
    gctBOOL             normalized;
    gctUINT32           components;
    gctSIZE_T           size;
    gctUINT32           stream;
    gctUINT32           offset;
    gctUINT32           stride;
} gcsVERTEX_ATTRIBUTES;

gceSTATUS
gcoVERTEX_Construct(IN gcoHAL Hal, OUT gcoVERTEX *Vertex);

gceSTATUS
gcoVERTEX_Destroy(IN gcoVERTEX Vertex);

gceSTATUS
gcoVERTEX_Reset(IN gcoVERTEX Vertex);

gceSTATUS
gcoVERTEX_EnableAttribute(IN gcoVERTEX Vertex,
                          IN gctUINT32 Index,
                          IN gceVERTEX_FORMAT Format,
                          IN gctBOOL Normalized,
                          IN gctUINT32 Components,
                          IN gcoSTREAM Stream,
                          IN gctUINT32 Offset,
                          IN gctUINT32 Stride);

gceSTATUS
gcoVERTEX_DisableAttribute(IN gcoVERTEX Vertex, IN gctUINT32 Index);

gceSTATUS
gcoVERTEX_Bind(IN gcoVERTEX Vertex);

/******************************************************************************
 **** gcoVERTEXARRAY Object ***************************************************/
#if defined(_WIN32)
#pragma warning (disable: 4201)        /* enable anonymous unions */
#endif
typedef struct _gcsATTRIBUTE {
    /* Enabled. */
    gctBOOL             enable;

    /* Number of components. */
    gctINT              size;

    /* Type of components. */
    gctUINT             type;

    /* Attribute format. */
    gceVERTEX_FORMAT    format;

    /* Flag whether the attribute is normalized or not. */
    gctBOOL             normalized;

    /* Stride of the component. */
    gctSIZE_T           stride;

    /* Divisor of the attribute */
    gctUINT             divisor;

    /* Offset of the attribute */
    gctUINT             offset;

    /* Pointer to the attribute data. */
    gctCONST_POINTER    pointer;

    /* Stream object owning the attribute data. */
    gcoBUFOBJ           stream;

    union
    {
        /* Generic values for attribute. */
        gctFLOAT        genericValue[4];

        /* Double values for attribute. */
        gctDOUBLE       doubleValue[4];
    };

    /* Generic size for attribute. */
    gctINT              genericSize;

    /* Original size for attribute. */
    gctINT              origSize;

    /* Vertex shader linkage. */
    gctUINT             linkage;

#if gcdUSE_WCLIP_PATCH
    /* Does it hold positions? */
    gctBOOL             isPosition;
#endif

    /* Index to vertex array */
    gctINT              arrayIdx;
    gctINT              arrayLoc[32];

    gceATTRIB_SCHEME    convertScheme;

    /* Pointer to the temporary buffer to be freed */
    gcoBUFOBJ           tempStream;

    /* Pointer to the temporary memory to be freed */
    gctCONST_POINTER    tempMemory;
} gcsATTRIBUTE, *gcsATTRIBUTE_PTR;

typedef struct _gcsVERTEXARRAY {
    /* Enabled. */
    gctBOOL             enable;

    /* Number of components. */
    gctINT              size;

    /* Type of components. */
    gctUINT             type;

    /* Attribute format. */
    gceVERTEX_FORMAT    format;

    /* Flag whether the attribute is normalized or not. */
    gctBOOL             normalized;

    /* Stride of the component. */
    gctUINT             stride;

    /* Divisor of the attribute */
    gctUINT             divisor;

    /* Pointer to the attribute data. */
    gctCONST_POINTER    pointer;

    /* Stream object owning the attribute data. */
    gcoSTREAM           stream;

    union
    {
        /* Generic values for attribute. */
        gctFLOAT        genericValue[4];

        /* Double values for attribute. */
        gctDOUBLE       doubleValue[4];
    };

    /* Generic size for attribute. */
    gctINT              genericSize;

    /* Vertex shader linkage. */
    gctUINT             linkage;

    gctBOOL             isPosition;
} gcsVERTEXARRAY, *gcsVERTEXARRAY_PTR;

gceSTATUS
gcoVERTEXARRAY_Construct(IN gcoHAL Hal, OUT gcoVERTEXARRAY *Vertex);

gceSTATUS
gcoVERTEXARRAY_Destroy(IN gcoVERTEXARRAY Vertex);

/* If don't consider isolation, STREAM_INFO / INDEX_INFO could be
 * include in the struct of instantDraw in chip level.
 */
typedef struct _gcsVERTEXARRAY_STREAM_INFO {
    gctUINT         attribMask;
    gctSIZE_T       first;
    gctSIZE_T       count;
    gcePRIMITIVE    primMode;
    gctSIZE_T       primCount;

    /* If vtxInstanceIdLastReg feature is true, vertexId and instanceId will use the
     * x y channel of last temp register, use vertexInstIndex to save the index.
     * Or, will save data to different bit field, save 'vertexIdLocation' to bit 7:0,
     * save 'useVertexId'(boolean) to bit 16:16, save 'instanceIdLocation' to bit 15:8,
     * and save 'useInstanceId'(boolean) to bit 17:17.
     */
    gctINT          vertexInstIndex;

    gctBOOL         instanced;
    gctSIZE_T       instanceCount;
    gctINT          baseInstance;

    union _gcsVERTEXARRAY_STREAM_INFO_UNION {
        struct _gcsVERTEXARRAY_STREAM_ES11_INFO {
            gcsVERTEXARRAY_PTR      attributes;
        } es11;

        struct _gcsVERTEXARRAY_STREAM_ES30_INFO {
            gcsATTRIBUTE_PTR        attributes;
        } es30;
    } u;
} gcsVERTEXARRAY_STREAM_INFO, *gcsVERTEXARRAY_STREAM_INFO_PTR;

typedef const struct _gcsVERTEXARRAY_STREAM_INFO *gcsVERTEXARRAY_STREAM_INFO_CONST_PTR;

typedef struct _gcsVERTEXARRAY_INDEX_INFO {
    gctSIZE_T       count;
    gceINDEX_TYPE   indexType;
    gctPOINTER      indexMemory;
    gctUINT         restartElement;

    union _gcsVERTEXARRAY_INDEX_INFO_UNION {
        struct _gcsVERTEXARRAY_INDEX_ES11_INFO {
            gcoINDEX        indexBuffer;
        } es11;

        struct _gcsVERTEXARRAY_INDEX_ES30_INFO {
            gcoBUFOBJ       indexBuffer;
        } es30;
    } u;
} gcsVERTEXARRAY_INDEX_INFO, *gcsVERTEXARRAY_INDEX_INFO_PTR;

typedef const struct _gcsVERTEXARRAY_INDEX_INFO *gcsVERTEXARRAY_INDEX_INFO_CONST_PTR;

gceSTATUS
gcoVERTEXARRAY_IndexBind(IN gcoVERTEXARRAY Vertex,
                         IN gcsVERTEXARRAY_INDEX_INFO_PTR IndexInfo);

#if gcdUSE_WCLIP_PATCH
gceSTATUS
gcoVERTEXARRAY_StreamBind(IN gcoVERTEXARRAY Vertex,
                          IN OUT gctFLOAT *WLimitRms,
                          IN OUT gctBOOL *WLimitRmsDirty,
                          IN gcsVERTEXARRAY_STREAM_INFO_CONST_PTR StreamInfo,
                          IN gcsVERTEXARRAY_INDEX_INFO_CONST_PTR IndexInfo);

gceSTATUS
gcoVERTEXARRAY_StreamBind_Ex(IN gcoVERTEXARRAY Vertex,
                             IN OUT gctFLOAT *WLimitRms,
                             IN OUT gctBOOL *WLimitRmsDirty,
                             IN OUT gcsVERTEXARRAY_STREAM_INFO_PTR StreamInfo,
                             IN gcsVERTEXARRAY_INDEX_INFO_PTR IndexInfo);
# else
gceSTATUS
gcoVERTEXARRAY_StreamBind(IN gcoVERTEXARRAY Vertex,
                          IN gcsVERTEXARRAY_STREAM_INFO_CONST_PTR StreamInfo,
                          IN gcsVERTEXARRAY_INDEX_INFO_CONST_PTR IndexInfo);
gceSTATUS
gcoVERTEXARRAY_StreamBind_Ex(IN gcoVERTEXARRAY Vertex,
                             IN OUT gcsVERTEXARRAY_STREAM_INFO_PTR StreamInfo,
                             IN gcsVERTEXARRAY_INDEX_INFO_PTR IndexInfo);
# endif

gceSTATUS
gcoVERTEXARRAY_IndexBind_Ex(IN gcoVERTEXARRAY Vertex,
                            IN OUT gcsVERTEXARRAY_STREAM_INFO_PTR StreamInfo,
                            IN gcsVERTEXARRAY_INDEX_INFO_PTR IndexInfo);

gceSTATUS
gcoVERTEXARRAY_Bind(IN gcoVERTEXARRAY Vertex,
                    IN gctUINT32 EnableBits,
                    IN gcsVERTEXARRAY_PTR VertexArray,
                    IN gctUINT First,
                    IN gctSIZE_T *Count,
                    IN gceINDEX_TYPE IndexType,
                    IN gcoINDEX IndexObject,
                    IN gctPOINTER IndexMemory,
                    IN OUT gcePRIMITIVE *PrimitiveType,
#if gcdUSE_WCLIP_PATCH
                    IN OUT gctUINT *PrimitiveCount,
                    IN OUT gctFLOAT *wLimitRms,
                    IN OUT gctBOOL *wLimitDirty
# else
                    IN OUT gctUINT *PrimitiveCount
# endif
);

/* Frame Database */
gceSTATUS
gcoHAL_AddFrameDB(void);

gceSTATUS
gcoHAL_DumpFrameDB(gctCONST_STRING Filename OPTIONAL);

gceSTATUS
gcoHAL_InitGPUProfile(void);

/******************************************************************************
 *********************gcoBUFOBJ object*****************************************
 ******************************************************************************/
/* Construct a new gcoBUFOBJ object. */
gceSTATUS
gcoBUFOBJ_Construct(IN gcoHAL Hal, IN gceBUFOBJ_TYPE Type, OUT gcoBUFOBJ *BufObj);

/* Destroy a gcoBUFOBJ object. */
gceSTATUS
gcoBUFOBJ_Destroy(IN gcoBUFOBJ BufObj);

/* Lock pbo in memory. */
gceSTATUS
gcoBUFOBJ_Lock(IN gcoBUFOBJ BufObj,
               OUT gctADDRESS *Address,
               OUT gctPOINTER *Memory);

/* Lock pbo in memory. */
gceSTATUS
gcoBUFOBJ_FastLock(IN gcoBUFOBJ BufObj,
                   OUT gctADDRESS *Address,
                   OUT gctPOINTER *Memory);

/* Unlock pbo that was previously locked with gcoBUFOBJ_Lock. */
gceSTATUS
gcoBUFOBJ_Unlock(IN gcoBUFOBJ BufObj);

/* Free existing pbo buffer. */
gceSTATUS
gcoBUFOBJ_Free(IN gcoBUFOBJ BufObj);

/* Upload data into an pbo buffer. */
gceSTATUS
gcoBUFOBJ_Upload(IN gcoBUFOBJ BufObj,
                 IN gctCONST_POINTER Buffer,
                 IN gctSIZE_T Offset,
                 IN gctSIZE_T Bytes,
                 IN gceBUFOBJ_USAGE Usage);

/* Bind an index object to the hardware. */
gceSTATUS
gcoBUFOBJ_IndexBind(IN gcoBUFOBJ Index,
                    IN gceINDEX_TYPE Type,
                    IN gctSIZE_T Offset,
                    IN gctSIZE_T Count,
                    IN gctUINT RestartElement);

/* Find min and max index for the index buffer */
gceSTATUS
gcoBUFOBJ_IndexGetRange(IN gcoBUFOBJ Index,
                        IN gceINDEX_TYPE Type,
                        IN gctSIZE_T Offset,
                        IN gctUINT32 Count,
                        OUT gctUINT32 *MinimumIndex,
                        OUT gctUINT32 *MaximumIndex);

/* Sets buffer upload endian hint */
gceSTATUS
gcoBUFOBJ_SetBufferEndianHint(IN gcoBUFOBJ BufObj);

/*  Query a buffer object dirty status */
gceSTATUS
gcoBUFOBJ_SetDirty(IN gcoBUFOBJ BufObj, IN gctBOOL Dirty);

/*  Set BufObj is Mapped */
gceSTATUS
gcoBUFOBJ_SetMapped(IN gcoBUFOBJ BufObj, IN gctBOOL isMapped);

/*  Sets a buffer object as dirty */
gctBOOL
gcoBUFOBJ_IsDirty(IN gcoBUFOBJ BufObj);

/* Creates a new buffer if needed */
gceSTATUS
gcoBUFOBJ_AlignIndexBufferWhenNeeded(IN gcoBUFOBJ BufObj,
                                     IN gctSIZE_T Offset,
                                     OUT gcoBUFOBJ *AlignedBufObj);

/* Cache operations on whole range */
gceSTATUS
gcoBUFOBJ_CPUCacheOperation(IN gcoBUFOBJ BufObj, IN gceCACHEOPERATION Operation);

/* Cache operations on a specified range */
gceSTATUS
gcoBUFOBJ_CPUCacheOperation_Range(IN gcoBUFOBJ BufObj,
                                  IN gctSIZE_T Offset,
                                  IN gctSIZE_T Length,
                                  IN gceCACHEOPERATION Operation);

/* Return size of the bufobj */
gceSTATUS
gcoBUFOBJ_GetSize(IN gcoBUFOBJ BufObj, OUT gctSIZE_T_PTR Size);

/* Return memory node of the bufobj */
gceSTATUS
gcoBUFOBJ_GetNode(IN gcoBUFOBJ BufObj, OUT gcsSURF_NODE_PTR *Node);

gceSTATUS
gcoBUFOBJ_ReAllocBufNode(IN gcoBUFOBJ BufObj);

/* Handle GPU cache operations */
gceSTATUS
gcoBUFOBJ_SetCPUWrite(gcoBUFOBJ BufObj, gctBOOL Value);

/* Dump buffer. */
void
gcoBUFOBJ_Dump(IN gcoBUFOBJ BufObj);

#endif /* gcdENABLE_3D */

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_engine_h_ */
