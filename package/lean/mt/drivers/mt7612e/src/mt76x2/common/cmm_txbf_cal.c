/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_txbf_cal.c

	Abstract:
	Tx Beamforming calibration and profile related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Shiang     2010/07/12
*/

#include "rt_config.h"


/* 
	iAtan2 - fixed point atan2. Returns +/-pi. Scaled so pi=0x1000
		Code was taken from MyCordic:
			int MyCordic(int y, int x, int shift, int iter, int *alpha)
			Parameters are hard coded so it's equivalent to MyCordic(y, x, 0, 11, alpha10);
*/
static int iAtan2(int y, int x)
{
	int z = 0, xtemp, i;
	int onepi, halfpi;
	int shift=0;
	int iter=11;
	static int alpha[11] = {0x400, 0x25c, 0x13f, 0x0a2,
				0x051, 0x028, 0x014, 0x00a,
				0x005, 0x002, 0x001};
	
	onepi = (alpha[0]<<2), 
	halfpi = (alpha[0]<<1);
 
    if (x == 0) {
		if (y == 0)
              z = 0;
        else if (y > 0)
                z = halfpi;
        else
            z = -halfpi;
		}
    else if ((x < 0) && (y == 0)){
            z = -onepi;
		}
    else if ((x > 0) && (y == 0)){
            z = 0;
		}
	else{
		x <<= shift;
		y <<= shift;
		xtemp = x; 
		if ((x < 0) && (y > 0)){
            x = y;
            y = -xtemp;
            z = halfpi;
			}
		else if ((x < 0) && (y < 0)){
            x = -y;
            y = xtemp;
            z = -halfpi;
			}      
		for (i = 0; i < iter; i++){
			/* printf("%d %d %x\n", x, y, z); */
			if (y == 0)
				break;
			if (y < 0) {
				z -= alpha[i];
				xtemp = x - (y>>i);
				y = y + (x>>i);
				x = xtemp;
			}
			else {
				z += alpha[i];
				xtemp = x + (y>>i);
				y = y - (x>>i);
				x = xtemp;
				}
		}
	}

	if (z == alpha[0]*4)
		z = -(alpha[0]*4);
 
	return z;
}


/*
	isqrt - fixed point sqrt
		x - unsigned value
*/
static UINT32 isqrt (UINT32 x) 
{ 
	UINT32 base, y;

	if (x &      0xF0000000)
		base = 1<<15;
	else if (x & 0x0F000000)
		base = 1<<13;
	else if (x & 0x00F00000)
		base = 1<<11;
	else if (x & 0x000F0000)
		base = 1<<9;
	else
		base = 1<<7;

    y = 0; 
    while (base) { 
		y += base; 
		if  ((y * y) > x)
			y -= base;
		base >>= 1;
    }
    return y; 
} 


/*
	icexp - fixed point complex exponential
		phase - 0 to 255 representing 0 to 2pi
		return cos and sin in 1p10 format
*/
static void icexp(short c[2], int phase)
{
	/* cosine table generated with Matlab: round(1024*cos(2*pi*[0:255]/256) */
	static short cosTable[256] = {
		1024, 1024, 1023, 1021, 1019, 1016, 1013, 1009,
		1004, 999, 993, 987, 980, 972, 964, 955,
		946, 936, 926, 915, 903, 891, 878, 865,
		851, 837, 822, 807, 792, 775, 759, 742,
		724, 706, 688, 669, 650, 630, 610, 590,
		569, 548, 526, 505, 483, 460, 438, 415,
		392, 369, 345, 321, 297, 273, 249, 224,
		200, 175, 150, 125, 100, 75, 50, 25,
		0, -25, -50, -75, -100, -125, -150, -175,
		-200, -224, -249, -273, -297, -321, -345, -369,
		-392, -415, -438, -460, -483, -505, -526, -548,
		-569, -590, -610, -630, -650, -669, -688, -706,
		-724, -742, -759, -775, -792, -807, -822, -837,
		-851, -865, -878, -891, -903, -915, -926, -936,
		-946, -955, -964, -972, -980, -987, -993, -999,
		-1004, -1009, -1013, -1016, -1019, -1021, -1023, -1024,
		-1024, -1024, -1023, -1021, -1019, -1016, -1013, -1009,
		-1004, -999, -993, -987, -980, -972, -964, -955,
		-946, -936, -926, -915, -903, -891, -878, -865,
		-851, -837, -822, -807, -792, -775, -759, -742,
		-724, -706, -688, -669, -650, -630, -610, -590,
		-569, -548, -526, -505, -483, -460, -438, -415,
		-392, -369, -345, -321, -297, -273, -249, -224,
		-200, -175, -150, -125, -100, -75, -50, -25,
		0, 25, 50, 75, 100, 125, 150, 175,
		200, 224, 249, 273, 297, 321, 345, 369,
		392, 415, 438, 460, 483, 505, 526, 548,
		569, 590, 610, 630, 650, 669, 688, 706,
		724, 742, 759, 775, 792, 807, 822, 837,
		851, 865, 878, 891, 903, 915, 926, 936,
		946, 955, 964, 972, 980, 987, 993, 999,
		1004, 1009, 1013, 1016, 1019, 1021, 1023, 1024};
	c[0] = cosTable[phase & 0xFF];
	c[1] = cosTable[(phase-64) & 0xFF];
}


/*
	icMult - fixed point complex multiply
		r = a*b
*/
static void icMult(INT32 r[2], INT32 a[2], INT32 b0, INT32 b1)
{
	INT32 t;
	t = a[0]*b0 - a[1]*b1;
	r[1] = a[0]*b1 + a[1]*b0;
	r[0] = t;
}


/*
	------------ DIVIDER AND LNA CALIBRATION --------
*/
typedef	struct {
	LONG	i:8;
	LONG	q:8;
}	COMPLEX_VALUE;		/* Signed 8-bit complex values */

#define CALC_LENGTH		1024				/* Number of samples used to perform phase calculation for LNA or Divider Calibration */
#define CALC_LENGTH_DC	(CALC_LENGTH+512)	/* Number of samples used for DC removal */
#define MAX_CAPTURE_LENGTH		4096			/* Maximum number of samples to capture */
#define DIVCAL_CAPTURE_LENGTH	(CALC_LENGTH+1024)	/* Length of capture for Divider or LNA Calibration */

#define FIXED_M_PI		0x1000						/* Scaling for fixed point PI */
#define DEG(rad)		(radToDeg180(rad-FIXED_M_PI)+180)	/* Convert fixed radians (0x1000=pi) to degrees range [0 360) */
#define DEG180(rad)		radToDeg180(rad)		/* Convert fixed radians (0x1000=pi) to degrees range [-180 180) */

#define BYTE_PHASE_SHIFT		5					/* Shift to convert from byte phase (0x80=pi) to normal phase (0x1000=pi) */
#define CONVERT_TO_BYTE_PHASE(p)	(int)(((p)+(1<<(BYTE_PHASE_SHIFT-1)))>>BYTE_PHASE_SHIFT)	/* Convert from normal phase to byte phase */

#define R65_LNA_LOW		0x4
#define R65_LNA_MID		0x8
#define R65_LNA_HIGH	0xC


/*
	radMod2pi - converts angle in radians to the range [-pi pi)
*/
static LONG radMod2pi(LONG a)
{
	while (a < -FIXED_M_PI)
		a += 2*FIXED_M_PI;
	while (a >= FIXED_M_PI)
		a -= 2*FIXED_M_PI;

	return a;
}


/*
	radToDeg180 - converts angle in radians to the deg range [-180 180)
*/
static int radToDeg180(LONG rad)
{
	return (int)(radMod2pi(rad)*180/FIXED_M_PI);
}


/*
	avgPhase - computes the average phase.
		Phase is adjusted so all values are within the range mPhase[0] +/-pi
			mPhase - values to average (radians)
			pLength - number of values to average
		return average
*/
static LONG avgPhase(LONG mPhase[], int pLength)
{
	int i;
	short cval[2];
	LONG sumCos = 0, sumSin=0;
	for (i=0; i<pLength; i++) {
		icexp(cval, CONVERT_TO_BYTE_PHASE(mPhase[i]));
		sumCos += cval[0];
		sumSin += cval[1];
	}

	return iAtan2(sumSin, sumCos);
}


typedef
	COMPLEX_VALUE (*PCAP_IQ_DATA)[3];	/* CAP_IQ_DATA - Buffer to hold I/Q data for three RX chains */


/*
	RemoveDC - calculate mean and subtract. Return peak values
		peak - used to return the peak value of the three RX chains
		iqData - pointer to array of I/Q data for the three RX chains. DC is removed from the samples
		dataLength - number of samples in iqData
*/
static void RemoveDC(
	IN int peak[3],
	IN COMPLEX_VALUE (*iqData)[3],
	IN int dataLength)
{
	int i, j;
	int dcI[3] = {0, 0, 0};
	int dcQ[3] = {0, 0, 0};

	/* Calculate DC offset for each RX chain */
	for (i=0; i<dataLength; i++) {
		for (j=0; j<3; j++) {
			dcI[j] += iqData[i][j].i;
			dcQ[j] += iqData[i][j].q;
		}
	}

	for (j=0; j<3; j++) {
		dcI[j] /= dataLength;
		dcQ[j] /= dataLength;
	}

	/* Subtract DC and find peak */
	peak[0] = peak[1] = peak[2] = 0;

	for (i=0; i<dataLength; i++) {
		for (j=0; j<3; j++) {
			int sati = iqData[i][j].i - dcI[j];
			int satq = iqData[i][j].q - dcQ[j];

			/* Saturate */
			if (sati > 127)
				sati = 127;
			else if (sati < -128)
				sati = -128;
			iqData[i][j].i = sati;

			if (satq > 127)
				satq = 127;
			else if (satq < -128)
				satq = -128;
			iqData[i][j].q = satq;

			/* Record peak */
			if (peak[j] < iqData[i][j].i)
				peak[j] = iqData[i][j].i;
			if (peak[j] < iqData[i][j].q)
				peak[j] = iqData[i][j].q;
		}
	}
}


/*
	CalcRFCalPhase - process RF calibration to calculate phase of the three channels
		Parameters:
			phase - returns the phase of each channel. Fixed point value scaled so 0x1000 = PI
			avgI, avgQ - returns the avg I/Q of each channel. Implied scale factor of 256
			peak - returns the peak value of each channel after DC removal
			iqData - the input I/Q data for three channels. DC is removed.
			relPhase - If true it returns phase relative to Ant1. Otherwise it returns the 
						phase relative to the reference signal.
			actTx - index of an active TX chain, used to detect start of signal
*/
static void CalcRFCalPhase(
	OUT LONG phase[3],
	OUT int avgI[3],
	OUT int avgQ[3],
	OUT int peak[3],
	IN COMPLEX_VALUE (*iqData)[3],
	IN BOOLEAN relPhase,
	IN int actTx)
{
	int i, j;
	LONG sumI[3], sumQ[3];
	static CHAR refSignal[64] = {	/* round(sin(-[0:63]*6*pi/64)*127) - three cycles per 64 samples */
		0, -37, -71, -98, -117, -126, -125, -112,
		-90, -60, -25, 12, 49, 81, 106, 122,
		127, 122, 106, 81, 49, 12, -25, -60,
		-90, -112, -125, -126, -117, -98, -71, -37,
		0, 37, 71, 98, 117, 126, 125, 112,
		90, 60, 25, -12, -49, -81, -106, -122,
		-127, -122, -106, -81, -49, -12, 25, 60,
		90, 112, 125, 126, 117, 98, 71, 37};


	/* Skip the first 200 samples to avoid the transient at the beginning */
	iqData += 200;

	/* Remove DC offset to help with low signal levels */
	RemoveDC(peak, iqData, CALC_LENGTH_DC);

	/* Search active channel to find sample with abs>12 */
	for (i=0; i<(CALC_LENGTH_DC-CALC_LENGTH); i++, iqData++) {
		if ((iqData[0][actTx].i*iqData[0][actTx].i + iqData[0][actTx].q*iqData[0][actTx].q) >= 144)
			break;
	}

	/* Move in 16 samples */
	iqData += 16;

	/* Sum the I and Q then calculate the angle of the sum */
	sumI[0] = sumI[1] = sumI[2] = 0;
	sumQ[0] = sumQ[1] = sumQ[2] = 0;

	for (i=0; i<CALC_LENGTH; i++) {
		/* Either calculate the phase relative to Ant1 or phase relative to reference */
		if (relPhase) {
			sumQ[0] += -iqData[i][0].i*iqData[i][1].q + iqData[i][0].q*iqData[i][1].i;
			sumI[0] +=  iqData[i][0].i*iqData[i][1].i + iqData[i][0].q*iqData[i][1].q;
			sumQ[2] += -iqData[i][2].i*iqData[i][1].q + iqData[i][2].q*iqData[i][1].i;
			sumI[2] +=  iqData[i][2].i*iqData[i][1].i + iqData[i][2].q*iqData[i][1].q;
		}
		else {
			int cval= refSignal[(i+16) % 64];
			int sval= refSignal[i % 64];
			for (j=0; j<3; j++) {
				sumQ[j] += -iqData[i][j].i*sval + iqData[i][j].q*cval;
				sumI[j] +=  iqData[i][j].i*cval + iqData[i][j].q*sval;
			}
		}
	}


	for (i=0; i<3; i++) {
		if (relPhase && i==1) {
			phase[i] = 0;
			avgI[i] = avgQ[i] = 0;
		}
		else {
		phase[i] = iAtan2(sumQ[i]>>6, sumI[i]>>6);
			/* Multiplication by refSignal added a scale factor of 128. Shift left by 1 for 256 scale factor */
			avgI[i] = (sumI[i]<<1)/CALC_LENGTH;
			avgQ[i] = (sumQ[i]<<1)/CALC_LENGTH;
		}
	}
}


#ifdef MT76x2
static VOID CalcDividerPhase(
	IN  PRTMP_ADAPTER	pAd,
	IN  BOOLEAN agcEnable,
	IN  UCHAR *preVGAGainIdx,
	OUT UCHAR *mPhase0
	)
{
	UCHAR  channel = pAd->CommonCfg.Channel;
	UINT32 phaseCaliStatus, phaseCaliResult, value32;
	UCHAR  divPhCalPath[2]={RX0TX0, RX1TX1};
	UCHAR  timeOutCount, AGCtimeOutCount;
	LONG   mCalPhase0[2];
	LONG   avgIData, avgQData;
	UCHAR  peakI[2];
	UCHAR   VGAGainIdx[2];
	BOOLEAN gBandFlg = FALSE;
    UINT32  agcLoopCnt = 20;	
    INT     i, RF_Path_n = 0;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		channel = pAd->ate.Channel;
#endif /* RALINK_ATE */

	// band check
	if (channel <= 14) gBandFlg = TRUE;

	/* DPD and TSSI HW off */
	RTMP_IO_READ32(pAd,TXBE_R8, &value32);
	value32 &= ~0x08000;
	RTMP_IO_WRITE32(pAd,TXBE_R8, value32); // DPD off

	RTMP_IO_READ32(pAd,CORE_R34, &value32);
	value32 &= ~0x60;
	value32 |= 0x40;
	RTMP_IO_WRITE32(pAd,CORE_R34, value32); // TSSI off
	
	/* Do Calibration */
	/* Divider closeloop settng */
	// RXA IQ CalSetting
	if (gBandFlg)
	{
		mt_rf_write(pAd, RF_Path0, RFDIGI_TRX17,   0x00010170); // set txg gain table = to manual mode
		mt_rf_write(pAd, RF_Path1, RFDIGI_TRX17,   0x00010170); // set txg gain table = to manual mode
		mt_rf_write(pAd, RF_Path0, RG_WF0_RXG_TOP, 0x00492016); // TRSWITCH
		mt_rf_write(pAd, RF_Path1, RG_WF0_RXG_TOP, 0x00492016); // TRSWITCH
		mt_rf_write(pAd, RF_Path0, RFDIGI_TOP1,    0x0004012C); // tx block mode. 0x0 should behind 0x10
		mt_rf_write(pAd, RF_Path1, RFDIGI_TOP1,    0x0004012C); // tx block mode. 0x0 should behind 0x10
		VGAGainIdx[0] = 2;
		VGAGainIdx[1] = 2;
	}
	else
	{
		mt_rf_write(pAd, RF_Path0, RFDIGI_TRX17, 0x000101D0); // set txg gain table = to manual mode
		mt_rf_write(pAd, RF_Path1, RFDIGI_TRX17, 0x000101D0); // set txg gain table = to manual mode
		VGAGainIdx[0] = 4;
		VGAGainIdx[1] = 4;
	}
	
	// DCOC for RXA IQ Cal
	RTMP_IO_WRITE32(pAd,CORE_R1,   0x00000000); // BW=20MHz ADC=40MHz
	RTMP_IO_WRITE32(pAd,CORE_R33,  0x00021E00); // RF/BBP DAC clock control
	
	// Send single tone
	RTMP_IO_WRITE32(pAd,DACCLK_EN_DLY_CFG, 0x80008000); // DAC Clock on
	RTMP_IO_WRITE32(pAd,TXBE_R6,           0x00000000); // Test format contol : Tx single tone setting
	
	/* Divider phase calibration process */
	for (i = 0; i < 2; i++) // ANT0, ANT1
	{		
		//AGCtimeOutCount = (SwAgc1stflg == TRUE) ? 0 : 19;
		AGCtimeOutCount = 0;
		while (AGCtimeOutCount < agcLoopCnt) // SW AGC update to make sure I peak value can prevent peak value from satuation or too low
		{
			RTMP_IO_WRITE32(pAd,CORE_R4, 0x00000001);	//core soft reset enable
			RTMP_IO_WRITE32(pAd,CORE_R4, 0x00000000);	//core soft reset disable
			RTMP_IO_WRITE32(pAd,TXBE_R1, 0x00001010); 	//no attenuation, full DAC swing
		
            RF_Path_n = i;

			// Set LNA to M
            if (agcEnable == 1)
			{
                mt_rf_write(pAd, RF_Path_n, RFDIGI_TRX4, ((1<<19)|(2<<16)|(1<<15)|((0 + VGAGainIdx[i])<<8)|(1<<7)|(0 + VGAGainIdx[i])));
                DBGPRINT(RT_DEBUG_WARN,("VGAGainIdx[i] = %d\n", VGAGainIdx[i]));					
		    }	
		    else
			{
                mt_rf_write(pAd, RF_Path_n, RFDIGI_TRX4, ((1<<19)|(2<<16)|(1<<15)|((0 + preVGAGainIdx[i])<<8)|(1<<7)|(0 + preVGAGainIdx[i]))); 
                DBGPRINT(RT_DEBUG_WARN,("preVGAGainIdx[i] = %d\n", preVGAGainIdx[i]));
			}
		    
			// Internal loopback
			RTMP_IO_WRITE32(pAd, TXBE_R4, 0x00000008); // a default setting, 2T
            mt_rf_write(pAd, RF_Path_n, RFDIGI_TOP4,    0x304F1047);     // tx block mode
					
			if (gBandFlg)
			{
                mt_rf_write(pAd, RF_Path_n, RFDIGI_TOP0,    0x80056F53); // manul mode for external loopback(chip mode=5)
			}
			else
			{
                mt_rf_write(pAd, RF_Path_n, RFDIGI_TOP0,    0x80056F57); // manul mode for external loopback(chip mode=5)
			}	
						
            mt_rf_write(pAd, RF_Path_n, RFDIGI_ABB_TO_AFE5,0x00C211F1);  // set ABB config switch
			RTMP_IO_WRITE32(pAd,        RF_BSI_CKDIV,      0x00000008); 	// Adjust SPI clock		 
            mt_rf_write(pAd, RF_Path_n, RFDIGI_TRX0,       0x0500010F);     // start rxiq dcoc
			RTMP_IO_WRITE32(pAd,        RF_BSI_CKDIV,      0x00000002); 	// Adjust SPI clock		 
            
            DBGPRINT(RT_DEBUG_WARN,("Loop%d\n",i));
				
			// Set Tx/Rx index	
			RTMP_IO_WRITE32(pAd,CAL_R2,  divPhCalPath[i]); 	// Tx0
			RTMP_IO_WRITE32(pAd,TXBE_R6, 0xC0002101); 		//Test format contol : Tx single tone setting
			RTMP_IO_WRITE32(pAd,CAL_R5,  0x0000140F); 		//set accumulation length
			//RTMP_IO_WRITE32(pAd,CAL_R5,  0x000040C); 
		
			//RtmpOsMsDelay(1); // waiting 1ms		
				
			// Enable Divider phase calibration
			RTMP_IO_WRITE32(pAd,CAL_R1, 0x00000086);
			RTMP_IO_READ32(pAd,CAL_R1, &phaseCaliStatus);
			timeOutCount = 0;
			while (phaseCaliStatus & 0x80)
			{
				if (timeOutCount == 10) 
				{
					DBGPRINT(RT_DEBUG_WARN,("phaseCaliStatus = %x\n", phaseCaliStatus));
					DBGPRINT(RT_DEBUG_WARN,("LNA HW calibration can't finish process\n"));
					break;
				}
	
				timeOutCount++;
	
				RtmpOsMsDelay(1); // waiting 1ms
				
				RTMP_IO_READ32(pAd,CAL_R1, &phaseCaliStatus);
			}
	
			// 0x2C2C
			// Bit 23:16	Correlator Phase
			// Bit 15:8 	 Correlator Q value
			// Bit 7:0		  Correlator I value
			RTMP_IO_READ32(pAd,CAL_R11, &phaseCaliResult);
			mPhase0[i] = (INT)((INT)((phaseCaliResult << 8) & 0xFF000000) >> 24);
			avgQData = (INT)((INT)((phaseCaliResult << 16) & 0xFF000000) >> 24);
			avgIData = (INT)((INT)((phaseCaliResult << 24) & 0xFF000000) >> 24);
			mCalPhase0[i] = iAtan2(avgQData, avgIData);
	
			peakI[i] = (UCHAR)(phaseCaliResult >> 24);
	
			DBGPRINT(RT_DEBUG_WARN,(
				    "Divider phase cal : CAL_R11=0x%x\n" 
					"Peak I value=0x%x\n"
					"I value=0x%lx, Q value=0x%lx\n", 
					phaseCaliResult, 
					peakI[i],
					avgIData, avgQData));
	
			RTMP_IO_WRITE32(pAd,CAL_R1, 0x00000006); // Disable Calibration

            if (agcEnable == 1)
            {         
			    // SW AGC calculation
			    //if (SwAgc1stflg == TRUE && VGAGainIdx[i] < 128)
			    if ((VGAGainIdx[i] < 128) && (AGCtimeOutCount < (agcLoopCnt -1)))
			    {
				    // Calculate AGC ad check if AGC update is enough or not
				    if (peakI[i] >= 0x20 && VGAGainIdx[i] > 0)
					    VGAGainIdx[i]--;
				    else if (peakI[i] >= 0x10)
				    {
					    AGCtimeOutCount = 20;
					    //SwAgc1stflg = FALSE;
					    break;
				    }
				    //else if (peakI[i] <= 16)
				    //	VGAGainIdx[i] += 5;
				    //else if (peakI[i] <= 20)
				    //	VGAGainIdx[i] += 4;
				    //else if (peakI[i] <= 25)
				    //	VGAGainIdx[i] += 3;
				    //else if (peakI[i] <= 28)
				    //	VGAGainIdx[i] += 2;
				    else
					    VGAGainIdx[i]++;
			    }

			    DBGPRINT(RT_DEBUG_WARN,("SW AGC = %d,AGCtimeOutCount = %d\n"
                                        "===================================\n", VGAGainIdx[i], AGCtimeOutCount));
			    AGCtimeOutCount++;	
            }
            else
            {
			    AGCtimeOutCount = 20;
			    //SwAgc1stflg = FALSE;
			    break;
            }
		}
		
        // Backup AGC gain 
        if (agcEnable == 1)
        {
            preVGAGainIdx[i] = VGAGainIdx[i];
            DBGPRINT(RT_DEBUG_WARN,("Backup SWAGC = %d\n", preVGAGainIdx[i]));
		}
		
		if (gBandFlg)
		{
		    // RF0 Chip mode release
			mt_rf_write(pAd, RF_Path0, RFDIGI_TOP0,    0x00056F53);    // Turn off the RFDIGI logic clock
			mt_rf_write(pAd, RF_Path0, RFDIGI_TOP0,    0x00056F50);    // Release manual control of top control
		    RtmpOsMsDelay(1); // waiting 1ms
			mt_rf_write(pAd, RF_Path0, RFDIGI_TOP0,    0x80056F50);    // Enable RFDIGI logic clock
			//mt_rf_write(pAd, RF_Path0, RFDIGI_TOP4,    0x00001047);    // tx block mode, should 0x0 should behind 0x10
		
		    // RF1 Chip mode release
			mt_rf_write(pAd, RF_Path1, RFDIGI_TOP0,    0x00056F53);    // Turn off the RFDIGI logic clock
			mt_rf_write(pAd, RF_Path1, RFDIGI_TOP0,    0x00056F50);    // Release manual control of top control
		    RtmpOsMsDelay(1); // waiting 1ms
			mt_rf_write(pAd, RF_Path1, RFDIGI_TOP0,    0x80056F50);    // Release manual control of top control
			//mt_rf_write(pAd, RF_Path1, RFDIGI_TOP4,    0x00001047);    // tx block mode, should 0x0 should behind 0x10
		}		
		else
		{
		    // RF0 Chip mode release
		    mt_rf_write(pAd, RF_Path0, RFDIGI_TOP0,    0x00056F57);    // Turn off the RFDIGI logic clock
		    mt_rf_write(pAd, RF_Path0, RFDIGI_TOP0,    0x00056F54);    // Release manual control of top control
		    RtmpOsMsDelay(1); // waiting 1ms
		    mt_rf_write(pAd, RF_Path0, RFDIGI_TOP0,    0x80056F54);    // Enable RFDIGI logic clock
			//mt_rf_write(pAd, RF_Path0, RFDIGI_TOP4,    0x00001047);    // tx block mode, should 0x0 should behind 0x10
		
		    // RF1 Chip mode release
            mt_rf_write(pAd, RF_Path1, RFDIGI_TOP0,    0x00056F57);    // Turn off the RFDIGI logic clock
            mt_rf_write(pAd, RF_Path1, RFDIGI_TOP0,    0x00056F54);    // Release manual control of top control
		    RtmpOsMsDelay(1); // waiting 1ms
            mt_rf_write(pAd, RF_Path1, RFDIGI_TOP0,    0x80056F54);    // Release manual control of top control
			//mt_rf_write(pAd, RF_Path1, RFDIGI_TOP4,    0x00001047);    // tx block mode, should 0x0 should behind 0x10
		}	
	}

	DBGPRINT(RT_DEBUG_WARN, (
			"HW Phase vs Driver Phase (deg)\n"
			"Peak I:0x%x	  ANT0/0 :%d		%d\n"
			"Peak I:0x%x	  ANT1/1 :%d		%d\n"
			"divPhase[0] : %d\n",
			peakI[0], (360*mPhase0[0])>> 8, DEG180(mCalPhase0[0]),
			peakI[1], (360*mPhase0[1])>> 8, DEG180(mCalPhase0[1]),
			(360*(mPhase0[0]-mPhase0[1]))>> 8));
	
}
#endif // MT76x2


#ifdef DBG
#ifdef LINUX
/* #define TIMESTAMP_CAL_CAPTURE0 */
/* #define TIMESTAMP_CAL_CAPTURE1 */
#endif /* LINUX */
#endif /* DBG */



/*
	ITxBFSaveData - save MAC data
		Returns pointer to allocated buffer containing saved data
*/
static UINT32 *ITxBFSaveData(PRTMP_ADAPTER pAd)
{
	UINT32 *saveData, *sdPtr, macAddr, maxAddr;

	/* Save 48KB MAC data. */
	if (os_alloc_mem(pAd, (UCHAR **)&saveData, 0xC000)!= NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Alloc memory failed\n", __FUNCTION__));
		return NULL;
	}

	maxAddr = 0x10000;

	for (sdPtr=saveData, macAddr=0x4000; macAddr<maxAddr; macAddr += 4, sdPtr++) 
	{
		RTMP_IO_READ32(pAd, macAddr, sdPtr);
	}
	return saveData;
}


/* 
	ITxBFSaveData - restore MAC data
		saveData - buffer containing data to restore
*/
static void ITxBFRestoreData(PRTMP_ADAPTER pAd, UINT32 *saveData)
{
	UINT32 *sdPtr, macAddr, maxAddr;

	maxAddr = 0x10000;

	for (sdPtr=saveData, macAddr=0x4000; macAddr<maxAddr; macAddr += 4, sdPtr++)
	{
		RTMP_IO_WRITE32(pAd, macAddr, *sdPtr);
	}
}


/*
	mapChannelKHz - map channel number to KHz
*/
static LONG mapChannelKHz(int ch)
{
	long x;
	MAP_CHANNEL_ID_TO_KHZ(ch, x);
	return x;
}


/*
	InterpParam - Interpolate calibration parameters
		ch - channel to interpolate for
		chBeg, chEnd - begining and ending channel
		yBeg, yEnd - the hex phase values corresponding to chBeg and chEnd
*/
#ifdef MT76x2
static UCHAR InterpParam_check(int ch, int chBeg, int chEnd, UCHAR yBeg, UCHAR yEnd)
{
	long x, xBeg, xEnd, yDelta;
	UCHAR output;

	x = mapChannelKHz(ch);
	xBeg = mapChannelKHz(chBeg);
	xEnd = mapChannelKHz(chEnd);
	yDelta = yEnd - yBeg;

	/*
		Handle the phase wraparound. We always assume the delta phase is in
		the range [-180, 180] degrees = [0x80, 0x7f] in hex
	*/
	if (yDelta >= 128)
		yDelta -= 256;
	else if (yDelta <= -128)
		yDelta += 256;

	output = yBeg + yDelta*(x-xBeg)/(xEnd-xBeg);

	DBGPRINT(RT_DEBUG_TRACE,
		  ("%s : \n"
		   "x    = mapChannelKHz(%d) = %ld\n"
		   "xBeg = mapChannelKHz(%d) = %ld\n"
		   "xEnd = mapChannelKHz(%d) = %ld\n"
		   "yDelta = %ld\n"
		   "output = %d\n",
		   __FUNCTION__,
		   ch, x,
		   chBeg, xBeg,
		   chEnd, xEnd,
		   yDelta,
		   output));

	return output;
}
#endif


/*
	InterpParam - Interpolate calibration parameters
		ch - channel to interpolate for
		chBeg, chEnd - begining and ending channel
		yBeg, yEnd - the hex phase values corresponding to chBeg and chEnd
*/
static UCHAR InterpParam(int ch, int chBeg, int chEnd, UCHAR yBeg, UCHAR yEnd)
{
	long x, xBeg, xEnd, yDelta;

	x = mapChannelKHz(ch);
	xBeg = mapChannelKHz(chBeg);
	xEnd = mapChannelKHz(chEnd);
	yDelta = yEnd - yBeg;

	/*
		Handle the phase wraparound. We always assume the delta phase is in
		the range [-180, 180] degrees = [0x80, 0x7f] in hex
	*/
	if (yDelta >= 128)
		yDelta -= 256;
	else if (yDelta <= -128)
		yDelta += 256;

	return yBeg + yDelta*(x-xBeg)/(xEnd-xBeg);
}


/*
	ITxBFDivParams - interpolate Divider calibration parameter based on channel and EEPROM
		divValues - returns the Divider Calibration values for this channel
		channel - the channel to interpolate for
		divParams - the Divider Calibration parameters from EEPROM
*/

#ifdef MT76x2
static void mt76x2_ITxBFDivParams(UCHAR divValues[2], int channel, ITXBF_DIV_PARAMS *divParams)
{
	if (channel <= 14)
		divValues[0] = InterpParam(channel, 1, 14, divParams->E1gBeg, divParams->E1gEnd);
	else if (channel <= 60)
		divValues[0] = InterpParam(channel, 36, 64, divParams->E1aLowBeg, divParams->E1aLowEnd);
	//else if (channel <= 60)
	//	divValues[0] = InterpParam(channel, 44, 60, divParams->aLowBeg, divParams->aLowEnd);
	else if (channel <= 120)
		divValues[0] = InterpParam(channel, 100, 120, divParams->E1aMidBeg, divParams->E1aMidMid);
	else if (channel <= 140)
		divValues[0] = InterpParam(channel, 120, 140, divParams->E1aMidMid, divParams->E1aMidEnd);
	else
		divValues[0] = InterpParam(channel, 149, 173, divParams->E1aHighBeg, divParams->E1aHighEnd);
}
#endif

/*
	ITxBFLnaParams - interpolate LNA compensation parameter based on channel and EEPROM.
		lnaValues - returns the quantized LNA compensation values for M-L, H-L and H-M
		channel - the channel to interpolate for
		lnaParams - the LNA Calibration parameters from EEPROM
*/

#ifdef MT76x2
static void mt76x2_ITxBFLnaParams(UCHAR lnaValues[3], int channel, ITXBF_LNA_PARAMS *lnaParams)
{
	int i;

	if (channel <= 14) 
	{
		lnaValues[0] = InterpParam(channel, 1, 14, lnaParams->E1gBeg[0], lnaParams->E1gEnd[0]);
		lnaValues[1] = InterpParam(channel, 1, 14, lnaParams->E1gBeg[1], lnaParams->E1gEnd[1]);
		lnaValues[2] = InterpParam(channel, 1, 14, lnaParams->E1gBeg[2], lnaParams->E1gEnd[2]);
	}
	else if (channel <= 64) 
	{
		lnaValues[0] = InterpParam_check(channel, 36, 64, lnaParams->E1aLowBeg[0], lnaParams->E1aLowEnd[0]);
		lnaValues[1] = InterpParam_check(channel, 36, 64, lnaParams->E1aLowBeg[1], lnaParams->E1aLowEnd[1]);
		lnaValues[2] = InterpParam_check(channel, 36, 64, lnaParams->E1aLowBeg[2], lnaParams->E1aLowEnd[2]);
	}
	else if (channel <= 120) 
	{
		lnaValues[0] = InterpParam(channel, 100, 120, lnaParams->E1aMidBeg[0], lnaParams->E1aMidMid[0]);
		lnaValues[1] = InterpParam(channel, 100, 120, lnaParams->E1aMidBeg[1], lnaParams->E1aMidMid[1]);
		lnaValues[2] = InterpParam(channel, 100, 120, lnaParams->E1aMidBeg[2], lnaParams->E1aMidMid[2]);
	}
	else if (channel <= 140) 
	{
		lnaValues[0] = InterpParam(channel, 120, 140, lnaParams->E1aMidMid[0], lnaParams->E1aMidEnd[0]);
		lnaValues[1] = InterpParam(channel, 120, 140, lnaParams->E1aMidMid[1], lnaParams->E1aMidEnd[1]);
		lnaValues[2] = InterpParam(channel, 120, 140, lnaParams->E1aMidMid[2], lnaParams->E1aMidEnd[2]);
	}
	else 
	{
		lnaValues[0] = InterpParam(channel, 149, 173, lnaParams->E1aHighBeg[0], lnaParams->E1aHighEnd[0]);
		lnaValues[1] = InterpParam(channel, 149, 173, lnaParams->E1aHighBeg[1], lnaParams->E1aHighEnd[1]);
		lnaValues[2] = InterpParam(channel, 149, 173, lnaParams->E1aHighBeg[2], lnaParams->E1aHighEnd[2]);
	}
}
#endif


/*
	ITxBFPhaseParams - interpolate Phase compensation parameters based on channel and EEPROM
		phaseValues - returns the Phase compensation values for this channel
		channel - the channel to interpolate for
		phaseParams - the Phase Calibration parameters from EEPROM
*/

#ifdef MT76x2
static void mt76x2_ITxBFPhaseParams( RTMP_ADAPTER *pAd, UCHAR phaseValues[2], UCHAR channel, ITXBF_PHASE_PARAMS *phaseParams)
{
     INT chanIdx; 
     INT dbg; 
     UINT dbg1, dbg2; 
    
     chanIdx = ate_txbf_get_chan_idx(pAd, channel, 1);
    
     if(pAd->CommonCfg.ITxBfCalibMode!=2) //Use the phase of group center for other channels in the group 
     {
         if (channel <= 48) 
         {
             phaseValues[0] = phaseParams->E1aPhase[1]; //chan44         
         }
         else if (channel <= 64) 
         {
             phaseValues[0] = phaseParams->E1aPhase[3]; //chan60
         }      
         else if (channel <= 118) 
         {
             phaseValues[0] = phaseParams->E1aPhase[5]; //chan108
         }      
         else if (channel <= 144) 
         {
             phaseValues[0] = phaseParams->E1aPhase[8]; //chan132
         }      
         else 
         {
             phaseValues[0] = phaseParams->E1aPhase[11];//chan157
         }
     }
     else    // interpolate iBF phase from 13 calibrated channels 
     {
         if(chanIdx >=0)
         {
             phaseValues[0] = phaseParams->E1aPhase[chanIdx]; 
             DBGPRINT(RT_DEBUG_TRACE,
		              ("%s : \n"
		              "Phase idx = %d, Phase = 0x%x\n",
		              __FUNCTION__,chanIdx, phaseValues[0]));
         } 
         else 
         {
             if (channel <= 48) 
	         {
		         phaseValues[0] = InterpParam(channel, 36, 44, phaseParams->E1aPhase[0], phaseParams->E1aPhase[1]);
                 dbg =1 ; dbg1= phaseParams->E1aPhase[0];  dbg2 =phaseParams->E1aPhase[1];
	         }
	         else if (channel <= 64) 
	         {
		        phaseValues[0] = InterpParam(channel, 52, 60, phaseParams->E1aPhase[2], phaseParams->E1aPhase[3]);
                dbg =2 ; dbg1= phaseParams->E1aPhase[2];  dbg2 =phaseParams->E1aPhase[3];
	         }
	         else if (channel <= 108) 
	         {
		         phaseValues[0] = InterpParam(channel, 100, 108, phaseParams->E1aPhase[4], phaseParams->E1aPhase[5]);
                 dbg =3 ; dbg1= phaseParams->E1aPhase[4];  dbg2 =phaseParams->E1aPhase[5];
	         }
	         else if (channel <= 118) 
	         {
		         phaseValues[0] = InterpParam(channel, 108, 116, phaseParams->E1aPhase[5], phaseParams->E1aPhase[6]);
                 dbg =4 ; dbg1= phaseParams->E1aPhase[5];  dbg2 =phaseParams->E1aPhase[6];
	         }
	         else if (channel <= 132) 
	         {
		         phaseValues[0] = InterpParam(channel, 120, 132, phaseParams->E1aPhase[7], phaseParams->E1aPhase[8]);
                 dbg =5 ; dbg1= phaseParams->E1aPhase[7];  dbg2 =phaseParams->E1aPhase[8];
	         }
	         else if (channel <= 144) 
	         {
		         phaseValues[0] = InterpParam(channel, 132, 140, phaseParams->E1aPhase[8], phaseParams->E1aPhase[9]);
                 dbg =6 ; dbg1= phaseParams->E1aPhase[8];  dbg2 =phaseParams->E1aPhase[9];
	         }
             else if (channel <= 157) 
             {
		         phaseValues[0] = InterpParam(channel, 149, 157, phaseParams->E1aPhase[10], phaseParams->E1aPhase[11]);
                 dbg =7 ; dbg1= phaseParams->E1aPhase[10];  dbg2 =phaseParams->E1aPhase[11];
	         }
	         else 
	         {
		         phaseValues[0] = InterpParam(channel, 157, 165, phaseParams->E1aPhase[11], phaseParams->E1aPhase[12]);
                 dbg =8 ; dbg1= phaseParams->E1aPhase[11];  dbg2 =phaseParams->E1aPhase[12];
	         }

	         DBGPRINT(RT_DEBUG_TRACE,
		             ("%s : \n"
		              "dbg_flag    = %d\n"
                      " PHASE_base0 = 0x%x, PHASE base1 = 0x%x\n",
		               __FUNCTION__,dbg, dbg1,dbg2));
         }
    }
}
#endif /*MT76x2*/


#define ITXBF_EEPROM_WORDS		19	/* 38 bytes of ITxBF parameters */



/*
	ITxBFGetEEPROM - Read ITxBF calibration parameters from EEPROM
		phaseParams - pointer to BBP Phase calibration parameters. If NULL then parameters are not returned
		lnaParams - pointer to BBP LNA calibration parameters. If NULL then parameters are not returned
		divParams - divider calibration parameters. If NULL then parameters are not returned
*/
BOOLEAN ITxBFGetEEPROM(
	IN RTMP_ADAPTER *pAd,
	IN ITXBF_PHASE_PARAMS *phaseParams,
	IN ITXBF_LNA_PARAMS *lnaParams,
	IN ITXBF_DIV_PARAMS *divParams,
	IN ITXBF_TANK_PARAMS *tankParams)
{
	USHORT	EE_Value[14], andValue,phase_array_size;
	INT		i;
    BOOLEAN EfuseStat;

    for(i=0; i<14; i++)
    {
        EE_Value[i]=0xFFFF;
    }  

    EfuseStat = TRUE;
    
	/* Get Phase parameters */
	if (phaseParams != NULL) 
	{
		/* Read and check for initialized values */
		andValue = 0xFFFF;
		
#ifdef MT76x2
		if (IS_MT76x2(pAd))
		{
            for(i=0; i<7; i++)
            {                
                if (pAd->chipCap.FlgITxBfBinWrite)
                    NdisMoveMemory((PUCHAR) (&EE_Value[i]), &(pAd->EEPROMImage[EEPROM1_ITXBF_CAL_RESPHASE + 2*i]),2);
                else
                    RT28xx_EEPROM_READ16(pAd, EEPROM1_ITXBF_CAL_RESPHASE + 2*i, EE_Value[i]);
                andValue &= EE_Value[i];
            }
                
            if (andValue == 0xFFFF) 
            {
                memset(phaseParams, 0xFF, sizeof(*phaseParams));
                EfuseStat = FALSE;
            } 
            else
            {

                for(i=0; i<6; i++)
                {
                    phaseParams->E1aPhase[2*i]  = (EE_Value[i] & 0x00FF);
                    phaseParams->E1aPhase[2*i+1]= (EE_Value[i] & 0xFF00)>>8;
                }
                phaseParams->E1aPhase[12]  = (EE_Value[6] & 0x00FF);
            }
            
            //PhaseError
            for(i=0; i<7; i++)
            {                
                if (pAd->chipCap.FlgITxBfBinWrite)
                    NdisMoveMemory((PUCHAR) (&EE_Value[i]), &(pAd->EEPROMImage[EEPROM1_ITXBF_CAL_RESPHASE_ERR + 2*i]),2);
                else
                    RT28xx_EEPROM_READ16(pAd, EEPROM1_ITXBF_CAL_RESPHASE_ERR + 2*i, EE_Value[i]);
                andValue &= EE_Value[i];
            }
                            
            for(i=0; i<6; i++)
            {
                phaseParams->E1aPhaseErr[2*i]  = (EE_Value[i] & 0x00FF);
                phaseParams->E1aPhaseErr[2*i+1]= (EE_Value[i] & 0xFF00)>>8;
            }
            phaseParams->E1aPhaseErr[12]  = (EE_Value[6] & 0x00FF);
		}
#endif /* MT76x2 */
	}

	/* Get Divider Phase parameters */
	if (divParams != NULL) 
	{
		/* Read and check for initialized values */
		andValue = 0xFFFF;

#ifdef MT76x2
		if (IS_MT76x2(pAd))
		{
            for(i=0; i<3; i++)
            {                
               if (pAd->chipCap.FlgITxBfBinWrite)
                   NdisMoveMemory((PUCHAR) (&EE_Value[i]), &(pAd->EEPROMImage[EEPROM1_ITXBF_CAL_DIVPHASE + 2*i ]),2);
               else
                   RT28xx_EEPROM_READ16(pAd, EEPROM1_ITXBF_CAL_DIVPHASE + 2*i, EE_Value[i]);
               
               andValue &= EE_Value[i];
            }
                            
            if (andValue == 0xFFFF) 
            {
                 memset(divParams, 0xFF, sizeof(*divParams));
                 EfuseStat = FALSE;
            } 
            else 
            {
                 for(i=0; i<2; i++)
                 {
                    divParams->E1aDivPhase[2*i]  = (EE_Value[i] & 0x00FF);
                    divParams->E1aDivPhase[2*i+1]= (EE_Value[i] & 0xFF00)>>8;
                 }
                 
                 divParams->E1aDivPhase[4]  = (EE_Value[2] & 0x00FF);
            }
		}/* IS_MT76x2(pAd) */
#endif
	}

	/* Get LNA Parameters */
	if (lnaParams != NULL) 
	{
		/* Read and check for initialized values */
		andValue = 0xFFFF;
		

#ifdef MT76x2
		if (IS_MT76x2(pAd))
		{
			for (i=0; i<14; i++) 
			{
				if (pAd->chipCap.FlgITxBfBinWrite)
					NdisMoveMemory((PUCHAR) (&EE_Value[i]), &(pAd->EEPROMImage[EEPROM1_ITXBF_CAL + 20 + 2*i]),2);
				else
					RT28xx_EEPROM_READ16(pAd, EEPROM1_ITXBF_CAL + 20 + 2*i, EE_Value[i]);
				andValue &= EE_Value[i];
			}

			if (andValue == 0xFFFF) 
			{
				memset(lnaParams, 0xFF, sizeof(*lnaParams));
                EfuseStat = FALSE;
			}
			else 
			{
				lnaParams->E1gBeg[0] = (EE_Value[0] & 0x00FF);
				lnaParams->E1gBeg[1] = (EE_Value[0] & 0xFF00)>>8;
				lnaParams->E1gBeg[2] = (EE_Value[1] & 0x00FF);
				lnaParams->E1gEnd[0] = (EE_Value[1] & 0xFF00)>>8;
				lnaParams->E1gEnd[1] = (EE_Value[2] & 0x00FF);
				lnaParams->E1gEnd[2] = (EE_Value[2] & 0xFF00)>>8;

				lnaParams->E1aHighBeg[0] = (EE_Value[3] & 0x00FF);
				lnaParams->E1aHighBeg[1] = (EE_Value[3] & 0xFF00)>>8;
				lnaParams->E1aHighBeg[2] = (EE_Value[4] & 0x00FF);
				lnaParams->E1aHighEnd[0] = (EE_Value[4] & 0xFF00)>>8;
				lnaParams->E1aHighEnd[1] = (EE_Value[5] & 0x00FF);
				lnaParams->E1aHighEnd[2] = (EE_Value[5] & 0xFF00)>>8;
			
				lnaParams->E1aLowBeg[0] = (EE_Value[6] & 0x00FF);
				lnaParams->E1aLowBeg[1] = (EE_Value[6] & 0xFF00)>>8;
				lnaParams->E1aLowBeg[2] = (EE_Value[7] & 0x00FF);
				lnaParams->E1aLowEnd[0] = (EE_Value[7] & 0xFF00)>>8;
				lnaParams->E1aLowEnd[1] = (EE_Value[8] & 0x00FF);
				lnaParams->E1aLowEnd[2] = (EE_Value[8] & 0xFF00)>>8;
			
				lnaParams->E1aMidBeg[0] = (EE_Value[9] & 0x00FF);
				lnaParams->E1aMidBeg[1] = (EE_Value[9] & 0xFF00)>>8;
				lnaParams->E1aMidBeg[2] = (EE_Value[10] & 0x00FF);
				lnaParams->E1aMidMid[0] = (EE_Value[10] & 0xFF00)>>8;
				lnaParams->E1aMidMid[1] = (EE_Value[11] & 0x00FF);
				lnaParams->E1aMidMid[2] = (EE_Value[11] & 0xFF00)>>8;
				lnaParams->E1aMidEnd[0] = (EE_Value[12] & 0x00FF);
				lnaParams->E1aMidEnd[1] = (EE_Value[12] & 0xFF00)>>8;
				lnaParams->E1aMidEnd[2] = (EE_Value[13] & 0x00FF);
			}/* andValue == 0xFFFF */
		}/* IS_MT76x2(pAd) */
#endif	
	}/* lnaParams != NULL */
	
    /* Get tank code parameters */
    if (tankParams != NULL) 
    {
        /* Read and check for initialized values */
        andValue = 0xFFFF;    
#ifdef MT76x2
        if (IS_MT76x2(pAd))
        {                                  
            for(i=0; i<5; i++)
            {                
                if (pAd->chipCap.FlgITxBfBinWrite)
                    NdisMoveMemory((PUCHAR) (&EE_Value[i]), &(pAd->EEPROMImage[EEPROM1_ITXBF_CAL_TANK + 2*i ]),2);
                else
                    RT28xx_EEPROM_READ16(pAd, EEPROM1_ITXBF_CAL_TANK + 2*i , EE_Value[i]);
                andValue &= EE_Value[i];
            }
                                
            if (andValue == 0xFFFF) 
            {
                 memset(tankParams, 0xFF, sizeof(*tankParams));
                 EfuseStat = FALSE;
            } 
            else 
            {
                 for(i=0; i<5; i++)
                 {
                     tankParams->TankCode[2*i]  = (EE_Value[i] & 0x00FF);      //Repeater tank code
                     tankParams->TankCode[2*i+1]= (EE_Value[i] & 0xFF00)>>8;   // IQM tank code
                 }                     
            }
        }/* IS_MT76x2(pAd) */
#endif
    }
    
    return EfuseStat;
    
}


/*
	ITxBFSetEEPROM - Save ITxBF calibration parameters in EEPROM
		phaseParams - pointer to BBP calibration parameters. If NULL then parameters are not written
		lnaParams - pointer to BBP LNA calibration parameters. If NULL then parameters are not written
		divParams - divider calibration parameters. If NULL then parameters are not written
*/
void ITxBFSetEEPROM(
	IN PRTMP_ADAPTER pAd,
	IN ITXBF_PHASE_PARAMS *phaseParams,
	IN ITXBF_LNA_PARAMS *lnaParams,
	IN ITXBF_DIV_PARAMS *divParams,
    IN ITXBF_TANK_PARAMS *tankParams)
{
	USHORT	EE_Value[14], eeTmp;
	int		i, eeAddr;

    for(i=0; i<14; i++)
    {
        EE_Value[i]=0xFFFF;
    }
	/* Set EEPROM parameters */

	/* Phase parameters */
	if (phaseParams != NULL) 
	{

#ifdef MT76x2
		if (IS_MT76x2(pAd))
		{
            for (i=0; i<6 ; i++)
            {      
			    EE_Value[i] = phaseParams->E1aPhase[2*i] | (phaseParams->E1aPhase[2*i+1]<<8);
            }
			EE_Value[6] = phaseParams->E1aPhase[12]  | 0xFF00;

			for (i=0; i<7; i++)
			{
				eeAddr = EEPROM1_ITXBF_CAL_RESPHASE + 2*i ;
			
				if (pAd->chipCap.FlgITxBfBinWrite)
					NdisMoveMemory((PUCHAR) (&eeTmp), &(pAd->EEPROMImage[eeAddr]),2);
				else
					RT28xx_EEPROM_READ16(pAd, eeAddr, eeTmp);
			
				if (eeTmp != EE_Value[i])
				{
					if (pAd->chipCap.FlgITxBfBinWrite)
						NdisMoveMemory(&(pAd->EEPROMImage[eeAddr]), (PUCHAR) (&EE_Value[i]),2);
					else
						RT28xx_EEPROM_WRITE16(pAd, eeAddr, EE_Value[i]);
				}

				DBGPRINT(RT_DEBUG_TRACE, 
				   ("ITxBFGetEEPROM check ::: \n"
				   	"EEPROM origina data =0x%x\n"
				   	"Input data = 0x%x\n"
				   	"Saved low byte data in EEPROM=0x%x\n"
				   	"Saved high byte data in EEPROM=0x%x\n",
				   	eeTmp,
				   	EE_Value[i],
				   	pAd->EEPROMImage[eeAddr],
				   	pAd->EEPROMImage[eeAddr + 1]));
			}
			
            //PhaseError
            for (i=0; i<6 ; i++){      
			    EE_Value[i] = phaseParams->E1aPhaseErr[2*i] | (phaseParams->E1aPhaseErr[2*i+1]<<8);
            }
			EE_Value[6] = phaseParams->E1aPhaseErr[12] | 0xFF00;

			for (i=0; i<7; i++)
			{
				eeAddr = EEPROM1_ITXBF_CAL_RESPHASE_ERR + 2*i ;
			
				if (pAd->chipCap.FlgITxBfBinWrite)
					NdisMoveMemory((PUCHAR) (&eeTmp), &(pAd->EEPROMImage[eeAddr]),2);
				else
					RT28xx_EEPROM_READ16(pAd, eeAddr, eeTmp);
			
				if (eeTmp != EE_Value[i])
				{
					if (pAd->chipCap.FlgITxBfBinWrite)
						NdisMoveMemory(&(pAd->EEPROMImage[eeAddr]), (PUCHAR) (&EE_Value[i]),2);
					else
						RT28xx_EEPROM_WRITE16(pAd, eeAddr, EE_Value[i]);
				}				
			}
		}/* IS_MT76x2(pAd) */
#endif	
	}/* phaseParams != NULL */

	/* Divider Phase parameters */
	if (divParams != NULL) 
	{

#ifdef MT76x2
		if (IS_MT76x2(pAd))
		{
            EE_Value[0] = divParams->E1aDivPhase[0] | (divParams->E1aDivPhase[1]<<8);
			EE_Value[1] = divParams->E1aDivPhase[2] | (divParams->E1aDivPhase[3] <<8);
			EE_Value[2] = divParams->E1aDivPhase[4] | 0xFF00;
			
			for (i=0; i<3; i++)
			{
				eeAddr = EEPROM1_ITXBF_CAL_DIVPHASE + 2*i;
				if (pAd->chipCap.FlgITxBfBinWrite)
					NdisMoveMemory((PUCHAR) (&eeTmp), &(pAd->EEPROMImage[eeAddr]),2);
				else
					RT28xx_EEPROM_READ16(pAd, eeAddr, eeTmp);
			
				if (eeTmp != EE_Value[i])
				{
					if (pAd->chipCap.FlgITxBfBinWrite)
						NdisMoveMemory(&(pAd->EEPROMImage[eeAddr]), (PUCHAR) (&EE_Value[i]),2);
					else
						RT28xx_EEPROM_WRITE16(pAd, eeAddr, EE_Value[i]);
				}
			}
		}/* IS_MT76x2(pAd) */
#endif	
	}/* divParams != NULL*/

	/* LNA Phase parameters */
	if (lnaParams != NULL) {

#ifdef MT76x2
		if (IS_MT76x2(pAd))
		{
			EE_Value[0]  = lnaParams->E1gBeg[0]     | (lnaParams->E1gBeg[1]<<8);
			EE_Value[1]  = lnaParams->E1gBeg[2]     | (lnaParams->E1gEnd[0]<<8);
			EE_Value[2]  = lnaParams->E1gEnd[1]     | (lnaParams->E1gEnd[2]<<8);

			EE_Value[3]  = lnaParams->E1aHighBeg[0] | (lnaParams->E1aHighBeg[1]<<8);
			EE_Value[4]  = lnaParams->E1aHighBeg[2] | (lnaParams->E1aHighEnd[0]<<8);
			EE_Value[5]  = lnaParams->E1aHighEnd[1] | (lnaParams->E1aHighEnd[2]<<8);
			EE_Value[6]  = lnaParams->E1aLowBeg[0]  | (lnaParams->E1aLowBeg[1]<<8);
			EE_Value[7]  = lnaParams->E1aLowBeg[2]  | (lnaParams->E1aLowEnd[0]<<8);
			EE_Value[8]  = lnaParams->E1aLowEnd[1]  | (lnaParams->E1aLowEnd[2]<<8);
			EE_Value[9]  = lnaParams->E1aMidBeg[0]  | (lnaParams->E1aMidBeg[1]<<8);
			EE_Value[10] = lnaParams->E1aMidBeg[2]  | (lnaParams->E1aMidMid[0]<<8);
			EE_Value[11] = lnaParams->E1aMidMid[1]  | (lnaParams->E1aMidMid[2]<<8);
			EE_Value[12] = lnaParams->E1aMidEnd[0]  | (lnaParams->E1aMidEnd[1]<<8);
			EE_Value[13] = lnaParams->E1aMidEnd[2]  | 0xAA00;

			for (i=0; i<14; i++)
			{
				eeAddr = EEPROM1_ITXBF_CAL + 20 + 2*i;
				if (pAd->chipCap.FlgITxBfBinWrite)
					NdisMoveMemory((PUCHAR) (&eeTmp), &(pAd->EEPROMImage[eeAddr]),2);
				else
					RT28xx_EEPROM_READ16(pAd, eeAddr, eeTmp);
			
				if (eeTmp != EE_Value[i])
				{
					if (pAd->chipCap.FlgITxBfBinWrite)
						NdisMoveMemory(&(pAd->EEPROMImage[eeAddr]), (PUCHAR) (&EE_Value[i]),2);
					else
						RT28xx_EEPROM_WRITE16(pAd, eeAddr, EE_Value[i]);
				}	

				DBGPRINT(RT_DEBUG_INFO, 
				   ("ITxBFGetEEPROM check ::: \n"
				   	"EEPROM origina data =0x%x\n"
				   	"Input data = 0x%x\n"
				   	"Saved low byte data in EEPROM=0x%x\n"
				   	"Saved high byte data in EEPROM=0x%x\n",
				   	eeTmp,
				   	pAd->EEPROMImage[eeAddr],
				   	pAd->EEPROMImage[eeAddr + 1],
				   	EE_Value[i]));
			}
		}/* IS_MT76x2(pAd) */
#endif	
	}/* lnaParams != NULL */

    /* tank code parameters */
	if (tankParams != NULL) 
	{
#ifdef MT76x2
		if (IS_MT76x2(pAd))
		{         
            for (i=0; i<5 ; i++)
            {      
                EE_Value[i] = tankParams->TankCode[2*i] | (tankParams->TankCode[2*i+1]<<8);
            }

            for (i=0; i<5; i++)
            {
                eeAddr = EEPROM1_ITXBF_CAL_TANK + 2*i;
                if (pAd->chipCap.FlgITxBfBinWrite)
                    NdisMoveMemory((PUCHAR) (&eeTmp), &(pAd->EEPROMImage[eeAddr]),2);
                else
                    RT28xx_EEPROM_READ16(pAd, eeAddr, eeTmp);
                   
                if (eeTmp != EE_Value[i])
                {
                    if (pAd->chipCap.FlgITxBfBinWrite)
                        NdisMoveMemory(&(pAd->EEPROMImage[eeAddr]), (PUCHAR) (&EE_Value[i]),2);
                    else
                        RT28xx_EEPROM_WRITE16(pAd, eeAddr, EE_Value[i]);
                }

				DBGPRINT(RT_DEBUG_INFO, 
				   ("ITxBFGetEEPROM check ::: \n"
				   	"EEPROM origina data =0x%x\n"
				   	"Input data = 0x%x\n"
				   	"Saved low byte data in EEPROM=0x%x\n"
				   	"Saved high byte data in EEPROM=0x%x\n",
				   	eeTmp,
				   	pAd->EEPROMImage[eeAddr],
				   	pAd->EEPROMImage[eeAddr + 1],
				   	EE_Value[i]));
            }
                    	    
		}/* IS_MT76x2(pAd) */
#endif
	}/* phaseParams != NULL */
}


/*
	ITxBFLoadLNAComp - load the LNA compensation registers
*/

#ifdef MT76x2
VOID mt76x2_ITxBFLoadLNAComp(
	IN RTMP_ADAPTER *pAd)
{
	ITXBF_LNA_PARAMS lnaParams;
	UCHAR lnaValues[3];
	UCHAR bbpValue = 0;
	int i;
	UCHAR channel = pAd->CommonCfg.Channel;
	UINT  value32;


#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		channel = pAd->ate.Channel;
#endif /* RALINK_ATE */

	/* Get values */
	ITxBFGetEEPROM(pAd, 0, &lnaParams, 0, 0);

	mt76x2_ITxBFLnaParams(lnaValues, channel, &lnaParams);

	RTMP_IO_READ32(pAd, CAL_R0, &value32);
	for (i=0; i < NUM_CHAIN; i++)
	{
		RTMP_IO_WRITE32(pAd, CAL_R0, value32 | (i<<5));
		RTMP_IO_WRITE32(pAd, RXFE_R3, lnaValues[i]);
	}

	DBGPRINT(RT_DEBUG_TRACE, 
		  ("============== Interpolate LNA phase ===============\n"
		   "lnaValues[0] = %d\n"
		   "lnaValues[1] = %d\n"
		   "lnaValues[2] = %d\n"
		   "====================================================\n",
		   (360 * lnaValues[0]) >> 8, (360 * lnaValues[1]) >> 8, (360 * lnaValues[2]) >> 8));

	/* Enable RX Phase Compensation */
	RTMP_IO_READ32(pAd, TXBE_R12, &value32);
	RTMP_IO_WRITE32(pAd, TXBE_R12, value32 | 0x20);
}


INT ITxBFLoPhaseCalibrationStartUp(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN eepromUpdateFlg,
	OUT UCHAR *LoPhase)
{
	
	UINT CR_BK[25];
	BOOLEAN calStatusReport;
	ULONG stTimeChk0, stTimeChk1;
		
	NdisGetSystemUpTime(&stTimeChk0);
	// Backup CRs which are used in Divider Caliration
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX17, &CR_BK[0]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX17, &CR_BK[1]);
	mt_rf_read(pAd, RF_Path0, RG_WF0_RXG_TOP, &CR_BK[2]);
	mt_rf_read(pAd, RF_Path1, RG_WF0_RXG_TOP, &CR_BK[3]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TOP3, &CR_BK[4]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TOP3, &CR_BK[5]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TOP0, &CR_BK[6]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TOP0, &CR_BK[7]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TOP1, &CR_BK[6]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TOP1, &CR_BK[7]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX0, &CR_BK[6]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX0, &CR_BK[7]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX4, &CR_BK[8]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX4, &CR_BK[9]);

	RTMP_IO_READ32(pAd, TXBE_R8,   &CR_BK[10]);
	RTMP_IO_READ32(pAd, CORE_R34,  &CR_BK[11]);
	RTMP_IO_READ32(pAd, CORE_R1,   &CR_BK[12]);
	RTMP_IO_READ32(pAd, RF_BSI_CKDIV,  &CR_BK[13]);
	RTMP_IO_READ32(pAd, DACCLK_EN_DLY_CFG,   &CR_BK[14]);
	RTMP_IO_READ32(pAd, PWR_PIN_CFG,   &CR_BK[15]);
	RTMP_IO_READ32(pAd, CORE_R33,  &CR_BK[16]);
	RTMP_IO_READ32(pAd, TXBE_R6,   &CR_BK[17]);
	RTMP_IO_READ32(pAd, CORE_R4,   &CR_BK[18]);
	RTMP_IO_READ32(pAd, TXBE_R1,   &CR_BK[19]);
	RTMP_IO_READ32(pAd, AGC1_R0,   &CR_BK[20]);
	RTMP_IO_READ32(pAd, TXBE_R4,   &CR_BK[21]);
	RTMP_IO_READ32(pAd, CAL_R2,    &CR_BK[22]);
	RTMP_IO_READ32(pAd, TXBE_R6,   &CR_BK[23]);
	RTMP_IO_READ32(pAd, CAL_R5,    &CR_BK[24]);

	NdisGetSystemUpTime(&stTimeChk1);
	DBGPRINT(RT_DEBUG_INFO, ("%s : Lo Phase calibration duration1 = %ld ms\n", 
		     __FUNCTION__, (stTimeChk1 - stTimeChk0)*1000/OS_HZ));
	
	// Do the divider calibration
	NdisGetSystemUpTime(&stTimeChk0);
	calStatusReport = mt76x2_ITxBFLoPhaseCalibration(pAd, eepromUpdateFlg, LoPhase);
	NdisGetSystemUpTime(&stTimeChk1);
	DBGPRINT(RT_DEBUG_INFO, ("%s : Lo Phase calibration duration2 = %ld ms\n", 
		     __FUNCTION__, (stTimeChk1 - stTimeChk0)*1000/OS_HZ));
	
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX17, CR_BK[0]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX17, CR_BK[1]);
	mt_rf_write(pAd, RF_Path0, RG_WF0_RXG_TOP, CR_BK[2]);
	mt_rf_write(pAd, RF_Path1, RG_WF0_RXG_TOP, CR_BK[3]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TOP3, CR_BK[4]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TOP3, CR_BK[5]);
	//mt_rf_write(pAd, RF_Path0, RFDIGI_TOP0, CR_BK[6]); //
	//mt_rf_write(pAd, RF_Path1, RFDIGI_TOP0, CR_BK[7]); //
	mt_rf_write(pAd, RF_Path0, RFDIGI_TOP1, CR_BK[6]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TOP1, CR_BK[7]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX0, CR_BK[6]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX0, CR_BK[7]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX4, CR_BK[8]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX4, CR_BK[9]);

	RTMP_IO_WRITE32(pAd, TXBE_R8,   CR_BK[10]);
	RTMP_IO_WRITE32(pAd, CORE_R34,  CR_BK[11]);
	RTMP_IO_WRITE32(pAd, CORE_R1,   CR_BK[12]);
	RTMP_IO_WRITE32(pAd, RF_BSI_CKDIV,  CR_BK[13]);
	RTMP_IO_WRITE32(pAd, DACCLK_EN_DLY_CFG,   CR_BK[14]);
	RTMP_IO_WRITE32(pAd, PWR_PIN_CFG,   CR_BK[15]);
	RTMP_IO_WRITE32(pAd, CORE_R33,  CR_BK[16]);
	RTMP_IO_WRITE32(pAd, TXBE_R6,   CR_BK[17]);
	RTMP_IO_WRITE32(pAd, CORE_R4,   CR_BK[18]);
	RTMP_IO_WRITE32(pAd, TXBE_R1,   CR_BK[19]);
	RTMP_IO_WRITE32(pAd, AGC1_R0,   CR_BK[20]);
	RTMP_IO_WRITE32(pAd, TXBE_R4,   CR_BK[21]);
	RTMP_IO_WRITE32(pAd, CAL_R2,    CR_BK[22]);
	RTMP_IO_WRITE32(pAd, TXBE_R6,   CR_BK[23]);
	RTMP_IO_WRITE32(pAd, CAL_R5,    CR_BK[24]);

	return calStatusReport;
}


INT mt76x2_ITxBFLoPhaseCalibration(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN eepromUpdateFlg,
	OUT UCHAR *LoPhase)
{
	int i;
	UINT32 phaseCaliStatus, phaseCaliResult, value32;
	UCHAR  LoPhCalPath[2]={RX1TX0, RX0TX1};
	//UINT32 LnaVgaSet[2] = {0x000B8F8F, 0x000B9999}, TxgGain[2] = {0x160, 0x150};
    UINT32 LnaVgaSet[2] = {0x000A8585, 0x000A8585}, TxgGain[2] = {0x110, 0x110};
	UCHAR timeOutCount;
	INT      mPhase[2];
	INT      mCalPhase[2];
	INT      avgIData, avgQData;
	UCHAR  peakI[2];
	UCHAR  pathIdx[2] = {0, 1}, pathInv[2] = {1, 0};
	ITXBF_DIV_PARAMS LoPhaseParams;
    UCHAR channel = pAd->CommonCfg.Channel;
    UCHAR LoPhaseOffset;


#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		channel = pAd->ate.Channel;
#endif /* RALINK_ATE */

	/* DPD and TSSI HW off */
	RTMP_IO_READ32(pAd,TXBE_R8, &value32);
	value32 &= ~0x08000;
	RTMP_IO_WRITE32(pAd,TXBE_R8, value32); // DPD off

	RTMP_IO_READ32(pAd,CORE_R34, &value32);
	value32 &= ~0x60;
	value32 |= 0x40;
	RTMP_IO_WRITE32(pAd,CORE_R34, value32); // TSSI off

	//for (i = 0; i < 2; i++) // PM01, PM10
	for (i = 1; i < 2; i++) // just need PM10
	{
		mt_rf_write(pAd, pathIdx[i], RFDIGI_TRX17, TxgGain[i]);      // set txg gain table = to manual mode
		mt_rf_write(pAd, pathIdx[i], RG_WF0_RXG_TOP, 0x00492016);// TRSWITCH

		// ********* external llopback *********
		mt_rf_write(pAd, RF_Path1, RFDIGI_TOP3, 0x11C73047); // Force Divider ON
		mt_rf_write(pAd, RF_Path0, RFDIGI_TOP3, 0x11C73047); // Force Divider ON

		mt_rf_write(pAd, pathInv[i], RFDIGI_TOP0, 0x80056733); // enable SX/RX/for WF0
		mt_rf_write(pAd, pathInv[i], RFDIGI_TOP1, 0xF0040122); // set TRSW in RX mode for WF0
		mt_rf_write(pAd, pathIdx[i], RFDIGI_TOP0, 0x80056F23); // enable SX/RX/for WF1
		mt_rf_write(pAd, pathIdx[i], RFDIGI_TOP1, 0xF0040123); // set TRSW in RX mode for WF1

		// ********* cal setting ********
		RTMP_IO_WRITE32(pAd, CORE_R1, 0x00000000); // BW=20MHz ADC=40MHz

		// ********* pre dcoc ********
		RTMP_IO_WRITE32(pAd, RF_BSI_CKDIV, 0x00000008);
		mt_rf_write(pAd, RF_Path0, RFDIGI_TRX0, 0x0500010F);
		RTMP_IO_WRITE32(pAd, RF_BSI_CKDIV, 0x00000002);
		RTMP_IO_WRITE32(pAd, DACCLK_EN_DLY_CFG, 0x80008000); // DAC Clock on

		// ********* send single tone ******
		RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, 0x00000000); // to turn off "adda power down"
		RTMP_IO_WRITE32(pAd, CORE_R33, 0x00021E00); // RF/BBP clk control
		RTMP_IO_WRITE32(pAd, TXBE_R6, 0x40002101);   // Test format contol : Tx single tone setting
		RTMP_IO_WRITE32(pAd, CORE_R4, 0x00000001);  // core soft reset enable
		RTMP_IO_WRITE32(pAd, CORE_R4, 0x00000000);  // core soft reset enable
		RTMP_IO_WRITE32(pAd, TXBE_R1, 0x00000606);  // Tx power control

		// ********* LNA VGA setting ******
		mt_rf_write(pAd, pathInv[i], RFDIGI_TRX4, LnaVgaSet[i]);

		// ********* start iBF cal ******
		RTMP_IO_WRITE32(pAd, AGC1_R0, 0x00007408); // 01:1R ADC1, 00 : 1R ADC0
		RTMP_IO_WRITE32(pAd, TXBE_R4, 0x00000008); // a default setting, 2T
		RTMP_IO_WRITE32(pAd, CAL_R2, LoPhCalPath[i]);  // TR loop setting(0:T0R0,1:T1R0,5:T1R1,4:T0R1)

		RTMP_IO_WRITE32(pAd,TXBE_R6, 0xC0002101); 	//Test format contol : Tx single tone setting
		RTMP_IO_WRITE32(pAd,CAL_R5,  0x000040C); 	//set accumulation length
				
		// Enable Divider phase calibration
		RTMP_IO_WRITE32(pAd,CAL_R1, 0x00000086);
		RTMP_IO_READ32(pAd,CAL_R1, &phaseCaliStatus);
		timeOutCount = 0;
		while (phaseCaliStatus & 0x80)
		{
			if (timeOutCount == 10) 
			{
				DBGPRINT(RT_DEBUG_TRACE,("phaseCaliStatus = %x\n", phaseCaliStatus));
				DBGPRINT(RT_DEBUG_TRACE,("Lo phase HW calibration can't finish process\n"));
				break;
			}
	
			timeOutCount++;
	
			RtmpOsMsDelay(1); // waiting 1ms
				
			RTMP_IO_READ32(pAd,CAL_R1, &phaseCaliStatus);
		}
	
		// 0x2C2C
		// Bit 23:16	Correlator Phase
		// Bit 15:8 	 Correlator Q value
		// Bit 7:0		  Correlator I value
		RTMP_IO_READ32(pAd,CAL_R11, &phaseCaliResult);
		mPhase[i] = (INT)((INT)((phaseCaliResult << 8) & 0xFF000000) >> 24);
		avgQData = (INT)((INT)((phaseCaliResult << 16) & 0xFF000000) >> 24);
		avgIData = (INT)((INT)((phaseCaliResult << 24) & 0xFF000000) >> 24);
		mCalPhase[i] = iAtan2(avgQData, avgIData);
	
		peakI[i] = (UCHAR)(phaseCaliResult >> 24);
	
		DBGPRINT(RT_DEBUG_TRACE,(
                                "*****ITxBFLoPhaseCalibration()****\n"
				"Lo phase[%d] = 0x%x\n"
				"CAL_R11=0x%x\n" 
				"Peak I value=0x%x\n"
				"I value=0x%x, Q value=0x%x\n", 
				i,
				mPhase[i],
				phaseCaliResult, 
				peakI[i],
				avgIData, avgQData));
	
		RTMP_IO_WRITE32(pAd,CAL_R1, 0x00000006); // Disable Calibration

		// ********* Top Manual Release **********
		mt_rf_write(pAd, RF_Path0, RFDIGI_TOP0, 0x00056733); // WF0 manul mode
		mt_rf_write(pAd, RF_Path0, RFDIGI_TOP0, 0x00056730); // WF0 manul mode
		mt_rf_write(pAd, RF_Path0, RFDIGI_TOP0, 0x80056730); // WF0 manul mode
		mt_rf_write(pAd, RF_Path1, RFDIGI_TOP0, 0x00056F23); // WF0 manul mode
		mt_rf_write(pAd, RF_Path1, RFDIGI_TOP0, 0x00056F20); // WF0 manul mode
		mt_rf_write(pAd, RF_Path1, RFDIGI_TOP0, 0x80056F20); // WF0 manul mode
	}

	/* Save new reference values in EEPROM and BBP */
	ITxBFGetEEPROM(pAd, 0, 0, &LoPhaseParams, 0);

	if (eepromUpdateFlg == TRUE)
	{

        if( channel == 1)
        {
		LoPhaseParams.E1gBeg= mPhase[1];
        }
        else if( channel == 14)
        {
            LoPhaseParams.E1gEnd= mPhase[1];
        }

		ITxBFSetEEPROM(pAd, 0, 0, &LoPhaseParams, 0);

		*LoPhase = 0;

        DBGPRINT(RT_DEBUG_TRACE,(
                "####### DebugKK[2] chan= %d ,LOphase in efuse  = 0x%2x\n",
			    channel,mPhase[1]));
	}
	else
	{
        if ( channel == 1)
        {
            LoPhaseOffset = LoPhaseParams.E1gBeg;
        }        
        else
        {
            LoPhaseOffset = LoPhaseParams.E1gEnd;
        }

		*LoPhase = (mPhase[1] - LoPhaseOffset) & 0xFF;

		DBGPRINT(RT_DEBUG_TRACE,(
                "LO phase this time      = 0x%x\n"                         
			    "Lo Phase difference    = 0x%x\n"
			    "Lo Phase in Efuse  = 0x%x\n",
			    mPhase[1],*LoPhase, LoPhaseOffset));
	}

	return TRUE;
}

INT ITxBFDividerCalibrationStartUp(
	IN RTMP_ADAPTER *pAd,
	IN int calFunction,
	IN int calMethod,
	OUT UCHAR *divPhase)
{
	
	UINT CR_BK[35];
	BOOLEAN calStatusReport;
	ULONG stTimeChk0, stTimeChk1;
		
	NdisGetSystemUpTime(&stTimeChk0);
	// Backup CRs which are used in Divider Caliration
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX17, &CR_BK[0]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX38, &CR_BK[1]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX39, &CR_BK[2]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX42, &CR_BK[3]);
	mt_rf_read(pAd, RF_Path0, A_BAND_PA,	&CR_BK[4]);
	mt_rf_read(pAd, RF_Path0, A_BAND_IQM_TSSI_DIV_LPF,	&CR_BK[5]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX17, &CR_BK[6]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX38, &CR_BK[7]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX39, &CR_BK[8]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX42, &CR_BK[9]);
	mt_rf_read(pAd, RF_Path1, A_BAND_PA,	&CR_BK[10]);
	mt_rf_read(pAd, RF_Path1, A_BAND_IQM_TSSI_DIV_LPF,	&CR_BK[11]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX0,	&CR_BK[12]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX0,	&CR_BK[13]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX4,	&CR_BK[14]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX4,	&CR_BK[15]);
    mt_rf_read(pAd, RF_Path0, RFDIGI_TOP1,  &CR_BK[16]);
    mt_rf_read(pAd, RF_Path1, RFDIGI_TOP1,  &CR_BK[17]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_ABB_TO_AFE5,&CR_BK[19]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_ABB_TO_AFE5,&CR_BK[20]);
	
	RTMP_IO_READ32(pAd,CORE_R1,   &CR_BK[22]);
	RTMP_IO_READ32(pAd,CORE_R33,  &CR_BK[23]);
	RTMP_IO_READ32(pAd,DACCLK_EN_DLY_CFG, &CR_BK[24]);
	RTMP_IO_READ32(pAd,TXBE_R6,   &CR_BK[25]);
	RTMP_IO_READ32(pAd,CORE_R4,   &CR_BK[26]);
	RTMP_IO_READ32(pAd,TXBE_R1,   &CR_BK[27]);
	RTMP_IO_READ32(pAd,AGC1_R0,   &CR_BK[28]);
	RTMP_IO_READ32(pAd,TXBE_R4,   &CR_BK[29]);
	RTMP_IO_READ32(pAd,CAL_R2,	  &CR_BK[30]);
	RTMP_IO_READ32(pAd,CAL_R5,	  &CR_BK[31]);
	RTMP_IO_READ32(pAd,CAL_R1,	  &CR_BK[32]);
	RTMP_IO_READ32(pAd,TXBE_R5,   &CR_BK[33]);
	RTMP_IO_READ32(pAd,PWR_PIN_CFG,&CR_BK[34]);
	NdisGetSystemUpTime(&stTimeChk1);
	DBGPRINT(RT_DEBUG_INFO, ("%s : Divider calibration duration1 = %ld ms\n", 
		     __FUNCTION__, (stTimeChk1 - stTimeChk0)*1000/OS_HZ));
	
	// Do the divider calibration
	NdisGetSystemUpTime(&stTimeChk0);
	RTMP_IO_WRITE32(pAd, AGC1_R0, 0x7408);
	calStatusReport = mt76x2_ITxBFDividerCalibration(pAd, calFunction, calMethod, divPhase);
	NdisGetSystemUpTime(&stTimeChk1);
	DBGPRINT(RT_DEBUG_INFO, ("%s : Divider calibration duration2 = %ld ms\n", 
		     __FUNCTION__, (stTimeChk1 - stTimeChk0)*1000/OS_HZ));
	
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX17, CR_BK[0]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX38, CR_BK[1]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX39, CR_BK[2]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX42, CR_BK[3]);
	mt_rf_write(pAd, RF_Path0, A_BAND_PA,	 CR_BK[4]);
	mt_rf_write(pAd, RF_Path0, A_BAND_IQM_TSSI_DIV_LPF,  CR_BK[5]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX17, CR_BK[6]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX38, CR_BK[7]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX39, CR_BK[8]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX42, CR_BK[9]);
	mt_rf_write(pAd, RF_Path1, A_BAND_PA,	 CR_BK[10]);
	mt_rf_write(pAd, RF_Path1, A_BAND_IQM_TSSI_DIV_LPF,  CR_BK[11]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX0,  CR_BK[12]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX0,  CR_BK[13]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX4,  CR_BK[14]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX4,  CR_BK[15]);
    mt_rf_write(pAd, RF_Path0, RFDIGI_TOP1,	 CR_BK[16]);
    mt_rf_write(pAd, RF_Path1, RFDIGI_TOP1,  CR_BK[17]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_ABB_TO_AFE5,CR_BK[19]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_ABB_TO_AFE5,CR_BK[20]);
	
	RTMP_IO_WRITE32(pAd,CORE_R1,    CR_BK[22]);
	RTMP_IO_WRITE32(pAd,CORE_R33,   CR_BK[23]);
	RTMP_IO_WRITE32(pAd,DACCLK_EN_DLY_CFG, CR_BK[24]);
	RTMP_IO_WRITE32(pAd,TXBE_R6,    CR_BK[25]);
	//RTMP_IO_WRITE32(pAd,CORE_R4,   CR_BK[26]);
	RTMP_IO_WRITE32(pAd,TXBE_R1,    CR_BK[27]);
	RTMP_IO_WRITE32(pAd,AGC1_R0,    CR_BK[28]);
	RTMP_IO_WRITE32(pAd,TXBE_R4,    CR_BK[29]);
	RTMP_IO_WRITE32(pAd,CAL_R2,     CR_BK[30]);
	RTMP_IO_WRITE32(pAd,CAL_R5,     CR_BK[31]);
	RTMP_IO_WRITE32(pAd,CAL_R1,     CR_BK[32]);	
	RTMP_IO_WRITE32(pAd,TXBE_R5,    CR_BK[33]);
	RTMP_IO_WRITE32(pAd,PWR_PIN_CFG,CR_BK[34]);	

    //Reset bbp
    RTMP_IO_WRITE32(pAd,CORE_R4, 0x00000003); //core soft reset enable
    RTMP_IO_WRITE32(pAd,CORE_R4, 0x00000000);   //core soft reset disable

	return calStatusReport;
}
#endif

/*
	ITxBFDividerCalibration - perform divider calibration
		calFunction - the function to perform
			0=>Display cal param,
			1=>Update EEPROM
			2=>Update BBP
			3=>Just return the quantized divider phase in divPhase
			10=> Display params and dump capture data
		calMethod - the calibration method to use. 0=>use default method for the band
		divPhase - if not NULL, returns the quantized divider phase (0, +/-90, 180 for 2.4G, 0,180 for 5G)
	returns TRUE if no errors
*/
#define ITXBF_MAX_WAIT_CYCLE	10000


#ifdef MT76x2
INT mt76x2_ITxBFDividerCalibration(
	IN RTMP_ADAPTER *pAd,
	IN int calFunction,
	IN int calMethod,
	OUT UCHAR *divPhase)
{
	
	ITXBF_DIV_PARAMS divParams;
	UCHAR channel = pAd->CommonCfg.Channel;
	UCHAR phase[2];
    UCHAR preVGAGainIdx[2];
	int result = TRUE;
	UINT32 value32;
    int chanidx,groupIdx;

	UCHAR divPhaseValue[2];
	ITXBF_PHASE_PARAMS phaseParams;
	UCHAR i,phaseValues[2];

#ifdef TIMESTAMP_CAL_CAPTURE1
	struct timeval tval0, tval1;
#endif
#ifdef TIMESTAMP_CAL_CAPTURE1
	do_gettimeofday(&tval0);
#endif

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		channel = pAd->ate.Channel;
#endif /* RALINK_ATE */

	/* Handle optional divPhase parameter */
	if (divPhase == NULL)
		divPhase = divPhaseValue;

#ifdef TIMESTAMP_CAL_CAPTURE1
	do_gettimeofday(&tval1);
#endif

	/* Do Calibration */
	// Optional A-Band or G-Band. RF Cal with each TX active */
	//CalcDividerPhase(pAd, phase);  
    CalcDividerPhase(pAd, TRUE, preVGAGainIdx, phase);

	/* Calculate difference */
	divPhase[0] = phase[0] - phase[1];
	
    DBGPRINT(RT_DEBUG_WARN, (
			"############## DebugKK[0] divderPase=0x%x,divderPase_0=0x%x,divderPase_1=0x%x\n",divPhase[0],phase[0],phase[1]));

	/* Compute the quantized delta phase */
	/* Quantize to 180 deg (0x80) with rounding */
	DBGPRINT(RT_DEBUG_WARN, (
				"HW Phase vs Driver Phase (deg)\n"
				"ANT0/0 :%d\n"
				"ANT1/1 :%d\n"
				"divPhase[0] : %d\n",
				(360*phase[0])>> 8,
				(360*phase[1])>> 8,
				(360*divPhase[0])>> 8));

	//divPhase[0] = (divPhase[0] + 0x40) & 0x80;	

	/* Either display parameters, update EEPROM, update BBP registers or dump capture data */
	switch (calFunction) {
	case 0:
		break;

	case 1:		
		/*
			Save new reference values in EEPROM. The new reference is just the delta phase
			values with the old ref value added back in
		*/
		ITxBFGetEEPROM(pAd, 0, 0, &divParams, 0);

		/* Only allow calibration on specific channels */
		if (channel == 1)
			divParams.E1gBeg = divPhase[0];
		else if (channel == 14)
			divParams.E1gEnd = divPhase[0];
		else if (channel == 36)
			divParams.E1aLowBeg = divPhase[0];
		else if (channel == 64)
			divParams.E1aLowEnd = divPhase[0];
		else if (channel == 100)
			divParams.E1aMidBeg = divPhase[0];
		else if (channel == 120)
			divParams.E1aMidMid = divPhase[0];
		else if (channel == 140)
			divParams.E1aMidEnd = divPhase[0];
		else if (channel == 149)
			divParams.E1aHighBeg = divPhase[0];
		else if (channel == 173)
			divParams.E1aHighEnd = divPhase[0];
		else {
			DBGPRINT(RT_DEBUG_ERROR, ("Invalid channel: %d\nMust calibrate channel 1, 14, 36, 64, 100, 120, 140, 149 or 173", channel) );
			result = FALSE;
			goto exitDivCal;
		}

		ITxBFSetEEPROM(pAd, 0, 0, &divParams, 0);
		break;

	case 2:
		/*
			Update BBP Registers. Quantize DeltaPhase to 90 or 180 depending on band. Then
			update original phase calibration values from EEPROM and set R176 for Ant 0 and Ant2
		*/
		ITxBFGetEEPROM(pAd, &phaseParams, 0, 0, 0);
		mt76x2_ITxBFPhaseParams(pAd, phaseValues, channel, &phaseParams);

        groupIdx = ate_txbf_chan_group_base_idx(pAd, channel);
        ITxBFGetEEPROM(pAd, 0, 0,  &divParams, 0);
        
        divPhase[0] -= divParams.E1aDivPhase[groupIdx];
        divPhase[0] = (divPhase[0] + 0x40) & 0x80; 

		/* Ant0 */
		RTMP_IO_WRITE32(pAd, CAL_R0, 0);
		RTMP_IO_WRITE32(pAd, TXBE_R13, phaseValues[0] + divPhase[0]);  // for method1
		//RTMP_IO_WRITE32(pAd, TXBE_R13, 0xC4);

		DBGPRINT(RT_DEBUG_WARN, (
			   " ============================================================ \n"
		       " Divider phase  = 0x%x\n"
		       " Residual phase = 0x%x\n"
		       " Tx phase compensated value = 0x%x\n"
		       " ============================================================ \n", 
		       divPhase[0], phaseValues[0], phaseValues[0] + divPhase[0]));

		/* Enable TX Phase Compensation */
		RTMP_IO_READ32(pAd, TXBE_R12, &value32);
		RTMP_IO_WRITE32(pAd, TXBE_R12, value32 | 0x08);
		break;

	case 3:
		/*
			Update BBP Registers. Quantize DeltaPhase to 90 or 180 depending on band. Then
			update original phase calibration values from EEPROM and set R176 for Ant 0 and Ant2
		*/
		ITxBFGetEEPROM(pAd, &phaseParams, 0, 0, 0);
		mt76x2_ITxBFPhaseParams(pAd, phaseValues, channel, &phaseParams);

        groupIdx = ate_txbf_chan_group_base_idx(pAd, channel);
        ITxBFGetEEPROM(pAd, 0, 0, &divParams, 0);
        
        divPhase[0] -= divParams.E1aDivPhase[groupIdx];
        divPhase[0] = (divPhase[0] + 0x40) & 0x80;  

		/* Ant0 */
		RTMP_IO_WRITE32(pAd, CAL_R0, 0);
		RTMP_IO_WRITE32(pAd, TXBE_R13, phaseValues[0] + divPhase[0]);  
		
		DBGPRINT(RT_DEBUG_WARN, (
				" ============================================================ \n"
                " Group base [%d] divider phase = 0x%x\n"
		   		" Divider phase  = 0x%x\n"
		       	" Residual phase = 0x%x\n"
		       	" Tx phase compensated value = 0x%x\n"
		       	" ============================================================ \n", 
		       	groupIdx,divParams.E1aDivPhase[groupIdx],divPhase[0], phaseValues[0], phaseValues[0] + divPhase[0]));

		break;
	case 10:
        /*Save divPhase in efuse if its a group center channel */        
        /* Update EEPROM */        
        ITxBFGetEEPROM(pAd, 0, 0, &divParams, 0);

        chanidx = ate_txbf_get_chan_idx(pAd, channel, 0);
        groupIdx = ate_txbf_chan_group_base_idx(pAd, channel);
        if (chanidx != -1)  // group center chan
        {       
            divParams.E1aDivPhase[chanidx] = divPhase[0]; 
            ITxBFSetEEPROM(pAd, 0, 0, &divParams, 0);
            divPhase[0] = 0;
          
            DBGPRINT(RT_DEBUG_WARN, (
			   " ============================================================ \n"
		       " Save Divider phase to efuse[%d], value = 0x%x\n"
		       " ============================================================ \n", 
		       chanidx, divParams.E1aDivPhase[chanidx]));
        }
        else if (groupIdx >= 0) 
        {             
            divPhase[0] -= divParams.E1aDivPhase[groupIdx];
            divPhase[0] = (divPhase[0] + 0x40) & 0x80;                        

            DBGPRINT(RT_DEBUG_WARN, (
			     " ============================================================ \n"
		         " group index  = %d\n"
                 " Base Divider phase  = 0x%x\n"
		         " Diff Divider phase  = 0x%x\n"
		         " ============================================================ \n", 
		         groupIdx, divParams.E1aDivPhase[groupIdx], divPhase[0]));
        }
		break;

	default:
		result = FALSE;
		break;
	}

exitDivCal:
#ifdef TIMESTAMP_CAL_CAPTURE1
	do_gettimeofday(&tval1);
	
	DBGPRINT(RT_DEBUG_ERROR, ("%s t1=%ld\n", __FUNCTION__,
			tval1.tv_usec - tval0.tv_usec));
#endif

	return result;
}

#endif

#define MAX_LNA_CAPS	10		/* Maximum number of LNA captures per calibration */



#ifdef MT76x2
INT ITxBFLNACalibrationStartUp(
		IN RTMP_ADAPTER *pAd,
		IN INT calFunction,
		IN INT calMethod,
		IN BOOLEAN gBand)
{

	UINT *pCR_BK;
	BOOLEAN cal_StatusFlg = FALSE;

	/* Allocate buffer for capture data */
	pCR_BK = kmalloc(67 * 4, MEM_ALLOC_FLAG);
	if (pCR_BK == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Alloc memory failed\n", __FUNCTION__));
		return FALSE;
	}

	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX17,  &pCR_BK[0]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX38,  &pCR_BK[1]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX39,  &pCR_BK[2]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX42,  &pCR_BK[3]);
	mt_rf_read(pAd, RF_Path0, A_BAND_PA,     &pCR_BK[4]);
	mt_rf_read(pAd, RF_Path0, A_BAND_IQM_TSSI_DIV_LPF,  &pCR_BK[5]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TOP4,   &pCR_BK[6]); 
	mt_rf_read(pAd, RF_Path0, RFDIGI_TOP0,   &pCR_BK[7]); 
	mt_rf_read(pAd, RF_Path0, RFDIGI_TOP1,   &pCR_BK[8]); 
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX0,   &pCR_BK[9]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX4,   &pCR_BK[10]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_ABB_TO_AFE5, &pCR_BK[11]);
	mt_rf_read(pAd, RF_Path0, 0x034,         &pCR_BK[12]);
	mt_rf_read(pAd, RF_Path0, 0x044,         &pCR_BK[13]);
	mt_rf_read(pAd, RF_Path0, 0x04C,         &pCR_BK[14]);
	mt_rf_read(pAd, RF_Path0, 0x12C,         &pCR_BK[15]);
	//mt_rf_read(pAd, RF_Path0, 0x284,         &pCR_BK[16]);
	//mt_rf_read(pAd, RF_Path0, 0x288,         &pCR_BK[17]);
	//mt_rf_read(pAd, RF_Path0, 0x28C,         &pCR_BK[18]);
	//mt_rf_read(pAd, RF_Path0, 0x290,         &pCR_BK[19]);
	mt_rf_read(pAd, RF_Path0, 0x298,         &pCR_BK[20]);
	mt_rf_read(pAd, RF_Path0, 0x2A0,         &pCR_BK[21]);
	mt_rf_read(pAd, RF_Path0, 0x26C,         &pCR_BK[22]);
	mt_rf_read(pAd, RF_Path0, 0x270,         &pCR_BK[23]);
	mt_rf_read(pAd, RF_Path0, 0x274,         &pCR_BK[24]);
	mt_rf_read(pAd, RF_Path0, 0x2A4,         &pCR_BK[25]);
	mt_rf_read(pAd, RF_Path0, 0x03C,         &pCR_BK[26]);

	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX17,  &pCR_BK[27]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX38,  &pCR_BK[28]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX39,  &pCR_BK[29]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX42,  &pCR_BK[30]);
	mt_rf_read(pAd, RF_Path1, A_BAND_PA,     &pCR_BK[31]);
	mt_rf_read(pAd, RF_Path1, A_BAND_IQM_TSSI_DIV_LPF,  &pCR_BK[32]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TOP4,   &pCR_BK[33]); 
	mt_rf_read(pAd, RF_Path1, RFDIGI_TOP0,   &pCR_BK[34]); 
	mt_rf_read(pAd, RF_Path1, RFDIGI_TOP1,   &pCR_BK[35]); 
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX0,   &pCR_BK[36]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX4,   &pCR_BK[37]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_ABB_TO_AFE5, &pCR_BK[38]);
	mt_rf_read(pAd, RF_Path1, 0x034,         &pCR_BK[39]);
	mt_rf_read(pAd, RF_Path1, 0x044,         &pCR_BK[40]);
	mt_rf_read(pAd, RF_Path1, 0x04C,         &pCR_BK[41]);
	mt_rf_read(pAd, RF_Path1, 0x12C,         &pCR_BK[42]);
	//mt_rf_read(pAd, RF_Path1, 0x284,         &pCR_BK[43]);
	//mt_rf_read(pAd, RF_Path1, 0x288,         &pCR_BK[44]);
	//mt_rf_read(pAd, RF_Path1, 0x28C,         &pCR_BK[45]);
	//mt_rf_read(pAd, RF_Path1, 0x290,         &pCR_BK[46]);
	mt_rf_read(pAd, RF_Path1, 0x298,         &pCR_BK[47]);
	mt_rf_read(pAd, RF_Path1, 0x2A0,         &pCR_BK[48]);
	mt_rf_read(pAd, RF_Path1, 0x26C,         &pCR_BK[49]);
	mt_rf_read(pAd, RF_Path1, 0x270,         &pCR_BK[50]);
	mt_rf_read(pAd, RF_Path1, 0x274,         &pCR_BK[51]);
	mt_rf_read(pAd, RF_Path1, 0x2A4,         &pCR_BK[52]);
	mt_rf_read(pAd, RF_Path1, 0x03C,         &pCR_BK[53]);

	RTMP_IO_READ32(pAd,CORE_R1,              &pCR_BK[54]);
	RTMP_IO_READ32(pAd,CORE_R4,   			 &pCR_BK[55]);
	RTMP_IO_READ32(pAd,CORE_R33,             &pCR_BK[56]);
	RTMP_IO_READ32(pAd,DACCLK_EN_DLY_CFG,	 &pCR_BK[57]);
	RTMP_IO_READ32(pAd,TXBE_R1,   			 &pCR_BK[58]);
	RTMP_IO_READ32(pAd,TXBE_R5,              &pCR_BK[59]);
	RTMP_IO_READ32(pAd,TXBE_R6, 			 &pCR_BK[60]);
	RTMP_IO_READ32(pAd,AGC1_R0, 			 &pCR_BK[61]);
	RTMP_IO_READ32(pAd,CAL_R1, 			     &pCR_BK[62]);
	RTMP_IO_READ32(pAd,CAL_R2,  			 &pCR_BK[63]);
	RTMP_IO_READ32(pAd,CAL_R5,  			 &pCR_BK[64]);
	RTMP_IO_READ32(pAd,PWR_PIN_CFG,          &pCR_BK[65]);
	RTMP_IO_READ32(pAd,TXBE_R4,              &pCR_BK[66]);

	// Do the LNA calibration
	cal_StatusFlg = mt76x2_ITxBFLNACalibration(pAd, calFunction, calMethod, gBand);

	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX17, pCR_BK[0]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX38, pCR_BK[1]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX39, pCR_BK[2]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX42, pCR_BK[3]);
	mt_rf_write(pAd, RF_Path0, A_BAND_PA,    pCR_BK[4]);
	mt_rf_write(pAd, RF_Path0, A_BAND_IQM_TSSI_DIV_LPF,  pCR_BK[5]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TOP4,  pCR_BK[6]); 
	//mt_rf_write(pAd, RF_Path0, RFDIGI_TOP0,  pCR_BK[7]); 
	mt_rf_write(pAd, RF_Path0, RFDIGI_TOP1,  pCR_BK[8]); 
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX0,  pCR_BK[9]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX4,  pCR_BK[10]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_ABB_TO_AFE5, pCR_BK[11]);
	mt_rf_write(pAd, RF_Path0, 0x034,        pCR_BK[12]);
	mt_rf_write(pAd, RF_Path0, 0x044,        pCR_BK[13]);
	mt_rf_write(pAd, RF_Path0, 0x04C,        pCR_BK[14]);
	mt_rf_write(pAd, RF_Path0, 0x12C,        pCR_BK[15]);
	//mt_rf_write(pAd, RF_Path0, 0x284,        pCR_BK[16]);
	//mt_rf_write(pAd, RF_Path0, 0x288,        pCR_BK[17]);
	//mt_rf_write(pAd, RF_Path0, 0x28C,        pCR_BK[18]);
	//mt_rf_write(pAd, RF_Path0, 0x290,        pCR_BK[19]);
	mt_rf_write(pAd, RF_Path0, 0x298,        pCR_BK[20]);
	mt_rf_write(pAd, RF_Path0, 0x2A0,        pCR_BK[21]);
	mt_rf_write(pAd, RF_Path0, 0x26C,        pCR_BK[22]);
	mt_rf_write(pAd, RF_Path0, 0x270,        pCR_BK[23]);
	mt_rf_write(pAd, RF_Path0, 0x274,        pCR_BK[24]);
	mt_rf_write(pAd, RF_Path0, 0x2A4,        pCR_BK[25]);
	mt_rf_write(pAd, RF_Path0, 0x03C,        pCR_BK[26]);

	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX17, pCR_BK[27]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX38, pCR_BK[28]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX39, pCR_BK[29]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX42, pCR_BK[30]);
	mt_rf_write(pAd, RF_Path1, A_BAND_PA,    pCR_BK[31]);
	mt_rf_write(pAd, RF_Path1, A_BAND_IQM_TSSI_DIV_LPF,  pCR_BK[32]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TOP4,  pCR_BK[33]); 
	//mt_rf_write(pAd, RF_Path1, RFDIGI_TOP0,  pCR_BK[34]); 
	mt_rf_write(pAd, RF_Path1, RFDIGI_TOP1,  pCR_BK[35]); 
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX0,  pCR_BK[36]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX4,  pCR_BK[37]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_ABB_TO_AFE5, pCR_BK[38]);
	mt_rf_write(pAd, RF_Path1, 0x034,        pCR_BK[39]);
	mt_rf_write(pAd, RF_Path1, 0x044,        pCR_BK[40]);
	mt_rf_write(pAd, RF_Path1, 0x04C,        pCR_BK[41]);
	mt_rf_write(pAd, RF_Path1, 0x12C,        pCR_BK[42]);
	//mt_rf_write(pAd, RF_Path1, 0x284,        pCR_BK[43]);
	//mt_rf_write(pAd, RF_Path1, 0x288,        pCR_BK[44]);
	//mt_rf_write(pAd, RF_Path1, 0x28C,        pCR_BK[45]);
	//mt_rf_write(pAd, RF_Path1, 0x290,        pCR_BK[46]);
	mt_rf_write(pAd, RF_Path1, 0x298,        pCR_BK[47]);
	mt_rf_write(pAd, RF_Path1, 0x2A0,        pCR_BK[48]);
	mt_rf_write(pAd, RF_Path1, 0x26C,        pCR_BK[49]);
	mt_rf_write(pAd, RF_Path1, 0x270,        pCR_BK[50]);
	mt_rf_write(pAd, RF_Path1, 0x274,        pCR_BK[51]);
	mt_rf_write(pAd, RF_Path1, 0x2A4,        pCR_BK[52]);
	mt_rf_write(pAd, RF_Path1, 0x03C,        pCR_BK[53]);
	RTMP_IO_WRITE32(pAd,CORE_R1,             pCR_BK[54]);
	RTMP_IO_WRITE32(pAd,CORE_R4,   			 pCR_BK[55]);
	RTMP_IO_WRITE32(pAd,CORE_R33,            pCR_BK[56]);
	RTMP_IO_WRITE32(pAd,DACCLK_EN_DLY_CFG,	 pCR_BK[57]);
	RTMP_IO_WRITE32(pAd,TXBE_R1,   			 pCR_BK[58]);
	RTMP_IO_WRITE32(pAd,TXBE_R5,             pCR_BK[59]);
	RTMP_IO_WRITE32(pAd,TXBE_R6, 			 pCR_BK[60]);
	RTMP_IO_WRITE32(pAd,AGC1_R0, 			 pCR_BK[61]);
	RTMP_IO_WRITE32(pAd,CAL_R1, 			 pCR_BK[62]);
	RTMP_IO_WRITE32(pAd,CAL_R2,  			 pCR_BK[63]);
	RTMP_IO_WRITE32(pAd,CAL_R5,  			 pCR_BK[64]);
	RTMP_IO_WRITE32(pAd,PWR_PIN_CFG,         pCR_BK[65]);
	RTMP_IO_WRITE32(pAd,TXBE_R4,         	 pCR_BK[66]);

	os_free_mem(NULL, pCR_BK);

	return cal_StatusFlg;
}


INT    avgPhase32[3];
UCHAR  MidVGA[2];

INT mt76x2_ITxBFLNACalibration(
	IN RTMP_ADAPTER *pAd,
	IN INT calFunction,
	IN INT calMethod,
	IN BOOLEAN gBand)
{

	UINT32  phaseCaliStatus, phaseCaliResult;
	UCHAR   AGCtimeOutCount, timeOutCount, agcGain;
	INT     mPhase0[2], mPhase1[2];
	INT     mCalPhase0[2], mCalPhase1[2];
	INT     avgIData, avgQData, peakIValue;
	UINT32  value32;
	UINT32  TRXSel_aband[2] = {0x01FECD28, 0x01FECD28};
	UINT32  RXGainSel[4]    = {0x000B8888, 0x000A8888, 0x000A8888, 0x00098888};
	//UINT32  RXGainSel_aBand[4] = {0x000B8888, 0x000A8888, 0x000A8888, 0x00098888};
    UINT32  agcGainSet;

	UINT32  TRSW[2]       = {0xF004013B, 0xF004012A};
	UINT32  TRSW_aband[2] = {0x0004012F, 0x0004012A};
	UINT32  TxgGainSel[2] = {0x00010130, 0x00010100};
	UINT32  TXA_TXOpSel[2]= {0x015ACA00, 0x297A3A00};
	UINT32  TXA_PaSel[2]  = {0x657C0000, 0x32746000};
	UINT32  TRX_27Sel[2]  = {0x00507777, 0x00507757};
	UINT32  TRX_38Sel[2]  = {0x02E0A0A4, 0x03030300};
	UINT32  TRX_39Sel[2]  = {0x3F10100E, 0x2E1F1F10};
	UINT32  TRX_40Sel[2]  = {0x00000000, 0xDC1A0014};
	UINT32  TRX_41Sel[2]  = {0x00000700, 0x2020477F};
	UINT32  TRX_42Sel[2]  = {0x000002A8, 0x016E1D28};
	UCHAR   i, ii;
	UCHAR   pathIdx[2] = {0, 1};
	UCHAR   TRLoopSel[2] = {0x00, 0x05};
	USHORT  ADCSel[2] = {0x7400, 0x7401}, DACSel[2] = {0x80, 0x81};
	INT     DiffPhase32[3];

	UCHAR channel = pAd->CommonCfg.Channel;

	ITXBF_LNA_PARAMS lnaParams;
	int result = TRUE;

#ifdef TIMESTAMP_CAL_CAPTURE1
	struct timeval tval0, tval1;
#endif

#ifdef TIMESTAMP_CAL_CAPTURE1
	do_gettimeofday(&tval0);
#endif

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		channel = pAd->ate.Channel;
#endif /* RALINK_ATE */

	for (i = 0; i < 2; i++) // HM, ML
	{
		for (ii = 0; ii < 2; ii++) // Path0 and path1
		{
			if (gBand)
			{
				//********* HG LNA Phase cal  *********
				mt_rf_write(pAd, pathIdx[ii], 0x0244, 0x00010100); //Set txg gain table = to manual mode
				mt_rf_write(pAd, pathIdx[ii], 0x0044, 0x02512016); //TRSWITCH
	
				if (i == 0)
				{
					mt_rf_write(pAd, pathIdx[ii], 0x0034, 0x00776058);
					mt_rf_write(pAd, pathIdx[ii], 0x004C, 0x6F090293);
					mt_rf_write(pAd, pathIdx[ii], 0x012C, 0x08100038);
				}
	
				//********* external llopback *********
				mt_rf_write(pAd, pathIdx[ii], 0x0010, 0x31C70006); // Force Divider ON
				mt_rf_write(pAd, pathIdx[ii], 0x0114, 0x00C211F1); // RSV LBpath to RX
				mt_rf_write(pAd, pathIdx[ii], 0x0000, 0x80056F53); // enable SX/TX/PA/RX for WF0, force chipmode in RX mode
				mt_rf_write(pAd, pathIdx[ii], 0x0004, TRSW[i]); // set TRSW in TX mode for WF0
			}
			else
			{
				//********* HG LNA Phase cal  *********
				mt_rf_write(pAd, pathIdx[ii], 0x0244, TxgGainSel[i]); //Set txg gain table = to manual mode
			
				//********* external llopback *********
				mt_rf_write(pAd, pathIdx[ii], 0x000C, 0x31C73047); // Force Divider ON
				mt_rf_write(pAd, pathIdx[ii], 0x0000, 0x80056F37); // enable SX/TX/PA/RX for WF1, force chipmode in RX mode
				mt_rf_write(pAd, pathIdx[ii], 0x0004, TRSW_aband[i]); // set TRSW in TX mode for WF1
			}

			//********* external PA Setting********
			if (gBand)
			{
				mt_rf_write(pAd, pathIdx[ii], 0x0054, TXA_TXOpSel[i]);
				mt_rf_write(pAd, pathIdx[ii], 0x0058, TXA_PaSel[i]);
			}
			else
			{
				mt_rf_write(pAd, pathIdx[ii], 0x0054, 0x297A3A00);
				mt_rf_write(pAd, pathIdx[ii], 0x0058, 0x012C6000);
			}
				
			mt_rf_write(pAd, pathIdx[ii], 0x0298, 0x03030300); 

			if (gBand)
			{
				mt_rf_write(pAd, pathIdx[ii], 0x029C, TRX_39Sel[i]);
			}
			else
			{
				mt_rf_write(pAd, pathIdx[ii], 0x027C, 0x00250027);
				mt_rf_write(pAd, pathIdx[ii], 0x0280, 0x00670024);
				mt_rf_write(pAd, pathIdx[ii], 0x0284, 0x00640065);
				mt_rf_write(pAd, pathIdx[ii], 0x0288, 0x00E500E7);
				mt_rf_write(pAd, pathIdx[ii], 0x028C, 0x03E700E4);
				mt_rf_write(pAd, pathIdx[ii], 0x0290, 0x03E403E5);
				mt_rf_write(pAd, pathIdx[ii], 0x0294, 0x03E103E3);
				mt_rf_write(pAd, pathIdx[ii], 0x029C, 0x2E1F1F10);
			}
				
    		RTMP_IO_WRITE32(pAd, 0x2704, 0x00000000); 

			if (gBand)
			{
				mt_rf_write(pAd, pathIdx[ii], 0x02A0, TRX_40Sel[i]); 
				mt_rf_write(pAd, pathIdx[ii], 0x02A8, TRX_42Sel[i]); 
				mt_rf_write(pAd, pathIdx[ii], 0x026c, TRX_27Sel[i]); // Gband Table
				mt_rf_write(pAd, pathIdx[ii], 0x0270, 0x0C0D0F04); // Gband Table
				mt_rf_write(pAd, pathIdx[ii], 0x0274, 0x13141517); // Gband Table
				mt_rf_write(pAd, pathIdx[ii], 0x0298, TRX_38Sel[i]); 
				mt_rf_write(pAd, pathIdx[ii], 0x02A4, TRX_41Sel[i]);      
				mt_rf_write(pAd, pathIdx[ii], 0x003C, 0x05B559C5); // set TXG gain for ePA   
			}
			else
			{
				mt_rf_write(pAd, pathIdx[ii], 0x02A8, TRXSel_aband[i]); 
			}

            AGCtimeOutCount = 0;
            agcGain = RXGainSel[2*i] & 0x3F; 

            while (AGCtimeOutCount < 20) // SW AGC update to make sure I peak value can prevent peak value from satuation or too low
            {
		        //********* cal setting ********
    		    RTMP_IO_WRITE32(pAd,          0x2004, 0x00000000); // BW=20MHz ADC=40MHz
			    mt_rf_write(pAd, pathIdx[ii], 0x0200, 0x0500010F); // start rxiq dcoc
    		    RTMP_IO_WRITE32(pAd,          0x1264, 0x80008000); // DAC Clock on
    		    RTMP_IO_WRITE32(pAd,          0x1204, 0x00000000); // [3] to turn off "adda power down"
    		    RTMP_IO_WRITE32(pAd,          0x2084, 0x00021E00); // RF/BBP clk control
    		    RTMP_IO_WRITE32(pAd,          0x2010, 0x00000001); // core soft reset enable
    		    RTMP_IO_WRITE32(pAd,          0x2010, 0x00000000); // core soft reset disable
    		    RTMP_IO_WRITE32(pAd,          0x2704, 0x00000000); // Tx power control

                agcGainSet = (RXGainSel[2*i] & 0xF8080)|(agcGain << 8)| agcGain ; 

			    //********* HG LNA Phase cal  *********
                mt_rf_write(pAd, pathIdx[ii], 0x0210, agcGainSet); // Use 2cf4 force RXGAIN, HG(B)/MG(A)/8484 LG(9)/8585
            	
			    //********* start iBF cal
			    mt_rf_write(pAd, pathIdx[ii], 0x0114, 0x00C211F1); // connecting TxLPF out and RxLPF in, and closing the TIA
    		    RTMP_IO_WRITE32(pAd,          0x2300, ADCSel[ii]); // 01:1R ADC1, 00 : 1R ADC0
    		    RTMP_IO_WRITE32(pAd,          0x2710, 0x00000008); // a default setting, 2T
    		    RTMP_IO_WRITE32(pAd,          0x2c08, TRLoopSel[ii]); // TR loop setting(0:T0R0,1:T1R0,5:T1R1,4:T0R1)
    		    RTMP_IO_WRITE32(pAd,          0x2714, DACSel[ii]); // 1 : DAC1, 0 : DAC0                                                    
    		    RTMP_IO_WRITE32(pAd,          0x2718, 0xC0002101); // Test format contol : Tx single tone setting

			    RtmpOsMsDelay(10); // waiting 10ms

    		    RTMP_IO_WRITE32(pAd,          0x2c14, 0x0000040c); // set accumulation length

			    // Enable LNA phase calibration and polling if HW is ready to read phase result
			    RTMP_IO_WRITE32(pAd,CAL_R1, 0x00000086);

			    RtmpOsMsDelay(1); // waiting 1ms
			
			    RTMP_IO_READ32(pAd,CAL_R1, &phaseCaliStatus);
			    timeOutCount = 0;
				while (phaseCaliStatus & 0x80)
				{
					if (timeOutCount == 3) 
					{
						DBGPRINT(RT_DEBUG_WARN, (
								"LNA HW calibration can't finish process\n"
				    			"phaseCaliStatus = %x\n", phaseCaliStatus));
						break;
					}

					timeOutCount++;

				    RtmpOsMsDelay(1); // waiting 1ms
			
					RTMP_IO_READ32(pAd,CAL_R1, &phaseCaliStatus);
				}

				// 0x2C2C
				// Bit 23:16    Correlator Phase
				// Bit 15:8     Correlator Q value
				// Bit 7:0      Correlator I value
				RTMP_IO_READ32(pAd,CAL_R11, &phaseCaliResult);
				mPhase0[ii] = (INT)((INT)((phaseCaliResult << 8) & 0xFF000000) >> 24);
				avgQData = (INT)((INT)((phaseCaliResult << 16) & 0xFF000000) >> 24);
				avgIData = (INT)((INT)((phaseCaliResult << 24) & 0xFF000000) >> 24);
				mCalPhase0[ii] = iAtan2(avgQData, avgIData);
				peakIValue = phaseCaliResult >> 24;

			    DBGPRINT(RT_DEBUG_TRACE, (
					" ==========================LNACAL= Loop%d, Path%d, agcCnt %d,agcGain=%d,High ======================== \n"
					" agcGainSet = %x\n"
					" CAL_R11=0x%x\n" 
					" Peak I value=0x%x\n"
					" I value=0x%x, Q value=0x%x\n"
					" mPhase0 = 0x%x\n"
					" ============================================================ \n", 
					i, ii,AGCtimeOutCount,agcGain,
                    agcGainSet,
					phaseCaliResult, 
					peakIValue,
					avgIData, avgQData,
					mPhase0[ii]));
			    
                //SW AGC gain control
                if ((agcGain < 62) && (AGCtimeOutCount < 19)) // BP_AGC gain 0~63
                {
                    // Calculate AGC ad check if AGC update is enough or not
                    if (peakIValue >= 0x40 && agcGain > 0)
                        agcGain --;
                    else if (peakIValue >= 0x30)
                    {
                        AGCtimeOutCount = 20;
                        break;
                    } 
                    else
                        agcGain++;
                }
                DBGPRINT(RT_DEBUG_TRACE,("LNA SW AGC = %d\n", agcGain));
                AGCtimeOutCount++;  
            }

			//********* BBP Soft Reset  *********
			RTMP_IO_WRITE32(pAd, 0x2718, 0x40002101); //Test format contol : Tx single tone setting
			RTMP_IO_WRITE32(pAd, 0x2010, 0x00000001); // core soft reset enable
			RTMP_IO_WRITE32(pAd, 0x2010, 0x00000000); // core soft reset disable
				
			//********* MG LNA Phase cal  *********
            agcGainSet = (RXGainSel[2*i + 1] & 0xF8080)|(agcGain << 8)| agcGain ; 
			if (gBand)
			{
				mt_rf_write(pAd, pathIdx[ii], 0x0004, TRSW[i]);    // set TRSW in TX mode for WF0
				mt_rf_write(pAd, pathIdx[ii], 0x0210, agcGainSet); // Use 2cf4 force RXGAIN, HG(B)/MG(A)/8484 LG(9)/8585
				//mt_rf_write(pAd, pathIdx[ii], 0x0210, RXGainSel[2*i + 1]); // Use 2cf4 force RXGAIN, HG(B)/MG(A)/8484 LG(9)/8585
				//mt_rf_write(pAd, pathIdx[ii], 0x0210, RXGainSel[2*i + 1]); // Use 2cf4 force RXGAIN, HG(B)/MG(A)/8484 LG(9)/8585
			}
			else
			{
				mt_rf_write(pAd, pathIdx[ii], 0x0004, TRSW_aband[i]); // set TRSW in TX mode for WF0
				mt_rf_write(pAd, pathIdx[ii], 0x0210, agcGainSet); // Use 2cf4 force RXGAIN, HG(B)/MG(A)/8484 LG(9)/8585
				//mt_rf_write(pAd, pathIdx[ii], 0x0210, RXGainSel_aBand[2*i + 1]); // Use 2cf4 force RXGAIN, HG(B)/MG(A)/8484 LG(9)/8585
			}
			
			//********* start iBF cal *********
			mt_rf_write(pAd, pathIdx[ii], 0x0114, 0x00C211F1); // connecting TxLPF out and RxLPF in, and closing the TIA
			RTMP_IO_WRITE32(pAd, 0x2300, ADCSel[ii]); // 01:1R ADC1, 00 : 1R ADC0
			RTMP_IO_WRITE32(pAd, 0x2710, 0x00000008); // a default setting, 2T
			RTMP_IO_WRITE32(pAd, 0x2c08, TRLoopSel[ii]); // TR loop setting(0:T0R0,1:T1R0,5:T1R1,4:T0R1)
			RTMP_IO_WRITE32(pAd, 0x2714, DACSel[ii]); // 1 : DAC1, 0 : DAC0   
			RTMP_IO_WRITE32(pAd, 0x2718, 0xC0002101); // Test format contol : Tx single tone setting
			RTMP_IO_WRITE32(pAd, 0x2c14, 0x0000040c); // set accumulation length

			// Enable LNA phase calibration and polling if HW is ready to read phase result
			RTMP_IO_WRITE32(pAd,CAL_R1, 0x00000086);

			RtmpOsMsDelay(1); // waiting 1ms
	
			RTMP_IO_READ32(pAd,CAL_R1, &phaseCaliStatus);
			timeOutCount = 0;
			while (phaseCaliStatus & 0x80)
			{
				if (timeOutCount == 3) 
				{
					DBGPRINT(RT_DEBUG_WARN, (
					"LNA HW calibration can't finish process\n"
					"phaseCaliStatus = %x\n", phaseCaliStatus));
					break;
				}
	
				timeOutCount++;
	
				RtmpOsMsDelay(1); // waiting 1ms
	
				RTMP_IO_READ32(pAd,CAL_R1, &phaseCaliStatus);
			}
	
			// 0x2C2C
			// Bit 23:16	Correlator Phase
			// Bit 15:8 	Correlator Q value
			// Bit 7:0		Correlator I value
			RTMP_IO_READ32(pAd,CAL_R11, &phaseCaliResult);
			mPhase1[ii] = (INT)((INT)((phaseCaliResult << 8) & 0xFF000000) >> 24);
			avgQData = (INT)((INT)((phaseCaliResult << 16) & 0xFF000000) >> 24);
			avgIData = (INT)((INT)((phaseCaliResult << 24) & 0xFF000000) >> 24);
			mCalPhase1[ii] = iAtan2(avgQData, avgIData);
			peakIValue = phaseCaliResult >> 24;

			DBGPRINT(RT_DEBUG_TRACE, (
					" ========================LNACAL= Loop%d, Path%d, Low ======================== \n"
					" agcGainSet = %x\n"
					" CAL_R11=0x%x\n" 
					" Peak I value=0x%x\n"
					" I value=0x%x, Q value=0x%x\n"
					" mPhase1 = 0x%x\n"
					" ============================================================ \n", 
					i, ii,
                    agcGainSet,
					phaseCaliResult, 
					peakIValue,
					avgIData, avgQData,
					mPhase1[ii]));

			if (gBand)
			{
				if (ii != 0)
				{
					mt_rf_write(pAd, RF_Path1, RFDIGI_TOP0, 0x00056F53); // WF1 manul mode
					mt_rf_write(pAd, RF_Path1, RFDIGI_TOP0, 0x00056F50); // WF1 manul mode
					mt_rf_write(pAd, RF_Path1, RFDIGI_TOP0, 0x80056F50); // WF1 manul mode
				}
				else
				{
					mt_rf_write(pAd, RF_Path0, RFDIGI_TOP0, 0x00056F53); // WF0 manul mode
					mt_rf_write(pAd, RF_Path0, RFDIGI_TOP0, 0x00056F50); // WF0 manul mode
					mt_rf_write(pAd, RF_Path0, RFDIGI_TOP0, 0x80056F50); // WF0 manul mode
				}
		    }
		}

		DiffPhase32[i] = ((INT)(((INT)((mPhase0[0]-mPhase1[0]) << 24)) >> 24) + 
			              (INT)(((INT)((mPhase0[1]-mPhase1[1]) << 24)) >> 24)) >> 1;

		DBGPRINT(RT_DEBUG_OFF, (
				" ============================ Loop%d ======================== \n"
				" ((mPhase0[0]-mPhase1[0])+(mPhase0[1]-mPhase1[1])) >> 1\n"
				" ((0x%x - 0x%x) + (0x%x - 0x%x)) >> 1 = 0x%x\n"
				" ============================================================ \n",
				i,
				mPhase0[0], mPhase1[0],
				mPhase0[1], mPhase1[1],
				DiffPhase32[i]));

	}

	avgPhase32[0] = DiffPhase32[0];
	avgPhase32[1] = DiffPhase32[1];
	avgPhase32[2] = DiffPhase32[1];

	DBGPRINT(RT_DEBUG_TRACE, (
			"Channel%d LNA average phase (deg)\n"
			"Ant0\n"
			"  PHM  :%d    Avg PHM  :%d\n"
			"  PML  :%d    Avg PML  :%d\n"
			"  PLUL :%d    Avg PLUL :%d\n",					
			channel,
			(360*DiffPhase32[0])>>8,(360*avgPhase32[0])>>8,
			(360*DiffPhase32[1])>>8,(360*avgPhase32[1])>>8,
			(360*DiffPhase32[1])>>8,(360*avgPhase32[1])>>8));

	/* Either display parameters, update EEPROM and BBP registers or dump capture data */
	switch (calFunction) {
	case 0:
		break;

	case 1:
		/* Save new reference values in EEPROM and BBP */
		ITxBFGetEEPROM(pAd, 0, &lnaParams, 0, 0);

		/* Only allow calibration on specific channels */
		if (channel == 1) {
			lnaParams.E1gBeg[0] = avgPhase32[0]; // H-M
			lnaParams.E1gBeg[1] = avgPhase32[1]; // M-L
			lnaParams.E1gBeg[2] = avgPhase32[2]; // L-uL = M-L
		}
		else if (channel == 14) {
			lnaParams.E1gEnd[0] = avgPhase32[0];
			lnaParams.E1gEnd[1] = avgPhase32[1];
			lnaParams.E1gEnd[2] = avgPhase32[2];
		}
		else if (channel == 36) {
			lnaParams.E1aLowBeg[0] = avgPhase32[0];
			lnaParams.E1aLowBeg[1] = avgPhase32[1];
			lnaParams.E1aLowBeg[2] = avgPhase32[2];
		}
		else if (channel == 64) {
			lnaParams.E1aLowEnd[0] = avgPhase32[0];
			lnaParams.E1aLowEnd[1] = avgPhase32[1];
			lnaParams.E1aLowEnd[2] = avgPhase32[2];
		}
		else if (channel == 100) {
			lnaParams.E1aMidBeg[0] = avgPhase32[0];
			lnaParams.E1aMidBeg[1] = avgPhase32[1];
			lnaParams.E1aMidBeg[2] = avgPhase32[2];
		}
		else if (channel == 120) {
			lnaParams.E1aMidMid[0] = avgPhase32[0];
			lnaParams.E1aMidMid[1] = avgPhase32[1];
			lnaParams.E1aMidMid[2] = avgPhase32[2];
		}
		else if (channel == 140) {
			lnaParams.E1aMidEnd[0] = avgPhase32[0];
			lnaParams.E1aMidEnd[1] = avgPhase32[1];
			lnaParams.E1aMidEnd[2] = avgPhase32[2];
		}
		else if (channel == 149) {
			lnaParams.E1aHighBeg[0] = avgPhase32[0];
			lnaParams.E1aHighBeg[1] = avgPhase32[1];
			lnaParams.E1aHighBeg[2] = avgPhase32[2];
		}
		else if (channel == 173) {
			lnaParams.E1aHighEnd[0] = avgPhase32[0];
	 		lnaParams.E1aHighEnd[1] = avgPhase32[1];
			lnaParams.E1aHighEnd[2] = avgPhase32[2];
		}
		else {
			DBGPRINT(RT_DEBUG_OFF,
					("Invalid channel: %d\nMust calibrate channel 1, 14, 36, 64, 100, 120, 140 ,149 or 173", channel) );
			result = FALSE;
			goto exitLnaCal;
		}

		ITxBFSetEEPROM(pAd, 0, &lnaParams, 0, 0);
		/* FALL THROUGH to update BBP */

		break;
		
	case 2:
		/* Update LNA phase difference into RXFE_R3 registers */
		RTMP_IO_READ32(pAd, CAL_R0, &value32);
		for (i=0; i<2; i++)
		{
			RTMP_IO_WRITE32(pAd, CAL_R0, value32 | (i<<5));
			RTMP_IO_WRITE32(pAd, RXFE_R3, avgPhase32[i]);
		}

		break;

	default:
		result = FALSE;
		break;
	}

exitLnaCal:

#ifdef TIMESTAMP_CAL_CAPTURE1
	do_gettimeofday(&tval1);
#endif

	return result;
}


INT ITxBFPhaseCalibrationStartUp(
	IN RTMP_ADAPTER *pAd,
	IN INT calFunction,
	IN BOOLEAN loOffsetCkFlg,
	IN USHORT ch)
{
	UINT32 CR_BK[35];

	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX17, &CR_BK[0]); 
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX38, &CR_BK[1]); 
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX39, &CR_BK[2]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX42, &CR_BK[3]);
	mt_rf_read(pAd, RF_Path0, A_BAND_PA,    &CR_BK[4]);
	mt_rf_read(pAd, RF_Path0, A_BAND_IQM_TSSI_DIV_LPF,  &CR_BK[5]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX0,  &CR_BK[6]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TRX4,  &CR_BK[7]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TOP0,  &CR_BK[8]);
	mt_rf_read(pAd, RF_Path0, RFDIGI_TOP1,  &CR_BK[9]);
	mt_rf_read(pAd, RF_Path0, RG_WF0_RXG_TOP,  &CR_BK[10]);
	
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX17, &CR_BK[11]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX38, &CR_BK[12]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX39, &CR_BK[13]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX42, &CR_BK[14]);
	mt_rf_read(pAd, RF_Path1, A_BAND_PA,    &CR_BK[15]);
	mt_rf_read(pAd, RF_Path1, A_BAND_IQM_TSSI_DIV_LPF,  &CR_BK[16]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX0,  &CR_BK[17]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TRX4,  &CR_BK[18]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TOP0,  &CR_BK[19]);
	mt_rf_read(pAd, RF_Path1, RFDIGI_TOP1,  &CR_BK[20]);
	mt_rf_read(pAd, RF_Path1, RG_WF0_RXG_TOP,  &CR_BK[21]);
	
	RTMP_IO_READ32(pAd, CORE_R1,            &CR_BK[22]);
	RTMP_IO_READ32(pAd, DACCLK_EN_DLY_CFG,  &CR_BK[23]);
	RTMP_IO_READ32(pAd, PWR_PIN_CFG,        &CR_BK[24]);
	RTMP_IO_READ32(pAd, CORE_R33,           &CR_BK[25]);
	RTMP_IO_READ32(pAd, AGC1_R0,            &CR_BK[26]);
	RTMP_IO_READ32(pAd, TXBE_R4,            &CR_BK[27]);
	RTMP_IO_READ32(pAd, CORE_R4,            &CR_BK[28]);
	RTMP_IO_READ32(pAd, TXBE_R1,            &CR_BK[29]);
	RTMP_IO_READ32(pAd, CAL_R2,  		    &CR_BK[30]);
	RTMP_IO_READ32(pAd, TXBE_R5, 		    &CR_BK[31]);
	RTMP_IO_READ32(pAd, TXBE_R6, 		    &CR_BK[32]);
	RTMP_IO_READ32(pAd, CAL_R5,  		    &CR_BK[33]);
	RTMP_IO_READ32(pAd, CAL_R1, 		    &CR_BK[34]);

	// Do the residual phase calibration
	mt76x2_ITxBFPhaseCalibration(pAd, calFunction, loOffsetCkFlg, ch);

	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX17, CR_BK[0]); 
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX38, CR_BK[1]); 
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX39, CR_BK[2]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX42, CR_BK[3]);
	mt_rf_write(pAd, RF_Path0, A_BAND_PA,    CR_BK[4]);
	mt_rf_write(pAd, RF_Path0, A_BAND_IQM_TSSI_DIV_LPF,  CR_BK[5]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX0,  CR_BK[6]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TRX4,  CR_BK[7]);
	mt_rf_write(pAd, RF_Path0, RFDIGI_TOP1,  CR_BK[9]);
	mt_rf_write(pAd, RF_Path0, RG_WF0_RXG_TOP,CR_BK[10]);
	
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX17, CR_BK[11]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX38, CR_BK[12]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX39, CR_BK[13]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX42, CR_BK[14]);
	mt_rf_write(pAd, RF_Path1, A_BAND_PA,    CR_BK[15]);
	mt_rf_write(pAd, RF_Path1, A_BAND_IQM_TSSI_DIV_LPF,  CR_BK[16]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX0,  CR_BK[17]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TRX4,  CR_BK[18]);
	mt_rf_write(pAd, RF_Path1, RFDIGI_TOP1,  CR_BK[20]);
	mt_rf_write(pAd, RF_Path1, RG_WF0_RXG_TOP,CR_BK[21]);
	
	RTMP_IO_WRITE32(pAd, CORE_R1,            CR_BK[22]);
	RTMP_IO_WRITE32(pAd, DACCLK_EN_DLY_CFG,	 CR_BK[23]);
	RTMP_IO_WRITE32(pAd, PWR_PIN_CFG,        CR_BK[24]);
	RTMP_IO_WRITE32(pAd, CORE_R33,           CR_BK[25]);
	RTMP_IO_WRITE32(pAd, AGC1_R0,            CR_BK[26]);
	RTMP_IO_WRITE32(pAd, TXBE_R4,            CR_BK[27]);
	RTMP_IO_WRITE32(pAd, CORE_R4,            CR_BK[28]);
	RTMP_IO_WRITE32(pAd, TXBE_R1,            CR_BK[29]);
	RTMP_IO_WRITE32(pAd, CAL_R2,  			 CR_BK[30]);
	RTMP_IO_WRITE32(pAd, TXBE_R5, 			 CR_BK[31]);
	RTMP_IO_WRITE32(pAd, TXBE_R6, 			 CR_BK[32]);
	RTMP_IO_WRITE32(pAd, CAL_R5,  			 CR_BK[33]);
	RTMP_IO_WRITE32(pAd, CAL_R1, 			 CR_BK[34]);

	return TRUE;
}


INT mt76x2_ITxBFPhaseCalibration(
	IN RTMP_ADAPTER *pAd,
	IN INT calFunction,
	IN BOOLEAN loOffsetCkFlg,
	IN USHORT ch)
{
	ITXBF_PHASE_PARAMS phaseParams;
	UINT32 phaseCaliStatus, phaseCaliResult;
	UINT32 value32[2];
	UCHAR  timeOutCount;
	INT    mPhase[2];
    INT    mCalPhase0;
	LONG   avgIData, avgQData;
	UCHAR  i;
	UCHAR   divPhase[2]    = {0};
	INT     phaseValues[2] = {0};
	BOOLEAN gBandFlg      = FALSE;
	CHAR    rf_Path[2]    = {RF_Path0, RF_Path1};
	CHAR    rf_RevPath[2] = {RF_Path1, RF_Path0};
	CHAR    trLoopSetting[2] = {4, 1};
	CHAR    A_D_Sel[2]    = {0x80, 0x81};
	CHAR    LoPhase;

#ifdef RALINK_ATE
	ch = pAd->ate.Channel;
#endif

	if (ch <= 14) gBandFlg = TRUE;

	/* DPD and TSSI HW off */
	RTMP_IO_READ32(pAd,TXBE_R8, &value32[0]);
	value32[0] &= ~0x08000;
	RTMP_IO_WRITE32(pAd,TXBE_R8, value32[0]); // DPD off

	RTMP_IO_READ32(pAd,CORE_R34, &value32[0]);
	value32[0] &= ~0x60;
	value32[0] |= 0x40;
	RTMP_IO_WRITE32(pAd,CORE_R34, value32[0]); // TSSI off

	mt_rf_write(pAd, RF_Path0, RFDIGI_TOP4,    0x30D71047); 	// tx block mode
	mt_rf_write(pAd, RF_Path1, RFDIGI_TOP4,    0x30D71047); 	// tx block mode		



	for (i = 0; i < 2; i++) // Tx1Rx0, Tx0RX1
	{
	if (gBandFlg)
	{
			// ********* RXA IQ Cal Setting T0R1 *********
			RTMP_IO_WRITE32(pAd, TXBE_R8, 0x4100000A);   // dpd off
			RTMP_IO_WRITE32(pAd, CORE_R34, 0x0008AC40); // TSSI off
			mt_rf_write(pAd, rf_Path[i], RFDIGI_TRX17, 0x00010170); // Set txg gain table = to manual mode
			mt_rf_write(pAd, rf_Path[i], RG_WF0_RXG_TOP, 0x00492016); // TRSWITCH

			// ********* external llopback *********
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP3, 0x11C73047); // Force Divider ON
			mt_rf_write(pAd, rf_Path[i], RFDIGI_TOP3, 0x11C73047); // Force Divider ON
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP0, 0x80056733); // enable SX/RX/for WF0,
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP1, 0xF0040122); // set TRSW in TX mode for WF0
			mt_rf_write(pAd, rf_Path[i], RFDIGI_TOP0, 0x80056F23); // enable SX/RX/for WF1
			mt_rf_write(pAd, rf_Path[i], RFDIGI_TOP1, 0xF0040123); // set TRSW in TX mode for WF1
	    }
	    else
	    {
			// ********* external llopback *********
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TRX17, 0x000101C0); // Set txg gain table = to manual mode
	
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP3, 0x11C73047); // Force Divider ON
			mt_rf_write(pAd, rf_Path[i], RFDIGI_TOP3, 0x11C73047); // Force Divider ON
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP0, 0x80056737); // enable SX/RX/for WF0,
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP1, 0xF0040122); // set TRSW in TX mode for WF0
			mt_rf_write(pAd, rf_Path[i], RFDIGI_TOP0, 0x80056F27); // enable SX/RX/for WF1
			mt_rf_write(pAd, rf_Path[i], RFDIGI_TOP1, 0xF0040123); // set TRSW in TX mode for WF1
	    }
	
		// ********* cal setting ********
	    // DCOC for RXA IQ Cal
	    RTMP_IO_WRITE32(pAd,CORE_R1, 0x00000000); // BW=20MHz ADC=40MHz

		// ********* pre dcoc ********
		RTMP_IO_WRITE32(pAd, RF_BSI_CKDIV, 0x00000008);
		mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TRX0, 0x0500010F); // start rxiq dcoc   keter

		RtmpOsMsDelay(1); // waiting 1ms          

		RTMP_IO_WRITE32(pAd, RF_BSI_CKDIV,      0x00000002);
	    RTMP_IO_WRITE32(pAd, DACCLK_EN_DLY_CFG, 0x80008000);  //DAC Clock on

		// ********* send single tone ******                                                                             
	    RTMP_IO_WRITE32(pAd,PWR_PIN_CFG, 0x00000000);  //[3] to turn off "adda power down"
	    RTMP_IO_WRITE32(pAd,CORE_R33,    0x00021E00);  //RF/BBP clk control
		RTMP_IO_WRITE32(pAd,TXBE_R6, 	 0x40002101);  // Test format contol : Tx single tone setting                                                                                                      
		RTMP_IO_WRITE32(pAd,CORE_R4,   	 0x00000001);  // core soft reset enable                                  

		RtmpOsMsDelay(1); // waiting 1ms     
	
		RTMP_IO_WRITE32(pAd,CORE_R4,     0x00000000); // core soft reset disable
		RTMP_IO_WRITE32(pAd,TXBE_R1,     0x00000606); // Tx power control

		// ********* LNA VGA setting : MG                                                                                              
	    //mt_rf_write(pAd, RF_Path0,	   RFDIGI_TRX4, ((1<<19)|(2<<16)|(1<<15)|(MidVGA[0]<<8)|(1<<7)|MidVGA[0]));
	    //mt_rf_write(pAd, RF_Path1,	   RFDIGI_TRX4, ((1<<19)|(2<<16)|(1<<15)|(MidVGA[1]<<8)|(1<<7)|MidVGA[1]));
		if (gBandFlg)
		{
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TRX4, ((1<<19)|(2<<16)|(1<<15)|(4<<8)|(1<<7)|4));                    
		}
		else
		{
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TRX4, ((1<<19)|(2<<16)|(1<<15)|(4<<8)|(1<<7)|4));                
		}

		// ********* start iBF cal
		RTMP_IO_WRITE32(pAd, AGC1_R0,  0x7408); // a default setting, 2R
		RTMP_IO_WRITE32(pAd, TXBE_R4,  0x0008); // a default setting, 2T
		RTMP_IO_WRITE32(pAd, CAL_R2,   trLoopSetting[i]); // TR loop setting(0:T0R0,1:T1R0,5:T1R1,4:T0R1)              
		RTMP_IO_WRITE32(pAd, TXBE_R5,  A_D_Sel[i]); // 1 : DAC1, 0 : DAC0 
		RTMP_IO_WRITE32(pAd, TXBE_R6,  0xC0002101); // Test format contol : Tx single tone setting

		RtmpOsMsDelay(1); // waiting 1ms   

		if (gBandFlg)
		{
		RTMP_IO_WRITE32(pAd,CAL_R5,  0x0000040c); // set accumulation length
		}
		else
		{
			RTMP_IO_WRITE32(pAd,CAL_R5,  0x00001c0c); // set accumulation length                                   
		}
		
		// Enable phase calibration
		RTMP_IO_WRITE32(pAd,CAL_R1, 0x00000086);
	
		RtmpOsMsDelay(1); // waiting 1ms   
		
		RTMP_IO_READ32(pAd,CAL_R1, &phaseCaliStatus);
		timeOutCount = 0;
		while (phaseCaliStatus & 0x80)
		{
			if (timeOutCount == 3) 
			{
				DBGPRINT(RT_DEBUG_TRACE, (
					"Residual Phase1 HW calibration doesn't finish\n"
					"phaseCaliStatus = %x\n", 
					phaseCaliStatus));
					
				break;
			}
		
			timeOutCount++;
		
			RtmpOsMsDelay(1); // waiting 1ms                                                                   
		
			RTMP_IO_READ32(pAd,CAL_R1, &phaseCaliStatus);
		}
				
		// 0x2C2C
		// Bit 23:16	Correlator Phase
		// Bit 15:8 	Correlator Q value
		// Bit 7:0		Correlator I value
		RTMP_IO_READ32(pAd,CAL_R11, &phaseCaliResult);
		mPhase[i] = (INT)((INT)((phaseCaliResult << 8) & 0xFF000000) >> 24);                                     
		avgQData = (INT)((INT)((phaseCaliResult << 16) & 0xFF000000) >> 24);
		avgIData = (INT)((INT)((phaseCaliResult << 24) & 0xFF000000) >> 24);
		mCalPhase0 = iAtan2(avgQData, avgIData);
		
		DBGPRINT(RT_DEBUG_TRACE, (
				"===============================Path%d==============================\n"                             
			    "Tx phase cal : CAL_R11=0x%x\n"                                                                           
			    "Phase=0x%x\n"                                                                                     
				"Peak I value=0x%x\n"
				"I value=0x%lx, Q value=0x%lx\n"
				"MidVGA[0] = 0x%x\n",
				rf_Path[i],
				phaseCaliResult, 
				//(360*mPhase[i])>>8,                                                                                  
				mPhase[i],                                                                                         
				phaseCaliResult >> 24,
				avgIData, avgQData,
				MidVGA[0]));
		
		RTMP_IO_WRITE32(pAd,CAL_R1, 0x00000006); 	// Disable Calibration		
		
		if (gBandFlg)
		{
			//mt_rf_write(pAd, rf_Path[i], RFDIGI_TRX17, 0x00000000);
			//mt_rf_write(pAd, rf_Path[i], RG_WF0_RXG_TOP, 0x02512016);
			//mt_rf_write(pAd, rf_Path[i], RFDIGI_TOP3, 0x11C73047);
			//mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP3, 0x11C73047);
			//mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP1, 0x00040120);
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP0, 0x00056733); // enable SX/RX/for WF1 
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP0, 0x00056730); // enable SX/RX/for WF1        
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP0, 0x80056730); // enable SX/RX/for WF1                         
			mt_rf_write(pAd, rf_Path[i], RFDIGI_TOP0, 0x00056F23); // enable SX/RX/for WF0,                        
			mt_rf_write(pAd, rf_Path[i], RFDIGI_TOP0, 0x00056F20); // enable SX/RX/for WF0,                        
			mt_rf_write(pAd, rf_Path[i], RFDIGI_TOP0, 0x80056F20); // enable SX/RX/for WF0,                        
			//mt_rf_write(pAd, rf_Path[i], RFDIGI_TOP1, 0x00040120);
			//mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TRX0, 0x0508000F);
			//mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TRX4, 0x00000C0C);
		}
		else
		{
			//mt_rf_write(pAd, rf_Path[i], RFDIGI_TRX17, 0x00000000);
			//mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP3, 0x11C73047);
			//mt_rf_write(pAd, rf_Path[i], RFDIGI_TOP3, 0x11C73047);
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP0, 0x00056737); // enable SX/RX/for WF1    
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP0, 0x00056734); // enable SX/RX/for WF1  
			mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP0, 0x80056734); // enable SX/RX/for WF1   
			//mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TOP1, 0x00040120);				
			mt_rf_write(pAd, rf_Path[i],    RFDIGI_TOP0, 0x00056F27); // enable SX/RX/for WF0,                        		                     
			mt_rf_write(pAd, rf_Path[i],    RFDIGI_TOP0, 0x00056F24); // enable SX/RX/for WF0,                                              
			mt_rf_write(pAd, rf_Path[i],    RFDIGI_TOP0, 0x80056F24); // enable SX/RX/for WF0,                        
			//mt_rf_write(pAd, rf_Path[i], RFDIGI_TOP1, 0x00040120);
			//mt_rf_write(pAd, rf_Path[i], RFDIGI_TRX0, 0x0508000F);
			//mt_rf_write(pAd, rf_RevPath[i], RFDIGI_TRX4, 0x00000C0C);
	    }
    }

    phaseValues[0] = mPhase[0] - mPhase[1];
		
    DBGPRINT(RT_DEBUG_TRACE, ("Tx phase : 0x%x\n", phaseValues[0]));

	/* Either display parameters, update EEPROM and BBP registers or dump capture data */
	switch (calFunction) {
	case 0:
		/* Remove Divider phase */
		ITxBFDividerCalibrationStartUp(pAd, 0, 0, divPhase);
		//phaseValues[0] -= divPhase[0];
		
		DBGPRINT(RT_DEBUG_TRACE, (
			"Divider phase  : 0x%x\n"
			"Before Residual phase : 0x%x\n", divPhase[0], phaseValues[0]));
		
		if (gBandFlg == TRUE)
		{
			ITxBFLoPhaseCalibrationStartUp(pAd, loOffsetCkFlg, &LoPhase);
			printk("Before LoPhase = 0x%x\n", LoPhase);
			LoPhase = ABS_One(LoPhase);
		
			if (LoPhase > 0x20 && LoPhase < 0x60)
			{
				phaseValues[0] = phaseValues[0] + 0x80;
		    }
				
			printk("After LoPhase = 0x%x\n", LoPhase);
		}
		
		DBGPRINT(RT_DEBUG_TRACE, (
			"After Residual phase : 0x%x\n", phaseValues[0]));
		
		break;

	case 1:
		RTMP_IO_WRITE32(pAd, CAL_R0, 0);
		RTMP_IO_WRITE32(pAd, TXBE_R13, phaseValues[0]);  // for method1
	
		DBGPRINT(RT_DEBUG_TRACE, ("Tx phase : 0x%x\n", phaseValues[0]));

		/* Remove Divider phase */
		ITxBFDividerCalibrationStartUp(pAd, 0, 0, divPhase);
		phaseValues[0] -= divPhase[0];

		DBGPRINT(RT_DEBUG_TRACE, (
			"Divider phase  : 0x%x\n"
			"Residual phase : 0x%x\n", divPhase[0], phaseValues[0]));
		
		if (gBandFlg == TRUE)
		{
			ITxBFLoPhaseCalibrationStartUp(pAd, loOffsetCkFlg, &LoPhase);
		}
		
		/* Save new reference values in EEPROM and BBP */
		ITxBFGetEEPROM(pAd, &phaseParams, 0, 0, 0);
				
		/* Only allow calibration on specific channels */
		if (ch == 1) 
			phaseParams.E1gBeg = phaseValues[0];
		else if (ch == 14)
			phaseParams.E1gEnd = phaseValues[0];
		else if (ch == 36)
			phaseParams.E1aLowBeg = phaseValues[0];
		else if (ch == 64) 
			phaseParams.E1aLowEnd = phaseValues[0];
		else if (ch == 100)
			phaseParams.E1aMidBeg = phaseValues[0];
		else if (ch == 120)
			phaseParams.E1aMidMid = phaseValues[0];
		else if (ch == 140)
			phaseParams.E1aMidEnd = phaseValues[0];
		else if (ch == 149)
			phaseParams.E1aHighBeg = phaseValues[0];
		else if (ch == 173)
			phaseParams.E1aHighEnd = phaseValues[0];
		else {
			DBGPRINT(RT_DEBUG_OFF,
					("Invalid channel: %d\nMust calibrate channel 1, 14, 36, 64, 100, 120, 140, 149 or 173", ch) );
			return FALSE;
		}

		ITxBFSetEEPROM(pAd, &phaseParams, 0, 0, 0);

	case 2:
		/* Remove Divider phase */
		ITxBFDividerCalibrationStartUp(pAd, 0, 0, divPhase);
		//phaseValues[0] -= divPhase[0];

		DBGPRINT(RT_DEBUG_TRACE, (
			"Divider phase  : 0x%x\n"
			"Before Residual phase : 0x%x\n", divPhase[0], phaseValues[0]));
		
		if (gBandFlg == TRUE)
		{
			ITxBFLoPhaseCalibrationStartUp(pAd, loOffsetCkFlg, &LoPhase);
			printk("Before LoPhase = 0x%x\n", LoPhase);
			LoPhase = ABS_One(LoPhase);
			
			if (LoPhase > 0x20 && LoPhase < 0x60)
			{
				phaseValues[0] = phaseValues[0] + 0x80;
			}

			printk("After LoPhase = 0x%x\n", LoPhase);
		}

		DBGPRINT(RT_DEBUG_TRACE, ("After Residual phase : 0x%x\n", phaseValues[0]));

		RTMP_IO_WRITE32(pAd, CAL_R0, 0);
		RTMP_IO_WRITE32(pAd, TXBE_R13, phaseValues[0]);  // for method1

		break;
	}

	return TRUE;
}
#endif


#ifdef MT76x2
#ifdef TXBF_SUPPORT
static SC_TABLE_ENTRY impSubCarrierTable[3] = { {228, 255, 1, 28}, {198, 254, 2, 58}, {134, 254, 2, 122} };
static SC_TABLE_ENTRY expSubCarrierTable[3] = { {228, 255, 1, 28}, {198, 254, 2, 58}, {134, 254, 2, 122} };	

INT TxBfProfileTagRead(
    IN PRTMP_ADAPTER     pAd,
	IN PFMU_PROFILE      *prof,
	IN UCHAR             profileIdx)
{
	UINT    value32, readValue32[5];
	
	// Read PFMU_R10 (0x2f28) first
	RTMP_IO_READ32(pAd, PFMU_R10, &value32);
	value32 &= (~0x3C00);
	// Wite PFMU_R10 to trigger read command
	RTMP_IO_WRITE32(pAd, PFMU_R10, ((profileIdx << 10)|value32));
	// Read PFMU_R19 ~ R23
	RTMP_IO_READ32(pAd, PFMU_R11, &readValue32[0]);
	RTMP_IO_READ32(pAd, PFMU_R12, &readValue32[1]);
	RTMP_IO_READ32(pAd, PFMU_R13, &readValue32[2]);
	RTMP_IO_READ32(pAd, PFMU_R14, &readValue32[3]);
	RTMP_IO_READ32(pAd, PFMU_R15, &readValue32[4]);

	/*
	    Bit 63:62       ng[1:0]
	        61:60       cw[1:0]
	        59:58       nrow[1:0]
	        57:56       ncol[1:0]
	        1 : 0       LM[1:0]
	*/
	prof->ng   = (UCHAR)((readValue32[1] >> 30) & 0x3);
	prof->cw   = (UCHAR)((readValue32[1] >> 28) & 0x3);
	prof->nrow = (UCHAR)((readValue32[1] >> 26) & 0x3);
	prof->ncol = (UCHAR)((readValue32[1] >> 24) & 0x3);
	prof->LM   = (UCHAR)(readValue32[0] & 0x3);

	/*
	    Bit 55 : 8       Mac Adress [47:0]
	*/
	prof->DMAC[0] = (UCHAR)((readValue32[1] >> 16) & 0x0FF);
	prof->DMAC[1] = (UCHAR)((readValue32[1] >> 8) & 0x0FF);
	prof->DMAC[2] = (UCHAR)((readValue32[1]) & 0x0FF);
	prof->DMAC[3] = (UCHAR)((readValue32[0] >> 24) & 0x0FF);
	prof->DMAC[4] = (UCHAR)((readValue32[0] >> 16) & 0x0FF);
	prof->DMAC[5] = (UCHAR)((readValue32[0] >> 8)  & 0x0FF);

	/*
	    Bit 63 : 56       Tx1_scale_2ss[7:0]
	        55 : 48       Tx0_scale_2ss[7:0]
	        47 : 40       Tx1_scale_1ss[7:0]
	        36 : 32       Tx0_scale_1ss[7:0]
	*/
	prof->Tx1_scale_2ss = (UCHAR)((readValue32[3] >> 24) & 0x0FF);
	prof->Tx0_scale_2ss = (UCHAR)((readValue32[3] >> 16) & 0x0FF);
	prof->Tx1_scale_1ss = (UCHAR)((readValue32[3] >> 8) & 0x0FF);
	prof->Tx0_scale_1ss = (UCHAR)((readValue32[3] >> 0) & 0x0FF);

	/*
	    Bit 31 : 24       STS1_SNR[7:0]
	        23 : 16       STS0_SNR[7:0]
	*/
	prof->STS1_SNR = (UCHAR)((readValue32[2] >> 24) & 0x0FF);
	prof->STS0_SNR = (UCHAR)((readValue32[2] >> 16) & 0x0FF);

	/*
	    Bit 15 : 0       PFMU_TimeOut[15:0]
	*/
	prof->timeout = (USHORT)(readValue32[2] & 0x0FFFF);

	/*
		Bit 8 : 5	    LD, EO, IO, I/E
	*/
	prof->LD = (UCHAR)((readValue32[4] >> 8) & 0x1);
	prof->EO = (UCHAR)((readValue32[4] >> 7) & 0x1);
	prof->IO = (UCHAR)((readValue32[4] >> 6) & 0x1);
	prof->I_E= (UCHAR)((readValue32[4] >> 5) & 0x1);
	
	/*
	    Check profile valid staus
	*/
	prof->validFlg = (readValue32[0] >> 7) & 0x1;

	DBGPRINT(RT_DEBUG_TRACE, ("============================= TxBf profile Tage Info ==============================\n"
		   "Pofile valid status = %d\n"
		   "Profile index = %d\n\n"
		   "PFMU_11 = 0x%x, PFMU_R12=0x%x, PFMU_R13=0x%x, PFMU_R14=0x%x\n\n"
		   "ng[1:0]=%d, cw[1:0]=%d, nrow[1:0]=%d, ncol[1:0]=%d, LM[1:0]=%d\n\n"
		   "Destimation MAC Address=%x:%x:%x:%x:%x:%x\n\n"
		   "Tx1_scal_2ss=0x%x, Tx0_scale_2ss=0x%x, Tx1_scal_1ss=0x%x, Tx0_scale_1ss=0x%x\n\n"
		   "STS1_SNR=0x%x, STS0_SNR=0x%x\n\n"
		   "timeout[15:0]=0x%x\n\n"
		   "LD=%d, EO=%d, IO=%d, I/E=%d\n"
		   "===================================================================================\n",
		   prof->validFlg,
		   profileIdx,
		   readValue32[0], readValue32[1], readValue32[2], readValue32[3],
		   prof->ng, prof->cw, prof->nrow, prof->ncol, prof->LM,
		   prof->DMAC[0], prof->DMAC[1], prof->DMAC[2], prof->DMAC[3], prof->DMAC[4], prof->DMAC[5],
		   prof->Tx1_scale_2ss, prof->Tx0_scale_2ss, prof->Tx1_scale_1ss, prof->Tx0_scale_1ss,
		   prof->STS1_SNR, prof->STS0_SNR,
		   prof->timeout,
		   prof->LD, prof->EO, prof->IO, prof->I_E));

	return TRUE;
}


INT TxBfProfileTagWrite(
    IN PRTMP_ADAPTER     pAd,
	IN PFMU_PROFILE      *prof,
	IN UCHAR             profileIdx)
{
	UINT    value32, readValue32[5];

	// Read PFMU_R10 (0x2f28) first
	RTMP_IO_READ32(pAd, PFMU_R10, &value32);
	value32 &= (~0x3C00);
	// Wite PFMU_R10 to trigger read command
	RTMP_IO_WRITE32(pAd, PFMU_R10, ((profileIdx << 10)|value32));
	// Read PFMU_R19 ~ R23
	RTMP_IO_READ32(pAd, PFMU_R11, &readValue32[0]);
	RTMP_IO_READ32(pAd, PFMU_R12, &readValue32[1]);
	RTMP_IO_READ32(pAd, PFMU_R13, &readValue32[2]);
	RTMP_IO_READ32(pAd, PFMU_R14, &readValue32[3]);
	RTMP_IO_READ32(pAd, PFMU_R15, &readValue32[4]);

	DBGPRINT(RT_DEBUG_TRACE, ("============================= TxBf profile Tage Write ==============================\n"
		   "Profile index = %d\n\n",
		   profileIdx));

	/*
	    Bit 63:62       ng[1:0]
	         61:60       cw[1:0]
	         59:58       nrow[1:0]
	         57:56       ncol[1:0]
	         1 :  0       LM[1:0]
	*/
	if (prof->CMDInIdx & 1)
	{
		value32  = (LONG)prof->ng << 30;
		value32 |= (LONG)prof->cw << 28;
		value32 |= (LONG)prof->nrow << 26;
		value32 |= (LONG)prof->ncol << 24;
		readValue32[1] &= 0x00FFFFFF;
		readValue32[1] |= value32;

		readValue32[0] &= ~(0x3);
		readValue32[0] |= prof->LM;

		printk("ng[1:0]=%d, cw[1:0]=%d, nrow[1:0]=%d, ncol[1:0]=%d, LM[1:0]=%d\n\n",
			   prof->ng, prof->cw, prof->nrow, prof->ncol, prof->ncol);
	}

	/*
	    Bit 55 : 8       Mac Adress [47:0]
	*/
	if (prof->CMDInIdx & 2)
	{
		value32  = (LONG)prof->DMAC[0] << 16;
		value32 |= (LONG)prof->DMAC[1] << 8;
		value32 |= (LONG)prof->DMAC[2] << 0;
		readValue32[1] &= 0xFF000000;
		readValue32[1] |= value32;

		value32 = (LONG)prof->DMAC[3] << 24;
		value32 |= (LONG)prof->DMAC[4] << 16;
		value32 |= (LONG)prof->DMAC[5] << 8;
		readValue32[0] &= 0x000000FF;
		readValue32[0] |= value32;
		
		DBGPRINT(RT_DEBUG_TRACE,("Destimation MAC Address=%x:%x:%x:%x:%x:%x\n\n",
			   prof->DMAC[0], prof->DMAC[1], prof->DMAC[2], prof->DMAC[3], prof->DMAC[4], prof->DMAC[5]));
	}

	/*
	    Bit 63 : 56       Tx1_scale_2ss[7:0]
	        55 : 48       Tx0_scale_2ss[7:0]
	        47 : 40       Tx1_scale_1ss[7:0]
	        36 : 32       Tx0_scale_1ss[7:0]
	*/	
	if (prof->CMDInIdx & 4)
	{
		value32  = (LONG)prof->Tx1_scale_2ss << 24;
		value32 |= (LONG)prof->Tx0_scale_2ss << 16;
		value32 |= (LONG)prof->Tx1_scale_1ss << 8;
		value32 |= (LONG)prof->Tx0_scale_1ss << 0;
		readValue32[3] = value32;

		printk("Tx1_scal_2ss=0x%x, Tx0_scale_2ss=0x%x, Tx1_scal_1ss=0x%x, Tx0_scale_1ss=0x%x\n\n",
			   prof->Tx1_scale_2ss, prof->Tx0_scale_2ss, prof->Tx1_scale_1ss, prof->Tx0_scale_1ss);
	}

	/*
	    Bit 31 : 24       STS1_SNR[7:0]
	        23 : 16       STS0_SNR[7:0]
	*/
	if (prof->CMDInIdx & 8)
	{
		value32  = (LONG)prof->STS1_SNR << 24;
		value32 |= (LONG)prof->STS0_SNR << 16;
		readValue32[2] &= 0x0000FFFF;
		readValue32[2] |= value32;
		
		printk("STS1_SNR=0x%x, STS0_SNR=0x%x\n\n", prof->STS1_SNR, prof->STS0_SNR);
	}

	/*
	    Bit 15 : 0       timeout[15:0]
	*/
	if (prof->CMDInIdx & 16)
	{
		readValue32[2] &= 0xFFFF0000;
		readValue32[2] |= (ULONG)prof->timeout;
		
		printk("timeout[15:0]=0x%x\n\n", prof->timeout);
	}

	DBGPRINT(RT_DEBUG_TRACE,("PFMU_11 = 0x%x, PFMU_R12=0x%x, PFMU_R13=0x%x, PFMU_R14=0x%x\n\n",
		   readValue32[0], readValue32[1], readValue32[2], readValue32[3]));

	/*
		Bit 8 : 5	    LD, EO, IO, I/E
	*/
	if (prof->CMDInIdx & 32)
	{
		readValue32[4] &= 0xFFFFFE00;
		readValue32[4] |= prof->LD << 8;
		readValue32[4] |= prof->EO << 7;
		readValue32[4] |= prof->IO << 6;
		readValue32[4] |= prof->I_E<< 5;
			
		printk("LD=%d, EO=%d, IO=%d, I/E=%d\n"
			   "===================================================================================\n",
			   prof->LD, prof->EO, prof->IO, prof->I_E);
	}


	// Read PFMU_R10 (0x2f28) first
	RTMP_IO_READ32(pAd, PFMU_R10, &value32);
	value32 &= (~0x3C00);
	// Wite PFMU_R10 to trigger read command
	RTMP_IO_WRITE32(pAd, PFMU_R10, ((profileIdx << 10)|value32));
	// Write PFMU_R11 ~ R15
	RTMP_IO_WRITE32(pAd, PFMU_R15, readValue32[4]);
	RTMP_IO_WRITE32(pAd, PFMU_R14, readValue32[3]);
	RTMP_IO_WRITE32(pAd, PFMU_R13, readValue32[2]);
	RTMP_IO_WRITE32(pAd, PFMU_R12, readValue32[1]);
	RTMP_IO_WRITE32(pAd, PFMU_R11, readValue32[0]);

	prof->CMDInIdx = 0; // clear tag indicator

	return TRUE;
}


INT TxBfProfileDataRead(
    IN PRTMP_ADAPTER     pAd,
	IN PFMU_DATA        *pData,
	IN UCHAR             profileIdx,
	IN UCHAR             subcarrierIdx)
{
	UINT    value32, readValue32[5];	

	// Read PFMU_R10 (0x2f28) first
	RTMP_IO_READ32(pAd, PFMU_R10, &value32);
	// Wite PFMU_R10 to trigger read command
	RTMP_IO_WRITE32(pAd, PFMU_R10, ((profileIdx << 10)|subcarrierIdx));
	// Read PFMU_R19 ~ R23
	RTMP_IO_READ32(pAd, PFMU_R19, &readValue32[0]);
	RTMP_IO_READ32(pAd, PFMU_R20, &readValue32[1]);
	RTMP_IO_READ32(pAd, PFMU_R21, &readValue32[2]);
	RTMP_IO_READ32(pAd, PFMU_R22, &readValue32[3]);
	RTMP_IO_READ32(pAd, PFMU_R23, &readValue32[4]);

	pData->psi21 = (readValue32[0] >> 16) & 0x00FF;
	pData->phill = (readValue32[0] >> 0)  & 0x00FF;
	return TRUE;
}


INT TxBfProfileDataWrite(
    IN PRTMP_ADAPTER     pAd,
	IN PFMU_DATA         *pData,
	IN UCHAR             profileIdx,
	IN UCHAR             subcarrierIdx)
{
	UINT    value32, readValue32[5];

	// Read PFMU_R10 (0x2f28) first
	RTMP_IO_READ32(pAd, PFMU_R10, &value32);
	// Wite PFMU_R10 to trigger read command
	RTMP_IO_WRITE32(pAd, PFMU_R10, ((profileIdx << 10)|subcarrierIdx));
	// Read PFMU_R19 ~ R23
	RTMP_IO_READ32(pAd, PFMU_R19, &readValue32[0]);
	RTMP_IO_READ32(pAd, PFMU_R20, &readValue32[1]);
	RTMP_IO_READ32(pAd, PFMU_R21, &readValue32[2]);
	RTMP_IO_READ32(pAd, PFMU_R22, &readValue32[3]);
	RTMP_IO_READ32(pAd, PFMU_R23, &readValue32[4]);

	pData->psi21 = (readValue32[0] >> 16) & 0x00FF;
	pData->phill = (readValue32[0] >> 0)  & 0x00FF;

	printk("============================= TxBf profile Data Write ==============================\n"
		   "Profile index = %d,    subcarrierIdx = %d\n\n",
		   profileIdx, subcarrierIdx);

	if (pData->dCMDInIdx & 1)
	{
		readValue32[0] &= 0xFF00FFFF;
		readValue32[0] |= pData->psi21;
		
		printk("psi21 = 0x%x\n\n", pData->psi21);
	}		

	if (pData->dCMDInIdx & 2)
	{
		readValue32[0] &= 0xFFFFFF00;
		readValue32[0] |= pData->phill;

		printk("phill = 0x%x\n\n", pData->phill);
	}
	
	printk("PFMU_19 = 0x%x, PFMU_R20=0x%x, PFMU_R21=0x%x, PFMU_R22=0x%x\n\n"
		   "===================================================================================\n",
		   readValue32[0], readValue32[1], readValue32[2], readValue32[3]);

	// Read PFMU_R10 (0x2f28) first
	RTMP_IO_READ32(pAd, PFMU_R10, &value32);
	// Wite PFMU_R10 to trigger read command
	RTMP_IO_WRITE32(pAd, PFMU_R10, ((profileIdx << 10)|subcarrierIdx));
	// Wite PFMU_R19 ~ R23
	RTMP_IO_WRITE32(pAd, PFMU_R23, readValue32[4]);
	RTMP_IO_WRITE32(pAd, PFMU_R22, readValue32[3]);
	RTMP_IO_WRITE32(pAd, PFMU_R21, readValue32[2]);
	RTMP_IO_WRITE32(pAd, PFMU_R20, readValue32[1]);
	RTMP_IO_WRITE32(pAd, PFMU_R19, readValue32[0]);

	pData->dCMDInIdx = 0; // clear profile data write indicator
	
	return TRUE;
}


INT TxBfProfileTagValid(
    IN PRTMP_ADAPTER     pAd,
	IN PFMU_PROFILE      *prof,
	IN UCHAR             profileIdx)
{
	UINT    value32, readValue32[5];

	// Read PFMU_R10 (0x2f28) first
	RTMP_IO_READ32(pAd, PFMU_R10, &value32);
	value32 &= (~0x3C00);
	// Wite PFMU_R10 to trigger read command
	RTMP_IO_WRITE32(pAd, PFMU_R10, ((profileIdx << 10)|value32));
	// Read PFMU_R11 ~ R15
	RTMP_IO_READ32(pAd, PFMU_R11, &readValue32[0]);
	RTMP_IO_READ32(pAd, PFMU_R12, &readValue32[1]);
	RTMP_IO_READ32(pAd, PFMU_R13, &readValue32[2]);
	RTMP_IO_READ32(pAd, PFMU_R14, &readValue32[3]);
	RTMP_IO_READ32(pAd, PFMU_R15, &readValue32[4]);

	/*
	    Update the valid bit
	*/
	readValue32[0] &= ~(1 << 7);
	readValue32[0] |= prof->validFlg << 7;

	// Read PFMU_R10 (0x2f28) first
	RTMP_IO_READ32(pAd, PFMU_R10, &value32);
	value32 &= (~0x3C00);
	// Wite PFMU_R10 to trigger read command
	RTMP_IO_WRITE32(pAd, PFMU_R10, ((profileIdx << 10)|value32));
	// Write PFMU_R11 ~ R15
	RTMP_IO_WRITE32(pAd, PFMU_R15, readValue32[4]);
	RTMP_IO_WRITE32(pAd, PFMU_R14, readValue32[3]);
	RTMP_IO_WRITE32(pAd, PFMU_R13, readValue32[2]);
	RTMP_IO_WRITE32(pAd, PFMU_R12, readValue32[1]);
	RTMP_IO_WRITE32(pAd, PFMU_R11, readValue32[0]);
	
	return TRUE;
}
#endif /* TXBF_SUPPORT */
#endif /* MT76x2 */


#ifdef MT76x2
UCHAR Read_PFMUTxBfProfile(
	IN	PRTMP_ADAPTER	    pAd, 
	IN	PFMU_PROFILE	   *prof,
	IN  PFMU_DATA          *pData,
	IN	BOOLEAN			    implicitProfile)
{
	INT carrierIndex, scIndex;
	INT	profileNum;
	SC_TABLE_ENTRY *pTab = NULL;
	INT j, c;
	UCHAR  r163Value = 0;
	UINT32 value32;
	UCHAR  GrpTab[3] = {1, 2, 4};
	UCHAR  GrpInc;

#ifdef TIMESTAMP_BF_PROFILE
	struct timeval tval1, tval2;
	do_gettimeofday(&tval1);
#endif

	/* Disable Profile Updates during access */
	RTMP_IO_READ32(pAd,  PFMU_R1, &value32);
	RTMP_IO_WRITE32(pAd, PFMU_R1,  value32 & ~0x330);

	for (profileNum = 0; profileNum < 8; profileNum++)
	{
		/* Read Tag */
		TxBfProfileTagRead(pAd, prof, profileNum);

		if (implicitProfile == 1 && prof->I_E == 0 && prof->validFlg == 1)
		{
			DBGPRINT(RT_DEBUG_TRACE,("@@@@@@@@ Valid ITxBf profile is found @@@@@@@@@\n"
				   "implicitProfile = %d   prof->cw = %d \n",
				   implicitProfile, prof->cw));
			pTab = &impSubCarrierTable[prof->cw];
			break; // While valid implicit profile is found, break the seeking loop
		}
		
		if (implicitProfile == 0 && prof->I_E == 1 && prof->validFlg == 1)
		{
			DBGPRINT(RT_DEBUG_TRACE,("@@@@@@@@ Valid ETxBf profile is found @@@@@@@@@\n"
					"implicitProfile = %d   prof->cw = %d \n",
				   	implicitProfile, prof->cw));
			pTab = &expSubCarrierTable[prof->cw];
			break; // While valid explicit profile is found, break the seeking loop
		}	
	}	

	if (prof->validFlg != 1) return prof->validFlg;
		
	/* Negative subcarriers */
	carrierIndex = 0;
	GrpInc = GrpTab[prof->ng];
	for (scIndex=pTab->lwb1; scIndex <= pTab->upb1; scIndex++)
	{
		/* Read subcarrier data */
		TxBfProfileDataRead(pAd, pData, profileNum, scIndex);
		pData->data[carrierIndex++][0] = pData->psi21;
		pData->data[carrierIndex][1] = pData->phill;
	}	

	/* Positive subcarriers */
	for (scIndex=pTab->lwb2; scIndex <= pTab->upb2; scIndex++)
	{
		/* Read subcarrier data */
		TxBfProfileDataRead(pAd, pData, profileNum, scIndex);
		pData->data[carrierIndex++][0] = pData->psi21;
		pData->data[carrierIndex][1] = pData->phill;
	}

	/* Restore Profile Updates */
	RTMP_IO_WRITE32(pAd,  PFMU_R1, value32);

#ifdef TIMESTAMP_BF_PROFILE
	do_gettimeofday(&tval2);
	DBGPRINT(RT_DEBUG_WARN, ("BF Read elasped = %ld usec\n", tval2.tv_usec - tval1.tv_usec));
#endif

	return prof->validFlg;
}
#endif // MT76x2


#ifdef DBG
#ifdef LINUX
/* #define TIMESTAMP_CALC_CALIBRATION */
#endif /* LINUX */
#endif /* DBG */

#define P_RESOLUTION	256		/* Resolution of phase calculation: 2pi/256 */

static UCHAR maxCarrTab[3] = {PROFILE_MAX_CARRIERS_20,
		                      PROFILE_MAX_CARRIERS_40,
		                 	  PROFILE_MAX_CARRIERS_80};

INT32 ei0[PROFILE_MAX_CARRIERS_40][2];
INT32 ei1[PROFILE_MAX_CARRIERS_40][2];
INT32 ei2[PROFILE_MAX_CARRIERS_40][2];

/*
	iCalcCalibration - calculate calibration parameters
		Returns 0 if successful, -1 if profiles are invalid
*/

#ifdef MT76x2
int iCalcCalibration(PRTMP_ADAPTER pAd, int calParams[2], int profileNum)
{
	int	  pi, maxCarriers, ii;
	short rot[2];
	INT32 c0[2], c1[2];
	INT32 minSum=0;
	int	  di1=0;
	INT32 ed[2], id[2];
	PFMU_PROFILE *pExpProf, *pImpProf;
    PFMU_DATA    *pExpData, *pImpData;
	SHORT        cos_psi21_e, sin_psi21_e, cos_phill_e, sin_phill_e;
	SHORT        cos_psi21_i, sin_psi21_i, cos_phill_i, sin_phill_i;
	int result = 0;
	
#ifdef TIMESTAMP_CALC_CALIBRATION
	struct timeval tval1, tval2;
#endif

	if (os_alloc_mem(pAd, (UCHAR **)&pExpData, sizeof(PFMU_DATA)) != NDIS_STATUS_SUCCESS)
	{
		return -3;
	}

	if (os_alloc_mem(pAd, (UCHAR **)&pImpData, sizeof(PFMU_DATA))!= NDIS_STATUS_SUCCESS)
	{
		os_free_mem(pAd, pExpData);
		return -3;
	}

	if (os_alloc_mem(pAd, (UCHAR **)&pExpProf, sizeof(PFMU_PROFILE))!= NDIS_STATUS_SUCCESS)
	{
		os_free_mem(pAd, pImpData);
		os_free_mem(pAd, pExpData);
		return -3;
	}
	
	if (os_alloc_mem(pAd, (UCHAR **)&pImpProf, sizeof(PFMU_PROFILE)) != NDIS_STATUS_SUCCESS)
	{
		os_free_mem(pAd, pImpData);
		os_free_mem(pAd, pExpData);
		os_free_mem(pAd, pExpProf);
		return -3;
	}

	/* Read Implicit and Explicit data */
	//if (Read_PFMUTxBfProfile(pAd, pImpProf, pImpData, TRUE) != 1)
	if (Read_PFMUTxBfProfile(pAd, pImpProf, pImpData, TRUE) != 1)
	{
		result = -1;
		goto exitCalcCal;
	}

	maxCarriers = maxCarrTab[pImpProf->cw];
	for (pi=0; pi<maxCarriers; pi++) {
		DBGPRINT(RT_DEBUG_TRACE,("pImpData->data[%d][0]=0x%x                     pImpData->data[%d][1]=0x%x\n",
			    pi, pImpData->data[pi][0], pi, pImpData->data[pi][1]));
	}
    if (Read_PFMUTxBfProfile(pAd, pExpProf, pExpData, FALSE) != 1)
    {
    	result = -1;
		goto exitCalcCal;
    }

	maxCarriers = maxCarrTab[pExpProf->cw];
	for (pi=0; pi<maxCarriers; pi++) {
		DBGPRINT(RT_DEBUG_TRACE,("pExpData->data[%d][0]=0x%x                     pExpData->data[%d][1]=0x%x\n",
			    pi, pExpData->data[pi][0], pi, pExpData->data[pi][1]));
	}

	/* Quit if MAC addresses don't match */
	for (ii=0; ii<6; ii++) {
		if (pImpProf->DMAC[ii] != pExpProf->DMAC[ii]) {
			result = -2;
			printk("MAC Addr in ITxBf and ETxBf tag is different!!!!!!!\n");
			goto exitCalcCal;
		}
	}

	/* Quit if profiles cannot be used */
	if (pImpProf->nrow > 2 || pExpProf->nrow > 2) {
		result = -1;
		printk("Profile Matrix size is not correct!!!!!!\n");
		goto exitCalcCal;
	}

	/* 
		If Implicit profile is legacy then zero out the unused carriers so they don't
		affect the calculation
	*/
	if (pImpProf->LM == 0) {
		memset(pImpData->data[0], 0x00, sizeof(pImpData->data[0]));
		memset(pImpData->data[1], 0x00, sizeof(pImpData->data[0]));
		memset(pImpData->data[PROFILE_MAX_CARRIERS_20-2], 0x00, sizeof(pImpData->data[0]));
		memset(pImpData->data[PROFILE_MAX_CARRIERS_20-1], 0x00, sizeof(pImpData->data[0]));
	}

#ifdef TIMESTAMP_CALC_CALIBRATION
	do_gettimeofday(&tval1);
#endif

    //maxCarriers = maxCarrTab[pImpProf->cw];
    maxCarriers = maxCarrTab[pExpProf->cw];

	/* Compute Exp .* conj(Imp).*/
	// Combine 2Tx signal to Rx 1st input so just need 2x1 matrix to calculate phase difference
	// ITBf vector Vi = |cos(psi21)exp(jphill)|        vector Ve = |cos(psi21)exp(jphill)|
	//                        |       sin(psi21)       |                          |       sin(psi21)       |

	for (pi=0; pi<maxCarriers; pi++) {
	//for (pi=3; pi<maxCarriers; pi++) {	
		icexp(rot, pExpData->data[pi][0] * 256/P_RESOLUTION);
		cos_psi21_e = rot[0];
		sin_psi21_e = rot[1];
		icexp(rot, pExpData->data[pi][1] * 256/P_RESOLUTION);
		cos_phill_e = rot[0];
		sin_phill_e = rot[1];
		
		// 1.20
		ed[0] = cos_psi21_e * cos_phill_e; // real part
		ed[1] = cos_psi21_e * sin_phill_e; // image part

		icexp(rot, pImpData->data[pi][0] * 256/P_RESOLUTION);
		cos_psi21_i = rot[0];
		sin_psi21_i = rot[1];
		icexp(rot, pImpData->data[pi][1] * 256/P_RESOLUTION);
		cos_phill_i = rot[0];
		sin_phill_i = rot[1];
		
		//1.09
		id[0] = (cos_psi21_i * cos_phill_i) >> 11; // real part
		id[1] = (cos_psi21_i * sin_phill_i) >> 11; // image part

		// ei0[pi] : 2.29
		icMult(ei0[pi], ed, id[0], -id[1]);

		ed[0] = -sin_psi21_e;
		ed[1] = 0;
		id[0] = -sin_psi21_i;
		id[1] = 0;
		// ei1[pi] : 1.20
		icMult(ei1[pi], ed, id[0], -id[1]);	

	}

	/* Search for best Phase 1 */
	// |ei0|
	// |ei1| = Vi .* Ve
	// ei0 + ei1 * exp(Phase[ii]) = exp(ei0) (1 + exp(Phase(ei1) - Phase(ei0)) * exp(Phase[ii]))
	// Max value should be Phase[ii] = Phase[ei0] - Phase[ei1]
	// By the check every phase index and find the min power (if sum -= isqrt(xx)), then you can find Phase[ii]
	for (ii=0; ii<P_RESOLUTION; ii++) {
		INT32 ei[2], sum = 0;

		icexp(rot, ii*256/P_RESOLUTION);

		for (pi=0; pi<maxCarriers; pi++)
		//for (pi=3; pi<maxCarriers; pi++)
		{
            // c1 : 1.(20-1) + 1.10 = 2.29
            ei[0] = ei1[pi][0] >> 1;
			ei[1] = ei1[pi][1] >> 1;
			icMult(c1, ei, rot[0], rot[1]);
			
			// c0 : 2.(29-17) , 2.(29-17) = 3.12 
			c0[0] = (ei0[pi][0] + c1[0])>>17;
			c0[1] = (ei0[pi][1] + c1[1])>>17;
			//sum : 3.12 + 3.12 = 6.24
			sum -= isqrt(c0[0]*c0[0] + c0[1]*c0[1]);
		}

		/* Record minimum */
		if (ii==0 || minSum>sum) {
			di1 = ii;
			minSum = sum;
		}
	}

	/* Convert to calibration parameters */
	calParams[0] = -di1 & 0xFF;  // Max power whe Phase[ii]= Phase[ei0] - Phase[ei1]
	calParams[1] = -di1 & 0xFF; // Phase[ii]= Phase[ei2] - Phase[ei1]
	// So if we compensate the phase at ANT1, then Phase[ei1] + (Phase[ei0] -Phase[ei1]) = Phase[ei0]
	//                                                 ANT2, then Phase[ei2] + (Phase[ei0] - Phase[ei2]) = Phase[ei0]
	//                                                 ANT0, no phase compensate
	// Hi = He * exp(Phase[ei0]) -> hi = he(t - delay)


#ifdef TIMESTAMP_CALC_CALIBRATION
	do_gettimeofday(&tval2);
    ATEDBGPRINT(RT_DEBUG_WARN, ("iCalcCal = %ld usec\n", tval2.tv_usec - tval1.tv_usec));
#endif

exitCalcCal:
	os_free_mem(pAd, pExpData);
	os_free_mem(pAd, pImpData);
	os_free_mem(pAd, pExpProf);
	os_free_mem(pAd, pImpProf);


	return result;
}

#endif

