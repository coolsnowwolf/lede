/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2019 - 2020
 *
 * Richard van Schagen <vschagen@cs.com>
 */

#include "eip93-common.h"
#include "eip93-core.h"

inline void *mtk_ring_next_wptr(struct mtk_device *mtk,
						struct mtk_desc_ring *ring)
{
	void *ptr = ring->write;

	if ((ring->write == ring->read - ring->offset) ||
		(ring->read == ring->base && ring->write == ring->base_end))
		return ERR_PTR(-ENOMEM);

	if (ring->write == ring->base_end)
		ring->write = ring->base;
	else
		ring->write += ring->offset;

	return ptr;
}

inline void *mtk_ring_next_rptr(struct mtk_device *mtk,
						struct mtk_desc_ring *ring)
{
	void *ptr = ring->read;

	if (ring->write == ring->read)
		return ERR_PTR(-ENOENT);

	if (ring->read == ring->base_end)
		ring->read = ring->base;
	else
		ring->read += ring->offset;

	return ptr;
}

inline int mtk_put_descriptor(struct mtk_device *mtk,
					struct eip93_descriptor_s desc)
{
	struct eip93_descriptor_s *cdesc;
	struct eip93_descriptor_s *rdesc;

	spin_lock(&mtk->ring->write_lock);
	cdesc = mtk_ring_next_wptr(mtk, &mtk->ring->cdr);

	if (IS_ERR(cdesc))
		return -ENOENT;

	rdesc = mtk_ring_next_wptr(mtk, &mtk->ring->rdr);

	if (IS_ERR(rdesc)) {
		spin_lock(&mtk->ring->write_lock);
		return -ENOENT;
	}

	memset(rdesc, 0, sizeof(struct eip93_descriptor_s));
	memcpy(cdesc, &desc, sizeof(struct eip93_descriptor_s));

	spin_unlock(&mtk->ring->write_lock);

	return 0;
}

inline void *mtk_get_descriptor(struct mtk_device *mtk)
{
	struct eip93_descriptor_s *cdesc;

	cdesc = mtk_ring_next_rptr(mtk, &mtk->ring->cdr);
	if (IS_ERR(cdesc)) {
		dev_err(mtk->dev, "Cant get Cdesc");
		return cdesc;
	}

	return mtk_ring_next_rptr(mtk, &mtk->ring->rdr);
}
