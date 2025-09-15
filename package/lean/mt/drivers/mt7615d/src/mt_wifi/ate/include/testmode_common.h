
#ifndef _TESTMODE_COMMON_H
#define _TESTMODE_COMMON_H

#ifndef COMPOS_TESTMODE_WIN
enum _EPQID {
	EP4QID = 0,
	EP5QID,
	EP6QID,
	EP7QID,
	EP8QID = 8,
	EP9QID,
	EPALLQID
};
#endif
/*test mode common*/
/*  */
/* Scheduler Register 4 (offset: 0x0594, default: 0x0000_0000) */
/* Note: */
/* 1. DW : double word */
/*  */
union _SCHEDULER_REGISTER4 {
	struct	{
		/* DW0 */
		ULONG   ForceQid : 4;
		ULONG   ForceMode : 1;
		ULONG   BypassMode : 1;
		ULONG   HybridMode : 1;
		ULONG   RgPredictNoMask : 1;
		ULONG   RgResetScheduler : 1;
		ULONG   RgDoneClearHeader : 1;
		ULONG   SwMode : 1;
		ULONG   Reserves0 : 5;
		ULONG   RgRateMap : 14;
		ULONG   Reserves1 : 2;
	}	Default;
	ULONG			word;
};

/* MT7637 for Band display */
#define MT7367_RO_AGC_DEBUG_2	(WF_PHY_BASE + 0x0584)
#define CR_ACI_HIT		(WF_PHY_BASE + 0x0594)
/* MT7637 for Band display end */

/*~test mode common*/

#endif /* _TESTMODE_COMMON_H */
