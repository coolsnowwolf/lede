/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_PHYTIUM_HW_REGS
#define __LINUX_PHYTIUM_HW_REGS

#define USBIEN			0x198
#define USBIEN_SUDAVIE		BIT(0)
#define USBIEN_SOFIE		BIT(1)
#define USBIEN_SUTOKIE		BIT(2)
#define USBIEN_SUSPIE		BIT(3)
#define USBIEN_URESIE		BIT(4)
#define USBIEN_HSPEEDIE		BIT(5)
#define USBIEN_LPMIE		BIT(7)

#define USBCS			0x1a3
#define USBCS_LSMODE		BIT(0)
#define USBCS_LPMNYET		BIT(1)
#define USBCS_SIGSUME		BIT(5)
#define USBCS_DISCON		BIT(6)
#define USBCS_WAKESRC		BIT(7)

#define USBIR_SOF		BIT(1)
#define USBIR_SUSP		BIT(3)
#define USBIR_URES		BIT(4)
#define USBIR_HSPEED		BIT(5)

#define USBIR_SUDAV		BIT(0)
#define USBIR_SUTOK		BIT(2)
#define USBIR_LPMIR		BIT(7)

#define OTGIRQ_IDLEIRQ		BIT(0)
#define OTGIRQ_SRPDETIRQ	BIT(1)
#define OTGIRQ_CONIRQ		BIT(2)
#define OTGIRQ_LOCSOFIRQ	BIT(2)
#define OTGIRQ_VBUSERRIRQ	BIT(3)
#define OTGIRQ_PERIPHIRQ	BIT(4)
#define OTGIRQ_IDCHANGEIRQ	BIT(5)
#define OTGIRQ_HOSTDISCON	BIT(6)
#define OTGIRQ_BSE0SRPIRQ	BIT(7)

#define OTGCTRL_BUSREQ		BIT(0)
#define OTGCTRL_ABUSDROP	BIT(1)
#define OTGCTRL_ASETBHNPEN	BIT(2)
#define OTGCTRL_BHNPEN		BIT(3)
#define OTGCTRL_SRPVBUSDETEN	BIT(4)
#define OTGCTRL_SRPDATDETEN	BIT(5)
#define OTGCTRL_FORCEBCONN	BIT(7)

#define OTGSTATUS_ID		BIT(6)

#define ENDPRST_EP		0x0f
#define ENDPRST_IO_TX		BIT(4)
#define ENDPRST_TOGRST		BIT(5)
#define ENDPRST_FIFORST		BIT(6)
#define ENDPRST_TOGSETQ		BIT(7)

#define FIFOCTRL_EP		0x0f
#define FIFOCTRL_IO_TX		BIT(4)
#define FIFOCTRL_FIFOAUTO	BIT(5)

#define SPEEDCTRL_LS		BIT(0)
#define SPEEDCTRL_FS		BIT(1)
#define SPEEDCTRL_HS		BIT(2)
#define SPEEDCTRL_HSDISABLE	BIT(7)

#define CON_TYPE_CONTROL	0x00
#define CON_TYPE_ISOC		0x04
#define CON_TYPE_BULK		0x08
#define CON_TYPE_INT		0x0C
#define CON_TYPE_ISOC_1_ISOD	0x00
#define CON_TYPE_ISOC_2_ISOD	0x10
#define CON_TYPE_ISOC_3_ISOD	0x20
#define CON_STALL		0x40
#define CON_VAL			0x80

#define ERR_TYPE		0x1c
#define ERR_COUNT		0x03
#define ERR_RESEND		BIT(6)
#define ERR_UNDERRIEN		BIT(7)

#define ERR_NONE			0
#define ERR_CRC				1
#define ERR_DATA_TOGGLE_MISMATCH	2
#define ERR_STALL			3
#define ERR_TIMEOUT			4
#define ERR_PID				5
#define ERR_TOO_LONG_PACKET		6
#define ERR_DATA_UNDERRUN		7

#define EP0CS_HCSETTOGGLE	BIT(6)
#define EP0CS_HCSET		BIT(4)
#define EP0CS_RXBUSY_MASK	BIT(3)
#define EP0CS_TXBUSY_MASK	BIT(2)
#define EP0CS_STALL		BIT(0)
#define EP0CS_HSNAK		BIT(1)
#define EP0CS_DSTALL		BIT(4)
#define EP0CS_CHGSET		BIT(7)

#define CS_ERR		0x01
#define CS_BUSY		0x02
#define CS_NPAK		0x0c
#define CS_NPAK_OFFSET	0x02
#define CS_AUTO		0x10

#define CON_BUF_SINGLE	0x00
#define CON_BUF_DOUBLE	0x01
#define CON_BUF_TRIPLE	0x02
#define CON_BUF_QUAD	0x03
#define CON_BUF	0x03

struct HW_REGS {
	uint8_t ep0Rxbc;        /*address 0x00*/
	uint8_t ep0Txbc;        /*address 0x01*/
	uint8_t ep0cs;          /*address 0x02*/
	int8_t reserved0;       /*address 0x03*/
	uint8_t lpmctrll;       /*address 0x04*/
	uint8_t lpmctrlh;       /*address 0x05*/
	uint8_t lpmclock;
	uint8_t ep0fifoctrl;
	struct ep {             /*address 0x08*/
		uint16_t rxbc;        //outbc (hcinbc)
		uint8_t rxcon;
		uint8_t rxcs;
		uint16_t txbc;         //inbc  (hcoutbc
		uint8_t txcon;
		uint8_t txcs;
	} ep[15];
	uint8_t reserved1[4];
	uint32_t fifodat[15];   /*address 0x84*/
	uint8_t ep0ctrl;        /*address 0xC0*/
	uint8_t tx0err;         /*address 0xC1*/
	uint8_t reserved2;
	uint8_t rx0err;         /*address 0xC3*/
	struct epExt {
		uint8_t txctrl;
		uint8_t txerr;
		uint8_t rxctrl;
		uint8_t rxerr;
	} epExt[15];
	uint8_t ep0datatx[64]; /*address 0x100*/
	uint8_t ep0datarx[64]; /*address 0x140*/
	uint8_t setupdat[8];    /*address 0x180*/
	uint16_t txirq;         /*address 0x188*/
	uint16_t rxirq;         /*address 0x18A*/
	uint8_t usbirq;         /*address 0x18C*/
	uint8_t reserved4;
	uint16_t rxpngirq;      /*address 0x18E*/
	uint16_t txfullirq;     /*address 0x190*/
	uint16_t rxemptirq;     /*address 0x192*/
	uint16_t txien;         /*address 0x194*/
	uint16_t rxien;         /*address 0x196*/
	uint8_t usbien;         /*address 0x198*/
	uint8_t reserved6;
	uint16_t rxpngien;      /*address 0x19A*/
	uint16_t txfullien;     /*address 0x19C*/
	uint16_t rxemptien;     /*address 0x19E*/
	uint8_t usbivect;       /*address 0x1A0*/
	uint8_t fifoivect;      /*address 0x1A1*/
	uint8_t endprst;        /*address 0x1A2*/
	uint8_t usbcs;          /*address 0x1A3*/
	uint16_t frmnr;         /*address 0x1A4*/
	uint8_t fnaddr;         /*address 0x1A6*/
	uint8_t clkgate;        /*address 0x1A7*/
	uint8_t fifoctrl;       /*address 0x1A8*/
	uint8_t speedctrl;      /*address 0x1A9*/
	uint8_t reserved8[1];   /*address 0x1AA*/
	uint8_t portctrl;       /*address 0x1AB*/
	uint16_t hcfrmnr;       /*address 0x1AC*/
	uint16_t hcfrmremain;   /*address 0x1AE*/
	uint8_t reserved9[4];   /*address 0x1B0*/
	uint16_t rxerrirq;      /*address 0x1B4*/
	uint16_t txerrirq;      /*address 0x1B6*/
	uint16_t rxerrien;      /*address 0x1B8*/
	uint16_t txerrien;      /*address 0x1BA*/
	/*OTG extension*/
	uint8_t otgirq;         /*address 0x1BC*/
	uint8_t otgstate;       /*address 0x1BD*/
	uint8_t otgctrl;        /*address 0x1BE*/
	uint8_t otgstatus;      /*address 0x1BF*/
	uint8_t otgien;         /*address 0x1C0*/
	uint8_t taaidlbdis;     /*address 0x1C1*/
	uint8_t tawaitbcon;     /*address 0x1C2*/
	uint8_t tbvbuspls;      /*address 0x1C3*/
	uint8_t otg2ctrl;       /*address 0x1C4*/
	uint8_t reserved10[2];  /*address 0x1C5*/
	uint8_t tbvbusdispls;   /*address 0x1C7*/
	uint8_t traddr;         /*address 0x1C8*/
	uint8_t trwdata;        /*address 0x1C9*/
	uint8_t trrdata;        /*address 0x1CA*/
	uint8_t trctrl;         /*address 0x1CB*/
	uint16_t isoautoarm;    /*address 0x1CC*/
	uint8_t adpbc1ien;      /*address 0x1CE*/
	uint8_t adpbc2ien;      /*address 0x1CF*/
	uint8_t adpbcctr0;      /*address 0x1D0*/
	uint8_t adpbcctr1;      /*address 0x1D1*/
	uint8_t adpbcctr2;      /*address 0x1D2*/
	uint8_t adpbc1irq;      /*address 0x1D3*/
	uint8_t adpbc0status;   /*address 0x1D4*/
	uint8_t adpbc1status;   /*address 0x1D5*/
	uint8_t adpbc2status;   /*address 0x1D6*/
	uint8_t adpbc2irq;      /*address 0x1D7*/
	uint16_t isodctrl;      /*address 0x1D8*/
	uint8_t reserved11[2];
	uint16_t isoautodump;   /*address 0x1DC*/
	uint8_t reserved12[2];
	uint8_t ep0maxpack;     /*address 0x1E0*/
	uint8_t reserved13;
	uint16_t rxmaxpack[15]; /*address 0x1E2*/
	struct rxsoftimer { /*address 0x200 to  0x23F*/
		uint16_t timer;
		uint8_t reserved;
		uint8_t ctrl;
	} rxsoftimer[16];

	struct  txsoftimer { /*address 0x240 to  0x27F*/
		uint16_t timer;
		uint8_t reserved;
		uint8_t ctrl;
	} txsoftimer[16];
	uint8_t reserved14[132];
	struct rxstaddr {       /*address 0x304*/
		uint16_t addr;
		uint16_t reserved;
	} rxstaddr[15];
	uint8_t reserved15[4];
	struct txstaddr {       /*address 0x344*/
		uint16_t addr;
		uint16_t reserved;
	} txstaddr[15];
	int8_t reserved16[4];	/*address 0x380*/
	struct irqmode {	/*address 0x384*/
		int8_t inirqmode;
		int8_t reserved21;
		int8_t outirqmode;
		int8_t reserved22;
	} irqmode[15];
	/*The Microprocessor control*/
	uint8_t cpuctrl;         /*address 0x3C0*/
	int8_t reserved17[15];
	/*The debug counters and workarounds*/
	uint8_t debug_rx_bcl;    /*address 0x3D0*/
	uint8_t debug_rx_bch;    /*address 0x3D1*/
	uint8_t debug_rx_status; /*address 0x3D2*/
	uint8_t debug_irq;       /*address 0x3D3*/
	uint8_t debug_tx_bcl;    /*address 0x3D4*/
	uint8_t debug_tx_bch;    /*address 0x3D5*/
	uint8_t debug_tx_status; /*address 0x3D6*/
	uint8_t debug_ien;       /*address 0x3D7*/
	uint8_t phywa_en;        /*address 0x3D8*/
	/*endian*/
	uint8_t wa1_cnt;       /*address 0x3D9*/
	int8_t reserved18[2];  /*address 0x3DA*/
	uint8_t endian_sfr_cs; /*address 0x3DC*/
	int8_t reserved19[2];    /*address 0x3DD*/
	uint8_t endian_sfr_s;  /*address 0x3DF*/
	int8_t reserved20[2];    /*address 0x3E0*/
	uint16_t txmaxpack[15]; /*address 0x3E2*/
};

struct CUSTOM_REGS {
	uint32_t secure_ctrl;	/*address 0x80000*/
	uint32_t secsid_atst;	/*address 0x80004*/
	uint32_t nsaid_smmuid;	/*address 0x80008*/
	uint32_t ace;		/*address 0x8000c*/
	uint32_t wakeup;	/*address 0x80010*/
	uint32_t debug;		/*address 0x80014*/
};

struct VHUB_REGS {
	uint32_t gen_cfg;	/*address 0x00*/
	uint32_t gen_st;	/*address 0x04*/
	uint32_t bc_cfg;	/*address 0x08*/
	uint32_t bc_st;		/*address 0x0c*/
	uint32_t adp_cfg;	/*address 0x10*/
	uint32_t adp_st;	/*address 0x14*/
	uint32_t dbg_cfg;	/*address 0x18*/
	uint32_t dbg_st;	/*address 0x1c*/
	uint32_t utmip_cfg;	/*address 0x20*/
	uint32_t utmip_st;	/*address 0x24*/
};

struct DMARegs {
	uint32_t conf;      /*address 0x400*/
	uint32_t sts;       /*address 0x404*/
	uint32_t reserved5[5];
	uint32_t ep_sel;    /*address 0x41C*/
	uint32_t traddr;    /*address 0x420*/
	uint32_t ep_cfg;    /*address 0x424*/
	uint32_t ep_cmd;    /*address 0x428*/
	uint32_t ep_sts;    /*address 0x42c*/
	uint32_t ep_sts_sid;/*address 0x430*/
	uint32_t ep_sts_en; /*address 0x434*/
	uint32_t drbl;      /*address 0x438*/
	uint32_t ep_ien;    /*address 0x43C*/
	uint32_t ep_ists;   /*address 0x440*/
};

#endif
