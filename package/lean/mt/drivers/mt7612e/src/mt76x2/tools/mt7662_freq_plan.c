#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct _CH_FREQ_MAP_{
	unsigned short channel;
	unsigned short freqKHz;
} CH_FREQ_MAP;

#define FREQ_REF 40
#define FREQ_REF_CYCLES 89
#define DOUBLER 1

CH_FREQ_MAP CH_HZ_ID_MAP[]=
		{
			{1, 2412},
			{2, 2417},
			{3, 2422},
			{4, 2427},
			{5, 2432},
			{6, 2437},
			{7, 2442},
			{8, 2447},
			{9, 2452},
			{10, 2457},
			{11, 2462},
			{12, 2467},
			{13, 2472},
			{14, 2484},

			/*  UNII */
			{36, 5180},
			{40, 5200},
			{44, 5220},
			{48, 5240},
			{52, 5260},
			{56, 5280},
			{60, 5300},
			{64, 5320},
			{149, 5745},
			{153, 5765},
			{157, 5785},
			{161, 5805},
			{165, 5825},
			{167, 5835},
			{169, 5845},
			{171, 5855},
			{173, 5865},
						
			/* HiperLAN2 */
			{100, 5500},
			{104, 5520},
			{108, 5540},
			{112, 5560},
			{116, 5580},
			{120, 5600},
			{124, 5620},
			{128, 5640},
			{132, 5660},
			{136, 5680},
			{140, 5700},
						
			/* Japan MMAC */
			{34, 5170},
			{38, 5190},
			{42, 5210},
			{46, 5230},
					
			/*  Japan */
			{183, 4915},
			{184, 4920},
			{185, 4925},
			{187, 4935},
			{188, 4940},
			{189, 4945},
			{192, 4960},
			{196, 4980},
			{200, 5000},
			{204, 5020},
			{208, 5040},	/* Japan, means J08 */
			{212, 5060},	/* Japan, means J12 */   
			{216, 5080},	/* Japan, means J16 */
};

int CH_HZ_ID_MAP_NUM = (sizeof(CH_HZ_ID_MAP)/sizeof(CH_FREQ_MAP));

void MapChannelID2KHZ(
	unsigned char Ch,
	double *pFreq)
{
	int chIdx;
	for (chIdx = 0; chIdx < CH_HZ_ID_MAP_NUM; chIdx++)
	{
		if ((Ch) == CH_HZ_ID_MAP[chIdx].channel)
		{
			(*pFreq) = CH_HZ_ID_MAP[chIdx].freqKHz * 1000;
			break;
		}
	}
	if (chIdx == CH_HZ_ID_MAP_NUM)
		(*pFreq) = 2412000;
}

int main(int argc, char *argv[])
{
	unsigned long fcal_target, integer, fraction;
	double fvco, n, freq_ref_cycles, freq_ref, fch = 0;
	int i;

	freq_ref_cycles = FREQ_REF_CYCLES;
	freq_ref = FREQ_REF;

	for (i = 0; i < CH_HZ_ID_MAP_NUM; i++) {
		/* FCAL target */
		MapChannelID2KHZ(CH_HZ_ID_MAP[i].channel, &fch);

		if (CH_HZ_ID_MAP[i].channel <= 14) { 
			fvco = (fch / 1000) * 4 / 3;
		} else
			fvco = (fch / 1000) * 2 / 3;

		//printf("%f\n", fvco);
		fcal_target = (unsigned long)(((freq_ref_cycles / freq_ref * fvco) + 1) + 0.5);
	
		/* SDM integer & fraction */
		n = fvco / freq_ref;

		//printf("%f\n", n);

		integer = (unsigned long)(n);
		fraction = (unsigned long)((n - integer) * (1 << 20));

		printf("\t{%d, %ld, %ld, %ld},\n", CH_HZ_ID_MAP[i].channel, fcal_target, integer, fraction); 
	}

	return 0;
}
