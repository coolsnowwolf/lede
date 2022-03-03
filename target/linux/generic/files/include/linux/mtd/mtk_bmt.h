#ifndef __MTK_BMT_H
#define __MTK_BMT_H

#ifdef CONFIG_MTD_NAND_MTK_BMT
int mtk_bmt_attach(struct mtd_info *mtd);
void mtk_bmt_detach(struct mtd_info *mtd);
#else
static inline int mtk_bmt_attach(struct mtd_info *mtd)
{
	return 0;
}

static inline void mtk_bmt_detach(struct mtd_info *mtd)
{
}
#endif

#endif
