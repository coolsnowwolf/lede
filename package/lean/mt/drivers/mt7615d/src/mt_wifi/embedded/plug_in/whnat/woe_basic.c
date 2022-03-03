/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: wifi_offload
	whnat_basic.c
*/


#include "woe_basic.h"


static char whnat_log_level = WHNAT_DBG_ERR;


/*
*
*/
char whnat_log_get(void)
{
	return whnat_log_level;
}

/*
*
*/
void whnat_log_set(char log)
{
	whnat_log_level = log;
}

/*
*
*/
void whnat_dma_buf_free(struct platform_device *dev, struct whnat_dma_buf *dma_buf)
{
	if (!dev || !dma_buf->alloc_va)
		return;

	memset(dma_buf->alloc_va, 0, dma_buf->alloc_size);
	dma_free_coherent(&dev->dev, dma_buf->alloc_size, dma_buf->alloc_va, dma_buf->alloc_pa);
	memset(dma_buf, 0, sizeof(struct whnat_dma_buf));
}

/*
*
*/
int whnat_dma_buf_alloc(struct platform_device *dev, struct whnat_dma_buf *dma_buf, UINT size)
{
	dma_addr_t paddr;
	void *vaddr;

	dma_buf->alloc_size = size;
	vaddr = dma_alloc_coherent(&dev->dev, size, &paddr, GFP_KERNEL);

	if (vaddr == NULL) {
		WHNAT_DBG(WHNAT_DBG_ERR, "Failed to allocate a big buffer,size=%d\n", size);
		return -1;
	}

	dma_buf->alloc_pa = paddr;
	dma_buf->alloc_va = vaddr;
	/*zero init this memory block*/
	memset(dma_buf->alloc_va, 0, size);
	return 0;
}

/*
*
*/
void whnat_dump_dmabuf(struct whnat_dma_buf *buf)
{
	WHNAT_DBG(WHNAT_DBG_OFF, "AllocPA\t:%pad\n", &buf->alloc_pa);
	WHNAT_DBG(WHNAT_DBG_OFF, "AllocVa\t:0x%p\n", buf->alloc_va);
	WHNAT_DBG(WHNAT_DBG_OFF, "Size\t:%lu\n", buf->alloc_size);
}


/*
*
*/
void whnat_dump_dmacb(struct whnat_dma_cb *cb)
{

	WHNAT_DBG(WHNAT_DBG_OFF, "AllocPA\t: %pad\n", &cb->alloc_pa);
	WHNAT_DBG(WHNAT_DBG_OFF, "AllocVa\t: %p\n", cb->alloc_va);
	WHNAT_DBG(WHNAT_DBG_OFF, "Size\t: %lu\n", cb->alloc_size);
	WHNAT_DBG(WHNAT_DBG_OFF, "Pkt\t: %p\n", cb->pkt);
	WHNAT_DBG(WHNAT_DBG_OFF, "token_id\t: %d\n", cb->token_id);
}

/*
*
*/
void whnat_dump_raw(char *str, unsigned char *va, unsigned int size)
{
	unsigned char *pt;
	char buf[512] = "";
	unsigned int len = 0;
	int x;

	pt = va;
	WHNAT_DBG(WHNAT_DBG_OFF, "%s: %p, len = %d\n", str, va, size);

	for (x = 0; x < size; x++) {
		if (x % 16 == 0) {
			sprintf(buf+len, "\n0x%04x : ", x);
			len = strlen(buf);
		}

		sprintf(buf+len, "%02x ", ((unsigned char)pt[x]));
		len = strlen(buf);
	}

	WHNAT_DBG(WHNAT_DBG_OFF, "%s\n", buf);
}

long whnat_str_tol(const char *str, char **endptr, int base)
{
	return simple_strtol(str, endptr, base);
}
