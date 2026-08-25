#ifndef _MT7570_H_
#define _MT7570_H_
#include <flash_layout/tc_partition.h> 

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>

#ifdef TCSUPPORT_MT7570


static inline int bob_check(int imagefd)
{
    char bob[BOB_RA_SIZE] = {0};
    int read_data  = 0;
	int temp = 0;
    if (read(imagefd, bob, BOB_RA_SIZE) < BOB_RA_SIZE) {
        fprintf(stderr, "read error imagefd=%d\n",imagefd);
        return 0;
    }

	// Get magic number
	memcpy(&read_data,bob+0x094,sizeof(read_data));
    
    fprintf(stderr, "read bob magic code is 0x%08x\n",read_data);
	//If magic number is exist , return 1
	if( (read_data == 0x07050700) || (read_data == 0xe7050700) || (read_data == 0x07050701) || (read_data == 0xe7050701) || (read_data == 0xa7050701) )
	{
		temp = 1 ;
	}
	else{
		temp = 0 ;
    }

    lseek(imagefd,0,SEEK_SET);
	return 1;
}


#endif

#endif


