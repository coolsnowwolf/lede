/*
 * Copyright (c) 2012 Quantenna Communications, Inc.
 */

#ifndef _QTN_UC_COMM_H
#define _QTN_UC_COMM_H

#define MAC_UNITS		1

#if defined(TOPAZ_PLATFORM) && defined(TOPAZ_128_NODE_MODE)
#define QTN_NCIDX_MAX			128
#define QTN_NODE_TBL_SIZE_LHOST		118
#define QTN_NODETID_NODE_SHIFT		7
#else
#define QTN_NCIDX_MAX			64
#define QTN_NODE_TBL_SIZE_LHOST		56
#define QTN_NODETID_NODE_SHIFT		6
#endif
#define QTN_MAX_BSS_VAPS		8
#define QTN_MAX_WDS_VAPS		8
#define QTN_MAX_VAPS			((QTN_MAX_BSS_VAPS) + (QTN_MAX_WDS_VAPS))
#define QTN_NODE_TBL_MUC_HEADRM		3 /* Allow for delayed delete on MUC */
#define QTN_NODE_TBL_SIZE_MUC		((QTN_NODE_TBL_SIZE_LHOST) + (QTN_NODE_TBL_MUC_HEADRM))
#define QTN_ASSOC_LIMIT			((QTN_NODE_TBL_SIZE_LHOST) - (QTN_MAX_VAPS))

#endif // #ifndef _QTN_UC_COMM_H

