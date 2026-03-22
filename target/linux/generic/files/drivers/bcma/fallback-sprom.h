#ifndef _FALLBACK_SPROM_H
#define _FALLBACK_SPROM_H

int __init bcma_fbs_register(void);
int bcma_get_fallback_sprom(struct bcma_bus *dev, struct ssb_sprom *out);

#endif /* _FALLBACK_SPROM_H */
