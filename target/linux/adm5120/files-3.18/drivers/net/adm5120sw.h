/*
 *	Defines for ADM5120 built in ethernet switch driver
 *
 *	Copyright Jeroen Vreeken (pe1rxq@amsat.org), 2005
 *
 *	Values come from ADM5120 datasheet and original ADMtek 2.4 driver,
 *	Copyright ADMtek Inc.
 */

#ifndef _INCLUDE_ADM5120SW_H_
#define _INCLUDE_ADM5120SW_H_

#define SIOCSMATRIX	SIOCDEVPRIVATE
#define SIOCGMATRIX	(SIOCDEVPRIVATE + 1)
#define SIOCGADMINFO	(SIOCDEVPRIVATE + 2)

struct adm5120_sw_info {
	u16	magic;
	u16	ports;
	u16	vlan;
};

#endif /* _INCLUDE_ADM5120SW_H_ */
