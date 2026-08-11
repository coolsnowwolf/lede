#ifndef _FALLBACK_SPROM_H
#define _FALLBACK_SPROM_H

int __init ssb_fbs_register(void);
int ssb_get_fallback_sprom(struct ssb_bus *dev, struct ssb_sprom *out);

#endif /* _FALLBACK_SPROM_H */
