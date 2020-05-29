#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <erics_tools.h>

/* possible reset intervals */
#define BANDWIDTH_MINUTE		  80
#define BANDWIDTH_HOUR			  81
#define BANDWIDTH_DAY			  82
#define BANDWIDTH_WEEK			  83
#define BANDWIDTH_MONTH			  84
#define BANDWIDTH_NEVER			  85

int main(int argc, char** argv)
{
	if(argc < 3)
	{
		return 1;
	}
	char* in_file = argv[1];
	char* out_file = argv[2];

	uint64_t interval = 0;
	unsigned char is_const;
	if(strstr(in_file, "-15m") != NULL)
	{
		interval=2;
		is_const=1;
	}
	else if(strstr(in_file, "-15h") != NULL)
	{
		interval=60*2;
		is_const=1;
	}
	else if(strstr(in_file, "-15d") != NULL)
	{
		interval= BANDWIDTH_HOUR;
		is_const=0;
	}
	else if(strstr(in_file, "-1y") != NULL)
	{
		interval = BANDWIDTH_DAY;
		is_const=0;
	}

	if(interval > 0)
	{
		FILE* input = fopen(in_file, "rb");
		FILE* output = fopen(out_file, "wb");
		if(input == NULL || output == NULL)
		{
			/* We leak a possible FILE struct for input if
			 * opening in_file suceeded, but out_file didn't
			 * because we are terminating the program and Linux
			 * recovers from this. */
			fprintf(stderr, "ERROR: could not open file(s) for reading/writing\n");
			return 1;
		}

		time_t last_backup;
		uint64_t accumulator;
		uint64_t oldest_start;
		uint64_t oldest_end;
		uint64_t recent_end;
		unsigned char bw_bits;
		list* node_list = initialize_list();

		fread(&last_backup, sizeof(time_t), 1, input);
		fread(&accumulator, sizeof(int64_t), 1, input);
		fread(&oldest_start, sizeof(time_t), 1, input);
		fread(&oldest_end, sizeof(time_t), 1, input);
		fread(&recent_end, sizeof(time_t), 1, input);
		fread(&bw_bits, sizeof(unsigned char), 1, input);

		int64_t bandwidth_64 = 0;
		int32_t bandwidth_32 = 0;
		void* bw_pointer= bw_bits == 32 ? (void*)&bandwidth_32 : (void*)&bandwidth_64;
		int read_size = bw_bits == 32 ? sizeof(int32_t) : sizeof(int64_t);
		while( fread(bw_pointer, read_size, 1, input) > 0)
		{
			uint64_t *next_bw = (uint64_t*)malloc(sizeof(uint64_t));
			if(bw_bits == 64)
			{
				if(bandwidth_64 < 0) { bandwidth_64 = 0; }
				*next_bw = *((uint64_t*)bw_pointer);
			}
			else
			{
				if(bandwidth_32 < 0) { bandwidth_32 = 0; }
				*next_bw =  (uint64_t)  *((uint32_t*)bw_pointer);
			}
			push_list(node_list, next_bw);
		}
		fclose(input);

		uint64_t *last_bw = (uint64_t*)malloc(sizeof(uint64_t));
		*last_bw = accumulator;
		push_list(node_list, last_bw);


		uint32_t num_ips = 1;
		uint32_t ip = 0;
		uint64_t time = 0;
		uint32_t num_nodes = node_list->length;
		fwrite(&num_ips, 4, 1, output);
		fwrite(&interval, 8, 1, output);
		fwrite(&time, 8, 1, output);
		fwrite(&is_const, 1, 1, output);
		fwrite(&ip, 4, 1, output);
		fwrite(&num_nodes, 4, 1, output);
		fwrite(&oldest_start, 8, 1, output);
		fwrite(&oldest_end, 8, 1, output);
		fwrite(&recent_end, 8, 1, output);
		fwrite(&bw_bits, 1, 1 , output);
		while(node_list->length > 0)
		{
			uint64_t *next_bw = (uint64_t*)shift_list(node_list);
			if(bw_bits == 32)
			{
				uint32_t small_bw = (uint32_t) (*next_bw);
				fwrite(&small_bw, 4, 1, output);
			}
			else
			{
				fwrite(next_bw, 8, 1, output);
			}
			free(next_bw);
		}
		fclose(output);

		unsigned long num_destroyed;
		destroy_list(node_list, DESTROY_MODE_FREE_VALUES, &num_destroyed);
	}


	return 0;
}
