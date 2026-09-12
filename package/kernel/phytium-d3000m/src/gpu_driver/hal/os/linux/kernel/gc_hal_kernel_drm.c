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


#if gcdENABLE_DRM

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)
# include <drm/drm_drv.h>
# include <drm/drm_file.h>
# include <drm/drm_ioctl.h>
#else
# include <drm/drmP.h>
#endif
#include <drm/drm_gem.h>
#include <linux/dma-buf.h>
#include "gc_hal_kernel_linux.h"
#include "gc_hal_drm.h"

#define _GC_OBJ_ZONE gcvZONE_KERNEL

/*******************************************************************************
 ****************************** gckKERNEL DRM Code *****************************
 *******************************************************************************/

struct viv_gem_object {
    struct drm_gem_object   base;
    uint32_t                node_handle;
    gckVIDMEM_NODE          node_object;
    gctBOOL                 cacheable;
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
void viv_gem_free_object(struct drm_gem_object *gem_obj);
struct dma_buf *viv_gem_prime_export(struct drm_gem_object *gem_obj, int flags);

static const struct drm_gem_object_funcs viv_gem_object_funcs = {
    .free   = viv_gem_free_object,
    .export = viv_gem_prime_export,
};
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
struct dma_buf *viv_gem_prime_export(struct drm_gem_object *gem_obj, int flags)
{
    struct drm_device *drm = gem_obj->dev;
#else
struct dma_buf *viv_gem_prime_export(struct drm_device *drm, struct drm_gem_object *gem_obj, int flags)
{
#endif
    struct viv_gem_object *viv_obj = container_of(gem_obj, struct viv_gem_object, base);
    struct dma_buf *dmabuf  = gcvNULL;
    gckGALDEVICE gal_dev = (gckGALDEVICE)drm->dev_private;

    if (gal_dev) {
        gckDEVICE device = gal_dev->devices[0];
        gckKERNEL kernel = device->kernels[0];

        gcmkVERIFY_OK(gckVIDMEM_NODE_Export(kernel, viv_obj->node_object, flags,
                                            (gctPOINTER *)&dmabuf, gcvNULL));
    }

    return dmabuf;
}

static struct drm_gem_object *viv_gem_prime_import(struct drm_device *drm, struct dma_buf *dmabuf)
{
    struct drm_gem_object *gem_obj = gcvNULL;
    struct viv_gem_object *viv_obj;

    gcsHAL_INTERFACE iface;
    gckGALDEVICE gal_dev;
    gckDEVICE device;
    gckKERNEL kernel;
    gctUINT32 processID;
    gckVIDMEM_NODE nodeObject;
    gceSTATUS status = gcvSTATUS_OK;

    gal_dev = (gckGALDEVICE)drm->dev_private;
    if (!gal_dev)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    device = gal_dev->devices[0];

    gckOS_ZeroMemory(&iface, sizeof(iface));
    iface.command = gcvHAL_WRAP_USER_MEMORY;
    iface.hardwareType = device->defaultHwType;
    iface.u.WrapUserMemory.desc.flag = gcvALLOC_FLAG_DMABUF;
    iface.u.WrapUserMemory.desc.handle = -1;
    iface.u.WrapUserMemory.desc.dmabuf = gcmPTR_TO_UINT64(dmabuf);
    gcmkONERROR(gckDEVICE_Dispatch(device, &iface));

    kernel = device->kernels[0];
    gcmkONERROR(gckOS_GetProcessID(&processID));
    gcmkONERROR(gckVIDMEM_HANDLE_Lookup(kernel,
                                        processID,
                                        iface.u.WrapUserMemory.node,
                                        &nodeObject));

    /* ioctl output */
    gem_obj = kzalloc(sizeof(*viv_obj), GFP_KERNEL);
    drm_gem_private_object_init(drm, gem_obj, dmabuf->size);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
    gem_obj->funcs = &viv_gem_object_funcs;
#endif
    viv_obj = container_of(gem_obj, struct viv_gem_object, base);
    viv_obj->node_handle = iface.u.WrapUserMemory.node;
    viv_obj->node_object = nodeObject;

OnError:
    return gem_obj;
}

void viv_gem_free_object(struct drm_gem_object *gem_obj)
{
    struct viv_gem_object *viv_obj  = container_of(gem_obj, struct viv_gem_object, base);
    struct drm_device *drm = gem_obj->dev;

    gcsHAL_INTERFACE iface;
    gckGALDEVICE gal_dev = (gckGALDEVICE)drm->dev_private;
    gckDEVICE device;

    device = gal_dev->devices[0];

    gckOS_ZeroMemory(&iface, sizeof(iface));
    iface.command = gcvHAL_RELEASE_VIDEO_MEMORY;
    iface.hardwareType = device->defaultHwType;
    iface.u.ReleaseVideoMemory.node = viv_obj->node_handle;
    gcmkVERIFY_OK(gckDEVICE_Dispatch(device, &iface));
    drm_gem_object_release(gem_obj);
    kfree(gem_obj);
}

static int viv_ioctl_gem_create(struct drm_device *drm, void *data, struct drm_file *file)
{
    int ret = 0;
    struct drm_viv_gem_create *args = (struct drm_viv_gem_create *)data;
    struct drm_gem_object *gem_obj = gcvNULL;
    struct viv_gem_object *viv_obj = gcvNULL;

    gcsHAL_INTERFACE iface;
    gckGALDEVICE gal_dev;
    gckKERNEL kernel;
    gctUINT32 processID;
    gckVIDMEM_NODE nodeObject;
    gctUINT32 flags = gcvALLOC_FLAG_DMABUF_EXPORTABLE;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT64 alignSize = PAGE_ALIGN(args->size);
    gckDEVICE device;

    gal_dev = (gckGALDEVICE)drm->dev_private;
    if (!gal_dev)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    device = gal_dev->devices[0];

    flags |= gcvALLOC_FLAG_FROM_USER;

    if (args->flags & DRM_VIV_GEM_CONTIGUOUS)
        flags |= gcvALLOC_FLAG_CONTIGUOUS;

    if (args->flags & DRM_VIV_GEM_CACHED)
        flags |= gcvALLOC_FLAG_CACHEABLE;

    if (args->flags & DRM_VIV_GEM_SECURE)
        flags |= gcvALLOC_FLAG_SECURITY;

    gckOS_ZeroMemory(&iface, sizeof(iface));
    iface.command = gcvHAL_ALLOCATE_LINEAR_VIDEO_MEMORY;
    iface.hardwareType = device->defaultHwType;
    iface.u.AllocateLinearVideoMemory.bytes = alignSize;
    iface.u.AllocateLinearVideoMemory.alignment = 256;
    iface.u.AllocateLinearVideoMemory.type = gcvVIDMEM_TYPE_GENERIC;
    iface.u.AllocateLinearVideoMemory.flag = flags;
    iface.u.AllocateLinearVideoMemory.pool = gcvPOOL_DEFAULT;
    gcmkONERROR(gckDEVICE_Dispatch(device, &iface));

    kernel = device->kernels[0];
    gcmkONERROR(gckOS_GetProcessID(&processID));
    gcmkONERROR(gckVIDMEM_HANDLE_Lookup(kernel, processID,
                                        iface.u.AllocateLinearVideoMemory.node,
                                        &nodeObject));

    /* ioctl output */
    gem_obj = kzalloc(sizeof(*viv_obj), GFP_KERNEL);
    drm_gem_private_object_init(drm, gem_obj, (size_t)alignSize);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
    gem_obj->funcs = &viv_gem_object_funcs;
#endif
    ret = drm_gem_handle_create(file, gem_obj, &args->handle);

    viv_obj = container_of(gem_obj, struct viv_gem_object, base);
    viv_obj->node_handle = iface.u.AllocateLinearVideoMemory.node;
    viv_obj->node_object = nodeObject;
    viv_obj->cacheable = flags & gcvALLOC_FLAG_CACHEABLE;

    /* drop reference from allocate - handle holds it now */
    drm_gem_object_unreference_unlocked(gem_obj);

OnError:
    return gcmIS_ERROR(status) ? -ENOTTY : 0;
}

static int viv_ioctl_gem_lock(struct drm_device *drm, void *data, struct drm_file *file)
{
    struct drm_viv_gem_lock *args = (struct drm_viv_gem_lock *)data;
    struct drm_gem_object *gem_obj = gcvNULL;
    struct viv_gem_object *viv_obj = gcvNULL;

    gcsHAL_INTERFACE iface;
    gceSTATUS status = gcvSTATUS_OK;
    gckGALDEVICE gal_dev = gcvNULL;
    gckDEVICE device;

    gal_dev = (gckGALDEVICE)drm->dev_private;
    if (!gal_dev)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    device = gal_dev->devices[0];

    gem_obj = drm_gem_object_lookup(file, args->handle);
    if (!gem_obj)
        gcmkONERROR(gcvSTATUS_NOT_FOUND);

    viv_obj = container_of(gem_obj, struct viv_gem_object, base);

    gckOS_ZeroMemory(&iface, sizeof(iface));
    iface.command = gcvHAL_LOCK_VIDEO_MEMORY;
    iface.hardwareType = device->defaultHwType;
    iface.u.LockVideoMemory.op = gcvLOCK_VIDEO_MEMORY_OP_LOCK |
                                 gcvLOCK_VIDEO_MEMORY_OP_MAP;
    iface.u.LockVideoMemory.node = viv_obj->node_handle;
    iface.u.LockVideoMemory.cacheable = args->cacheable;
    gcmkONERROR(gckDEVICE_Dispatch(device, &iface));

    args->logical = iface.u.LockVideoMemory.memory;

OnError:
    if (gem_obj)
        drm_gem_object_unreference_unlocked(gem_obj);

    return gcmIS_ERROR(status) ? -ENOTTY : 0;
}

static int viv_ioctl_gem_unlock(struct drm_device *drm, void *data, struct drm_file *file)
{
    struct drm_viv_gem_unlock *args = (struct drm_viv_gem_unlock *)data;
    struct drm_gem_object *gem_obj = gcvNULL;
    struct viv_gem_object *viv_obj = gcvNULL;

    gcsHAL_INTERFACE iface;
    gceSTATUS status = gcvSTATUS_OK;
    gckGALDEVICE gal_dev = gcvNULL;
    gckDEVICE device;

    gal_dev = (gckGALDEVICE)drm->dev_private;
    if (!gal_dev)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    device = gal_dev->devices[0];

    gem_obj = drm_gem_object_lookup(file, args->handle);
    if (!gem_obj)
        gcmkONERROR(gcvSTATUS_NOT_FOUND);

    viv_obj = container_of(gem_obj, struct viv_gem_object, base);

    memset(&iface, 0, sizeof(iface));
    iface.command = gcvHAL_UNLOCK_VIDEO_MEMORY;
    iface.hardwareType = device->defaultHwType;
    iface.u.UnlockVideoMemory.op = gcvLOCK_VIDEO_MEMORY_OP_UNLOCK |
                                       gcvLOCK_VIDEO_MEMORY_OP_UNMAP;
    iface.u.UnlockVideoMemory.node = (gctUINT64)viv_obj->node_handle;
    iface.u.UnlockVideoMemory.type = gcvVIDMEM_TYPE_GENERIC;
    gcmkONERROR(gckDEVICE_Dispatch(device, &iface));

    memset(&iface, 0, sizeof(iface));
    iface.command = gcvHAL_BOTTOM_HALF_UNLOCK_VIDEO_MEMORY;
    iface.hardwareType = device->defaultHwType;
    iface.u.BottomHalfUnlockVideoMemory.node = (gctUINT64)viv_obj->node_handle;
    iface.u.BottomHalfUnlockVideoMemory.type = gcvVIDMEM_TYPE_GENERIC;
    gcmkONERROR(gckDEVICE_Dispatch(device, &iface));

OnError:
    if (gem_obj)
        drm_gem_object_unreference_unlocked(gem_obj);

    return gcmIS_ERROR(status) ? -ENOTTY : 0;
}

static int viv_ioctl_gem_cache(struct drm_device *drm, void *data, struct drm_file *file)
{
    struct drm_viv_gem_cache *args = (struct drm_viv_gem_cache *)data;
    struct drm_gem_object *gem_obj = gcvNULL;
    struct viv_gem_object *viv_obj = gcvNULL;

    gcsHAL_INTERFACE iface;
    gceSTATUS status = gcvSTATUS_OK;
    gckGALDEVICE gal_dev = gcvNULL;
    gckDEVICE device;
    gceCACHEOPERATION cache_op = 0;

    gal_dev = (gckGALDEVICE)drm->dev_private;
    if (!gal_dev)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    device = gal_dev->devices[0];

    gem_obj = drm_gem_object_lookup(file, args->handle);
    if (!gem_obj)
        gcmkONERROR(gcvSTATUS_NOT_FOUND);

    viv_obj = container_of(gem_obj, struct viv_gem_object, base);

    switch (args->op) {
    case DRM_VIV_GEM_CLEAN_CACHE:
        cache_op = gcvCACHE_CLEAN;
        break;
    case DRM_VIV_GEM_INVALIDATE_CACHE:
        cache_op = gcvCACHE_INVALIDATE;
        break;
    case DRM_VIV_GEM_FLUSH_CACHE:
        cache_op = gcvCACHE_FLUSH;
        break;
    case DRM_VIV_GEM_MEMORY_BARRIER:
        cache_op = gcvCACHE_MEMORY_BARRIER;
        break;
    default:
        break;
    }

    gckOS_ZeroMemory(&iface, sizeof(iface));
    iface.command           = gcvHAL_CACHE;
    iface.hardwareType      = device->defaultHwType;
    iface.u.Cache.node      = viv_obj->node_handle;
    iface.u.Cache.operation = cache_op;
    iface.u.Cache.logical   = args->logical;
    iface.u.Cache.bytes     = args->bytes;
    gcmkONERROR(gckDEVICE_Dispatch(device, &iface));

OnError:
    if (gem_obj)
        drm_gem_object_unreference_unlocked(gem_obj);

    return gcmIS_ERROR(status) ? -ENOTTY : 0;
}

static int viv_ioctl_gem_query(struct drm_device *drm, void *data, struct drm_file *file)
{
    struct drm_viv_gem_query *args = (struct drm_viv_gem_query *)data;
    struct drm_gem_object *gem_obj = gcvNULL;
    struct viv_gem_object *viv_obj = gcvNULL;

    gceSTATUS status = gcvSTATUS_OK;
    gckGALDEVICE gal_dev = gcvNULL;

    gal_dev = (gckGALDEVICE)drm->dev_private;
    if (!gal_dev)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    gem_obj = drm_gem_object_lookup(file, args->handle);
    if (!gem_obj)
        gcmkONERROR(gcvSTATUS_NOT_FOUND);

    viv_obj = container_of(gem_obj, struct viv_gem_object, base);

    switch (args->param) {
    case DRM_VIV_GEM_PARAM_POOL:
        args->value = (__u64)viv_obj->node_object->pool;
        break;
    case DRM_VIV_GEM_PARAM_SIZE:
        args->value = (__u64)gem_obj->size;
        break;
    default:
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

OnError:
    if (gem_obj)
        drm_gem_object_unreference_unlocked(gem_obj);

    return gcmIS_ERROR(status) ? -ENOTTY : 0;
}

static int viv_ioctl_gem_timestamp(struct drm_device *drm,
                                   void *data, struct drm_file *file)
{
    struct drm_viv_gem_timestamp *args    = (struct drm_viv_gem_timestamp *)data;
    struct drm_gem_object *gem_obj = gcvNULL;
    struct viv_gem_object *viv_obj = gcvNULL;

    gceSTATUS status = gcvSTATUS_OK;
    gckGALDEVICE gal_dev = gcvNULL;

    gal_dev = (gckGALDEVICE)drm->dev_private;
    if (!gal_dev)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    gem_obj = drm_gem_object_lookup(file, args->handle);
    if (!gem_obj)
        gcmkONERROR(gcvSTATUS_NOT_FOUND);

    viv_obj = container_of(gem_obj, struct viv_gem_object, base);

    viv_obj->node_object->timeStamp += args->inc;
    args->timestamp = viv_obj->node_object->timeStamp;

OnError:
    if (gem_obj)
        drm_gem_object_unreference_unlocked(gem_obj);

    return gcmIS_ERROR(status) ? -ENOTTY : 0;
}

static int viv_ioctl_gem_set_tiling(struct drm_device *drm,
                                    void *data, struct drm_file *file)
{
    struct drm_viv_gem_set_tiling *args    = (struct drm_viv_gem_set_tiling *)data;
    struct drm_gem_object *gem_obj = gcvNULL;
    struct viv_gem_object *viv_obj = gcvNULL;

    gceSTATUS status = gcvSTATUS_OK;
    gckGALDEVICE gal_dev = gcvNULL;

    gal_dev = (gckGALDEVICE)drm->dev_private;
    if (!gal_dev)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    gem_obj = drm_gem_object_lookup(file, args->handle);
    if (!gem_obj)
        gcmkONERROR(gcvSTATUS_NOT_FOUND);

    viv_obj = container_of(gem_obj, struct viv_gem_object, base);

    viv_obj->node_object->tilingMode  = args->tiling_mode;
    viv_obj->node_object->tsMode      = args->ts_mode;
    viv_obj->node_object->tsCacheMode = args->ts_cache_mode;
    viv_obj->node_object->clearValue  = args->clear_value;

OnError:
    if (gem_obj)
        drm_gem_object_unreference_unlocked(gem_obj);

    return gcmIS_ERROR(status) ? -ENOTTY : 0;
}

static int viv_ioctl_gem_get_tiling(struct drm_device *drm,
                                    void *data, struct drm_file *file)
{
    struct drm_viv_gem_get_tiling *args = (struct drm_viv_gem_get_tiling *)data;
    struct drm_gem_object *gem_obj = gcvNULL;
    struct viv_gem_object *viv_obj = gcvNULL;

    gceSTATUS status = gcvSTATUS_OK;
    gckGALDEVICE gal_dev = gcvNULL;

    gal_dev = (gckGALDEVICE)drm->dev_private;
    if (!gal_dev)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    gem_obj = drm_gem_object_lookup(file, args->handle);
    if (!gem_obj)
        gcmkONERROR(gcvSTATUS_NOT_FOUND);

    viv_obj = container_of(gem_obj, struct viv_gem_object, base);

    args->tiling_mode   = viv_obj->node_object->tilingMode;
    args->ts_mode       = viv_obj->node_object->tsMode;
    args->ts_cache_mode = viv_obj->node_object->tsCacheMode;
    args->clear_value   = viv_obj->node_object->clearValue;

OnError:
    if (gem_obj)
        drm_gem_object_unreference_unlocked(gem_obj);

    return gcmIS_ERROR(status) ? -ENOTTY : 0;
}

static int viv_ioctl_gem_attach_aux(struct drm_device *drm,
                                    void *data, struct drm_file *file)
{
    struct drm_viv_gem_attach_aux *args = (struct drm_viv_gem_attach_aux *)data;
    struct drm_gem_object *gem_obj = gcvNULL;
    struct viv_gem_object *viv_obj = gcvNULL;
    struct drm_gem_object *gem_ts_obj = gcvNULL;

    gceSTATUS status = gcvSTATUS_OK;
    gckGALDEVICE gal_dev = gcvNULL;
    gckDEVICE device;
    gckVIDMEM_NODE nodeObj = gcvNULL;

    gal_dev = (gckGALDEVICE)drm->dev_private;
    if (!gal_dev)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    device = gal_dev->devices[0];

    gem_obj = drm_gem_object_lookup(file, args->handle);
    if (!gem_obj)
        gcmkONERROR(gcvSTATUS_NOT_FOUND);

    viv_obj = container_of(gem_obj, struct viv_gem_object, base);
    nodeObj = viv_obj->node_object;

    /* do not support re-attach */
    if (nodeObj->tsNode)
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);

    if (args->ts_handle) {
        struct viv_gem_object *viv_ts_obj;
        gckKERNEL kernel = device->kernels[0];
        gcsHAL_INTERFACE iface;
        gctBOOL is128BTILE = gckHARDWARE_IsFeatureAvailable(kernel->hardware,
                                                            gcvFEATURE_128BTILE);
        gctBOOL is2BitPerTile = is128BTILE ?
                                gcvFALSE :
                                gckHARDWARE_IsFeatureAvailable(kernel->hardware,
                                                               gcvFEATURE_TILE_STATUS_2BITS);
        gctBOOL isCompressionDEC400 = gckHARDWARE_IsFeatureAvailable(kernel->hardware,
                                                                     gcvFEATURE_COMPRESSION_DEC400);
        gctPOINTER entry = gcvNULL;
        gckVIDMEM_NODE ObjNode = gcvNULL;
        gctUINT32 processID = 0;
        gctUINT32 tileStatusFiller = isCompressionDEC400 ? 0xFFFFFFFF :
                                     is2BitPerTile ? 0x55555555 : 0x11111111;

        gem_ts_obj = drm_gem_object_lookup(file, args->ts_handle);
        if (!gem_ts_obj)
            gcmkONERROR(gcvSTATUS_NOT_FOUND);

        viv_ts_obj = container_of(gem_ts_obj, struct viv_gem_object, base);

        gcmkONERROR(gckVIDMEM_NODE_Reference(kernel, viv_ts_obj->node_object));
        nodeObj->tsNode = viv_ts_obj->node_object;

        /* Fill tile status node with tileStatusFiller value first time to avoid GPU hang. */
        /* Lock tile status node. */
        gckOS_ZeroMemory(&iface, sizeof(iface));
        iface.command = gcvHAL_LOCK_VIDEO_MEMORY;
        iface.hardwareType = device->defaultHwType;
        iface.u.LockVideoMemory.op = gcvLOCK_VIDEO_MEMORY_OP_LOCK |
                                       gcvLOCK_VIDEO_MEMORY_OP_MAP;
        iface.u.LockVideoMemory.node = viv_ts_obj->node_handle;
        iface.u.LockVideoMemory.cacheable = viv_ts_obj->cacheable;
        gcmkONERROR(gckDEVICE_Dispatch(device, &iface));

        gcmkONERROR(gckOS_GetProcessID(&processID));
        gcmkONERROR(gckVIDMEM_HANDLE_Lookup(kernel, processID, viv_ts_obj->node_handle, &ObjNode));
        gcmkONERROR(gckVIDMEM_NODE_LockCPU(kernel, ObjNode, gcvFALSE, gcvFALSE, &entry));

        /* Fill tile status node with tileStatusFiller. */
        memset(entry, tileStatusFiller, (__u64)gem_ts_obj->size);
        gcmkONERROR(gckVIDMEM_NODE_UnlockCPU(kernel, ObjNode, 0, gcvFALSE, gcvFALSE));

        /* UnLock tile status node. */
        memset(&iface, 0, sizeof(iface));
        iface.command = gcvHAL_UNLOCK_VIDEO_MEMORY;
        iface.hardwareType = device->defaultHwType;
        iface.u.UnlockVideoMemory.op = gcvLOCK_VIDEO_MEMORY_OP_UNLOCK |
                                       gcvLOCK_VIDEO_MEMORY_OP_UNMAP;
        iface.u.UnlockVideoMemory.node = (gctUINT64)viv_ts_obj->node_handle;
        iface.u.UnlockVideoMemory.type = gcvSURF_TYPE_UNKNOWN;
        gcmkONERROR(gckDEVICE_Dispatch(device, &iface));

        memset(&iface, 0, sizeof(iface));
        iface.command = gcvHAL_BOTTOM_HALF_UNLOCK_VIDEO_MEMORY;
        iface.hardwareType = device->defaultHwType;
        iface.u.BottomHalfUnlockVideoMemory.node = (gctUINT64)viv_ts_obj->node_handle;
        iface.u.BottomHalfUnlockVideoMemory.type = gcvSURF_TYPE_UNKNOWN;
        gcmkONERROR(gckDEVICE_Dispatch(device, &iface));
    }

OnError:
    if (gem_obj) {
        drm_gem_object_unreference_unlocked(gem_obj);

        if (gem_ts_obj)
            drm_gem_object_unreference_unlocked(gem_ts_obj);
    }
    return gcmIS_ERROR(status) ? -ENOTTY : 0;
}

static int viv_ioctl_gem_ref_node(struct drm_device *drm,
                                  void *data, struct drm_file *file)
{
    struct drm_viv_gem_ref_node *args = (struct drm_viv_gem_ref_node *)data;
    struct drm_gem_object *gem_obj = gcvNULL;
    struct viv_gem_object *viv_obj = gcvNULL;

    gceSTATUS status  = gcvSTATUS_OK;
    gckGALDEVICE gal_dev = gcvNULL;
    gckDEVICE device;
    gckKERNEL kernel = gcvNULL;
    gctUINT32 processID;
    gckVIDMEM_NODE nodeObj;
    gceDATABASE_TYPE type;
    gctUINT32 nodeHandle = 0, tsNodeHandle = 0;
    gctBOOL referred = gcvFALSE;
    gctBOOL isContiguous = gcvFALSE;
    int ret = 0;

    gal_dev = (gckGALDEVICE)drm->dev_private;
    if (!gal_dev)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    device = gal_dev->devices[0];

    kernel = device->kernels[0];

    gem_obj = drm_gem_object_lookup(file, args->handle);
    if (!gem_obj)
        gcmkONERROR(gcvSTATUS_NOT_FOUND);

    viv_obj = container_of(gem_obj, struct viv_gem_object, base);
    nodeObj = viv_obj->node_object;

    gcmkONERROR(gckOS_GetProcessID(&processID));
    gcmkONERROR(gckVIDMEM_HANDLE_Allocate(kernel, nodeObj, &nodeHandle));

    type = gcvDB_VIDEO_MEMORY |
           (nodeObj->type << gcdDB_VIDEO_MEMORY_TYPE_SHIFT) |
           (nodeObj->pool << gcdDB_VIDEO_MEMORY_POOL_SHIFT);

    gcmkONERROR(gckVIDMEM_NODE_IsContiguous(kernel, nodeObj, &isContiguous));
    if (isContiguous)
        type |= (gcvDB_CONTIGUOUS << gcdDB_VIDEO_MEMORY_DBTYPE_SHIFT);

    gcmkONERROR(gckKERNEL_AddProcessDB(kernel, processID, type,
                                       gcmINT2PTR(nodeHandle), gcvNULL, 0));
    gcmkONERROR(gckVIDMEM_NODE_Reference(kernel, nodeObj));
    referred = gcvTRUE;
    if (nodeObj->tsNode) {
        type = gcvDB_VIDEO_MEMORY |
               (nodeObj->tsNode->type << gcdDB_VIDEO_MEMORY_TYPE_SHIFT) |
               (nodeObj->tsNode->pool << gcdDB_VIDEO_MEMORY_POOL_SHIFT);

        gcmkONERROR(gckVIDMEM_NODE_IsContiguous(kernel, nodeObj->tsNode, &isContiguous));
        if (isContiguous)
            type |= (gcvDB_CONTIGUOUS << gcdDB_VIDEO_MEMORY_DBTYPE_SHIFT);

        gcmkONERROR(gckVIDMEM_HANDLE_Allocate(kernel, nodeObj->tsNode, &tsNodeHandle));
        gcmkONERROR(gckKERNEL_AddProcessDB(kernel, processID, type,
                                           gcmINT2PTR(tsNodeHandle), gcvNULL, 0));
        gcmkONERROR(gckVIDMEM_NODE_Reference(kernel, nodeObj->tsNode));
    }
    args->node = nodeHandle;
    args->ts_node = tsNodeHandle;

OnError:
    if (gcmIS_ERROR(status) && kernel) {
        gctUINT32 processID;

        gcmkVERIFY_OK(gckOS_GetProcessID(&processID));

        if (tsNodeHandle)
            gckVIDMEM_HANDLE_Dereference(kernel, processID, tsNodeHandle);

        if (nodeHandle)
            gckVIDMEM_HANDLE_Dereference(kernel, processID, nodeHandle);

        if (referred)
            gcmkONERROR(gckVIDMEM_NODE_Dereference(kernel, nodeObj));

        args->node = 0;
        args->ts_node = 0;

        ret = -ENOTTY;
    }

    if (gem_obj)
        drm_gem_object_unreference_unlocked(gem_obj);

    return ret;
}

static const struct drm_ioctl_desc viv_ioctls[] = {
    DRM_IOCTL_DEF_DRV(VIV_GEM_CREATE,        viv_ioctl_gem_create,     DRM_AUTH | DRM_RENDER_ALLOW),
    DRM_IOCTL_DEF_DRV(VIV_GEM_LOCK,          viv_ioctl_gem_lock,       DRM_AUTH | DRM_RENDER_ALLOW),
    DRM_IOCTL_DEF_DRV(VIV_GEM_UNLOCK,        viv_ioctl_gem_unlock,     DRM_AUTH | DRM_RENDER_ALLOW),
    DRM_IOCTL_DEF_DRV(VIV_GEM_CACHE,         viv_ioctl_gem_cache,      DRM_AUTH | DRM_RENDER_ALLOW),
    DRM_IOCTL_DEF_DRV(VIV_GEM_QUERY,         viv_ioctl_gem_query,      DRM_AUTH | DRM_RENDER_ALLOW),
    DRM_IOCTL_DEF_DRV(VIV_GEM_TIMESTAMP,     viv_ioctl_gem_timestamp,  DRM_AUTH | DRM_RENDER_ALLOW),
    DRM_IOCTL_DEF_DRV(VIV_GEM_SET_TILING,    viv_ioctl_gem_set_tiling, DRM_AUTH | DRM_RENDER_ALLOW),
    DRM_IOCTL_DEF_DRV(VIV_GEM_GET_TILING,    viv_ioctl_gem_get_tiling, DRM_AUTH | DRM_RENDER_ALLOW),
    DRM_IOCTL_DEF_DRV(VIV_GEM_ATTACH_AUX,    viv_ioctl_gem_attach_aux, DRM_AUTH | DRM_RENDER_ALLOW),
    DRM_IOCTL_DEF_DRV(VIV_GEM_REF_NODE,      viv_ioctl_gem_ref_node,   DRM_AUTH | DRM_RENDER_ALLOW),
};

static int viv_drm_open(struct drm_device *drm, struct drm_file *file)
{
    gctINT i, dev_index;
    gctUINT32 pid = _GetProcessID();
    gckGALDEVICE gal_dev = (gckGALDEVICE)drm->dev_private;
    gckDEVICE device;
    gceSTATUS status = gcvSTATUS_OK;

    for (dev_index = 0; dev_index < gal_dev->args.devCount; dev_index++) {
        device = gal_dev->devices[dev_index];

        for (i = 0; i < gcdMAX_GPU_COUNT; ++i) {
            if (device->kernels[i])
                gcmkONERROR(gckKERNEL_AttachProcessEx(device->kernels[i], gcvTRUE, pid));
        }
    }

    file->driver_priv = gcmINT2PTR(pid);

OnError:
    return gcmIS_ERROR(status) ? -ENODEV : 0;
}

static void viv_drm_postclose(struct drm_device *drm, struct drm_file *file)
{
    gctINT i, dev_index;
    gctUINT32 pid = gcmPTR2SIZE(file->driver_priv);
    gckGALDEVICE gal_dev = (gckGALDEVICE)drm->dev_private;
    gckDEVICE device;

    for (dev_index = 0; dev_index < gal_dev->args.devCount; dev_index++) {
        device = gal_dev->devices[dev_index];

        for (i = 0; i < gcdMAX_GPU_COUNT; ++i) {
            if (device->kernels[i])
                gcmkVERIFY_OK(gckKERNEL_AttachProcessEx(device->kernels[i], gcvFALSE, pid));
        }
    }
}

static const struct file_operations viv_drm_fops = {
    .owner              = THIS_MODULE,
    .open               = drm_open,
    .release            = drm_release,
    .unlocked_ioctl     = drm_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl       = drm_compat_ioctl,
#endif
    .poll               = drm_poll,
    .read               = drm_read,
    .llseek             = no_llseek,
};

static struct drm_driver viv_drm_driver = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
    .driver_features    = DRIVER_GEM | DRIVER_RENDER,
#else
    .driver_features    = DRIVER_GEM | DRIVER_PRIME | DRIVER_RENDER,
#endif
    .open               = viv_drm_open,
    .postclose          = viv_drm_postclose,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
    .gem_free_object_unlocked = viv_gem_free_object,
# else
    .gem_free_object          = viv_gem_free_object,
# endif
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0) || LINUX_VERSION_CODE > KERNEL_VERSION(6, 6, 12)
    .prime_handle_to_fd = drm_gem_prime_handle_to_fd,
    .prime_fd_to_handle = drm_gem_prime_fd_to_handle,
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0)
    .gem_prime_export   = viv_gem_prime_export,
#endif
    .gem_prime_import   = viv_gem_prime_import,
    .ioctls             = viv_ioctls,
    .num_ioctls         = DRM_VIV_NUM_IOCTLS,
    .fops               = &viv_drm_fops,
    .name               = "ftg340",
    .desc               = "ftg340 DRM",
    .date               = "20170808",
    .major              = 1,
    .minor              = 0,
};

int
viv_drm_probe(struct device *dev)
{
    int ret = 0;
    gceSTATUS status = gcvSTATUS_OK;
    gckGALDEVICE gal_dev = gcvNULL;
    struct drm_device *drm = gcvNULL;

    gal_dev = (gckGALDEVICE)dev_get_drvdata(dev);
    if (!gal_dev) {
        ret = -ENODEV;
        gcmkONERROR(gcvSTATUS_INVALID_OBJECT);
    }

    drm = drm_dev_alloc(&viv_drm_driver, dev);
    if (IS_ERR(drm)) {
        ret = PTR_ERR(drm);
        gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }
    drm->dev_private = (void *)gal_dev;

    ret = drm_dev_register(drm, 0);
    if (ret)
        gcmkONERROR(gcvSTATUS_GENERIC_IO);

    gal_dev->drm = (void *)drm;

OnError:
    if (gcmIS_ERROR(status)) {
        if (drm)
            drm_dev_unref(drm);
        pr_err("ftg340: Failed to setup drm device.\n");
    }
    return ret;
}

int
viv_drm_remove(struct device *dev)
{
    gckGALDEVICE gal_dev = (gckGALDEVICE)dev_get_drvdata(dev);

    if (gal_dev) {
        struct drm_device *drm = (struct drm_device *)gal_dev->drm;

        drm_dev_unregister(drm);
        drm_dev_unref(drm);
    }

    return 0;
}

#endif
