#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <erics_tools.h>
#define malloc safe_malloc
#define strdup safe_strdup

void free_split_pieces(char** split_pieces);

int main(int argc, char **argv)
{
	char *table = argv[1];
	char *delete_chain = argv[2];
	if(argc != 3)
	{
		printf("USAGE: %s [TABLE] [CHAIN TO DELETE]\n\n", argv[0]);
		return 0;
	}

	char *command = dynamic_strcat(3, "iptables -t ", table, " -L -n --line-numbers 2>/dev/null");
	unsigned long num_lines = 0;
	char** table_dump = get_shell_command_output_lines(command, &num_lines);
	free(command);
	

	unsigned long line_index;
	char* current_chain = NULL;
	list* delete_commands = initialize_list();


	for(line_index=0; line_index < num_lines; line_index++)
	{
		char* line = table_dump[line_index];
		unsigned long num_pieces = 0;
		char whitespace[] = { '\t', ' ', '\r', '\n' };
		char** line_pieces = split_on_separators(line, whitespace, 4, -1, 0, &num_pieces);
		if(strcmp(line_pieces[0], "Chain") == 0)
		{
			if(current_chain != NULL) { free(current_chain); }
			current_chain = strdup(line_pieces[1]);
		}
		else 
		{
			unsigned long line_num;
			int read = sscanf(line_pieces[0], "%ld", &line_num);

			if(read > 0 && current_chain != NULL && num_pieces >1)
			{
				if(strcmp(line_pieces[1], delete_chain) == 0)
				{
					char* delete_command = dynamic_strcat(7, "iptables -t ", table, " -D ", current_chain, " ", line_pieces[0], " 2>/dev/null");
					push_list(delete_commands, delete_command);
				}
			}
		}

		//free line_pieces
		free_null_terminated_string_array(line_pieces);
	}
	free_null_terminated_string_array(table_dump);
	
	/* final two commands to flush chain being deleted and whack it */
	unshift_list(delete_commands, dynamic_strcat(5, "iptables -t ", table, " -F ", delete_chain, " 2>/dev/null"));
	unshift_list(delete_commands, dynamic_strcat(5, "iptables -t ", table, " -X ", delete_chain, " 2>/dev/null"));

	/* run delete commands */
	while(delete_commands->length > 0)
	{
		char *next_command = (char*)pop_list(delete_commands);
		char **out = get_shell_command_output_lines(next_command, &num_lines);
		free_null_terminated_string_array(out);
	}

	return 0;
}

