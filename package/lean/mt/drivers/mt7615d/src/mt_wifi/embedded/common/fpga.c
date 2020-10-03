/*

*/


VOID fpga_tr_stop(RTMP_ADAPTER *pAd, UCHAR stop)
{
	UINT32 mac_val;
	/* enable/disable tx/rx*/
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &mac_val);

	switch (stop) {
	case 3:  /* stop tx + rx */
		mac_val &= (~0xc);
		break;

	case 2: /* stop rx */
		mac_val &= (~0x8);
		break;

	case 1: /* stop tx */
		mac_val &= (~0x4);
		break;

	case 4:
	default:
		mac_val |= 0x0c;
		break;
	}

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, mac_val);
}


