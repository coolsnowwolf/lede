#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) 
{
	FILE *rfp, *wfp ;
	char tmp[33][200] ;
	char *delim = " \t\n" ;
	char *p ;
	int line=0, flags=0, i ;
	
	if(argc != 3) {
		printf("Failed\n") ;
		return 0 ;
	}
	
	rfp = fopen(argv[1], "r") ;
	if(rfp == NULL) {
		printf("Open File %s Failed\n", argv[1]) ;
		return 0 ;
	}
	
	wfp = fopen(argv[2], "w+") ;
	if(wfp == NULL) {
		printf("Open File %s Failed\n", argv[2]) ;
		fclose(rfp) ;
		return 0 ;
	}

	while(fgets(tmp[line], 150, rfp)) {
		switch(flags) {
			case 0:
				if(strstr(tmp[line], "typedef PACKING union") != NULL)
					flags = 1 ;
				else
					flags = 0 ;	
				break ;
			
			case 1:
				if(strstr(tmp[line], "{") != NULL)
					flags = 2 ;
				else
					flags = 0 ;	
				break ;

			case 2:
				if(strstr(tmp[line], "PACKING struct") != NULL)
					flags = 3 ;
				else
					flags = 0 ;	
				break ;

			case 3:
				if(strstr(tmp[line], "{") != NULL)
					flags = 4 ;
				else
					flags = 0 ;	
				break ;

			case 4:
				if(strstr(tmp[line], ": 32;") != NULL) {
					line = 0 ;
					flags = 0 ;
				} else if(strstr(tmp[line], "} Bits;") != NULL) {
					fputs("#ifdef __BIG_ENDIAN\n", wfp) ;
					
					for(i=line-1 ; i>=0 ; i--) {
						fputs(tmp[i], wfp) ;
					}
					
					fputs("#else\n", wfp) ;
					
					for(i=0 ; i<line ; i++) {
						fputs(tmp[i], wfp) ;
					}
					
					fputs("#endif /* __BIG_ENDIAN */\n", wfp) ;
					fputs(tmp[line], wfp) ;
					
					line = 0 ;
					flags = 0 ;
					continue ;
				} else {
					line++ ;
					continue ;
				}
				break ;
		}
		
		fputs(tmp[line], wfp) ;
	}
	
	printf("Successful\n") ;
	
ret:
	fclose(rfp) ;
	fclose(wfp) ;
	return 0 ;
}
