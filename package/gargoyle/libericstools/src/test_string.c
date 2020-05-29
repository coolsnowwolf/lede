#include "erics_tools.h"

int main(void)
{
	FILE* f = fopen("tmp", "r");	
	char terminators[] = "\n\r";


	unsigned long length;
	char* file_data = (char*)read_entire_file(f, 100, &length);
	printf("%s\n", file_data);
	fclose(f);

	f = fopen("tmp", "r");	
	dyn_read_t next;
	next.terminator = '\n';
	while(next.terminator != EOF)
	{
       		next =  dynamic_read(f, terminators, 2, &length);
		printf("read \"%s\"\n", next.str);
		free(next.str);
	}
	fclose(f);

	return 0;
}
