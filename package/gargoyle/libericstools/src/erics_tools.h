/*
 * Copyright © 2008 by Eric Bishop <eric@gargoyle-router.com>
 * 
 * This work 'as-is' we provide.
 * No warranty, express or implied.
 * We've done our best,
 * to debug and test.
 * Liability for damages denied.
 *
 * Permission is granted hereby,
 * to copy, share, and modify.
 * Use as is fit,
 * free or for profit.
 * On this notice these rights rely.
 *
 *
 *
 *  Note that unlike other portions of Gargoyle this code
 *  does not fall under the GPL, but the rather whimsical
 *  'Poetic License' above.
 *
 *  Basically, this library contains a bunch of utilities
 *  that I find useful.  I'm sure other libraries exist
 *  that are just as good or better, but I like these tools 
 *  because I personally wrote them, so I know their quirks.
 *  (i.e. I know where the bodies are buried).  I want to 
 *  make sure that I can re-use these utilities for whatever
 *  code I may want to write in the future be it
 *  proprietary or open-source, so I've put them under
 *  a very, very permissive license.
 *
 *  If you find this code useful, use it.  If not, don't.
 *  I really don't care.
 *
 */

#ifndef ERICS_TOOLS_H
#define ERICS_TOOLS_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include <regex.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#ifndef stricmp
	#define stricmp strcasecmp
#endif



/* tree_map structs / prototypes */
typedef struct long_tree_map_node
{
	unsigned long key;
	void* value;
	
	signed char balance; 
	struct long_tree_map_node* left;
	struct long_tree_map_node* right;
} long_map_node;

typedef struct 
{
	long_map_node* root;
	unsigned long num_elements;

}long_map;

typedef struct
{
	long_map lm;
	unsigned char store_keys;
	unsigned long num_elements;

}string_map;



/* long map functions */
extern long_map* initialize_long_map(void);
extern void* get_long_map_element(long_map* map, unsigned long key);
void* get_smallest_long_map_element(long_map* map, unsigned long* smallest_key);
void* get_largest_long_map_element(long_map* map, unsigned long* largest_key);
void* remove_smallest_long_map_element(long_map* map, unsigned long* smallest_key);
void* remove_largest_long_map_element(long_map* map, unsigned long* largest_key);
extern void* set_long_map_element(long_map* map, unsigned long key, void* value);
extern void* remove_long_map_element(long_map* map, unsigned long key);
extern unsigned long* get_sorted_long_map_keys(long_map* map, unsigned long* num_keys_returned);
extern void** get_sorted_long_map_values(long_map* map, unsigned long* num_values_returned);
extern void** destroy_long_map(long_map* map, int destruction_type, unsigned long* num_destroyed);
extern void apply_to_every_long_map_value(long_map* map, void (*apply_func)(unsigned long key, void* value));


/* string map functions */
extern string_map* initialize_string_map(unsigned char store_keys);
extern void* get_string_map_element(string_map* map, const char* key);
extern void* set_string_map_element(string_map* map, const char* key, void* value);
extern void* remove_string_map_element(string_map* map, const char* key);
extern char** get_string_map_keys(string_map* map, unsigned long* num_keys_returned); 
extern void** get_string_map_values(string_map* map, unsigned long* num_values_returned);
extern void** destroy_string_map(string_map* map, int destruction_type, unsigned long* num_destroyed);
extern void apply_to_every_string_map_value(string_map* map, void (*apply_func)(char* key, void* value));
/*
 * three different ways to deal with values when data structure is destroyed
 */
#define DESTROY_MODE_RETURN_VALUES	20
#define DESTROY_MODE_FREE_VALUES 	21
#define DESTROY_MODE_IGNORE_VALUES	22


/* 
 * for convenience & backwards compatibility alias _string_map_ functions to 
 *  _map_ functions since string map is used more often than long map
 */
#define initialize_map		initialize_string_map
#define set_map_element		set_string_map_element
#define get_map_element		get_string_map_element
#define remove_map_element	remove_string_map_element
#define get_map_keys		get_string_map_keys
#define get_map_values		get_string_map_values
#define destroy_map		destroy_string_map



/* list structs / prototypes */

typedef struct list_node_struct
{
	struct list_node_struct* next;
	struct list_node_struct* previous;
	void* value;
} list_node;

typedef struct list_struct
{
	long length;
	list_node* head;
	list_node* tail;
	
}list;

extern list* initialize_list(void);			/* O(1) */



extern void* shift_list(list* l);			/* O(1) */
extern void  unshift_list(list* l, void* value);	/* O(1) */
extern void* pop_list(list* l);				/* O(1) */
extern void  push_list(list*l, void* value);		/* O(1) */


extern void**  destroy_list(list* l, int destruction_type, unsigned long* num_destroyed);	/* O(n) */
extern void* list_element_at(list* l, unsigned long index);					/* O(n) */
extern void** get_list_values(list* l, unsigned long* num_values_returned);			/* O(n) */

/* The idea behind the remove_internal_node function and
 * the other functions below that perform list operations on
 * list_node pointers instead of values is as follows:
 *
 * It is O(n) to remove arbitrary node from list. BUT, if
 * we have a pointer to that node already it is O(1).  So,
 * provide functions to manipulate list with list_node pointers
 * instead of values & a function to remove an internal node
 * given a pointer to that node.  This means we can have
 * access to internal nodes & use another
 * data structure to store internal nodes and delete in O(1)
 */
extern void remove_internal_list_node(list*l, list_node* internal);		/* O(1) */

extern list_node* create_list_node(void* value);			/* O(1) */
extern void* free_list_node(list_node* delete_node);			/* O(1) */
	
extern list_node* shift_list_node(list* l);				/* O(1) */
extern void  unshift_list_node(list* l, list_node* new_node);		/* O(1) */
extern list_node* pop_list_node(list* l);				/* O(1) */
extern void  push_list_node(list*l, list_node* new_node);		/* O(1) */






/* priority_queue structs / prototypes */

typedef struct priority_queue_node_struct
{
	unsigned long priority;
	char* id;
	void* value;
} priority_queue_node;


typedef struct priority_queue_struct
{
	long_map* priorities;
	string_map* ids;
	priority_queue_node* first;
	long length;
} priority_queue;

extern priority_queue* initialize_priority_queue(void);
extern priority_queue_node* create_priority_node(unsigned long priority, char* id, void* value);
extern void* free_priority_queue_node(priority_queue_node* pn);
extern void push_priority_queue(priority_queue* pq, unsigned long priority, char* id, void * value);
extern void* shift_priority_queue(priority_queue* pq, unsigned long* priority, char** id);
extern void* peek_priority_queue(priority_queue* pq, unsigned long* priority, char** id, int dynamic_alloc_id);
extern void* get_priority_queue_element_with_id(priority_queue* pq, char* id, long* priority);
extern void* remove_priority_queue_element_with_id(priority_queue* pq, char* id, long* priority);
extern void push_priority_queue_node(priority_queue* pq, priority_queue_node* pn);
extern priority_queue_node* shift_priority_queue_node(priority_queue* pq);
extern priority_queue_node* get_priority_queue_node_with_id(priority_queue* pq, char* id);
extern priority_queue_node* remove_priority_queue_node_with_id(priority_queue* pq, char* id);
extern void set_priority_for_id_in_priority_queue(priority_queue* pq, char* id, unsigned long priority);
extern priority_queue_node* peek_priority_queue_node(priority_queue* pq);
extern void** destroy_priority_queue(priority_queue* pq, int destroy_mode, unsigned long* num_destroyed);



/* string_util structs / prototypes */

typedef struct 
{
	char* str;
	int terminator;
} dyn_read_t;

/* non-dynamic functions */
extern char* replace_prefix(char* original, char* old_prefix, char* new_prefix);
extern char* trim_flanking_whitespace(char* str);
extern int safe_strcmp(const char* str1, const char* str2);
extern void to_lowercase(char* str);
extern void to_uppercase(char* str);

/* dynamic functions (e.g. new memory is allocated, return values must be freed) */
int free_null_terminated_string_array(char** strs);
char** copy_null_terminated_string_array(char** original);
extern char* dynamic_strcat(int num_strs, ...);
extern char* dcat_and_free(char** one, char** two, int free1, int free2);
extern char** split_on_separators(char* line, char* separators, int num_separators, int max_pieces, int include_remainder_at_max, unsigned long* num_pieces); /*if max_pieces < 0, it is ignored */
extern char* join_strs(char* separator, char** parts, int max_parts, int free_parts, int free_parts_array); /*if max_parts < 0, it is ignored*/
extern char* dynamic_replace(char* template_str, char* old_str, char* new_str);
int convert_to_regex(char* str, regex_t* p);


/* functions to dynamically read files */
extern dyn_read_t dynamic_read(FILE* open_file, char* terminators, int num_terminators, unsigned long* read_length);
extern int dyn_read_line(FILE* open_file, char** dest, unsigned long* read_len);
extern unsigned char* read_entire_file(FILE* in, unsigned long read_block_size, unsigned long* read_length);

/* run a command and get (dynamically allocated) output lines */
extern char** get_shell_command_output_lines(char* command, unsigned long* num_lines);


/*  comparison functions for qsort */ 
extern int sort_string_cmp(const void *a, const void *b);
extern int sort_string_icmp(const void *a, const void *b);

/* wrappers for qsort calls */
extern void do_str_sort(char** string_arr, unsigned long string_arr_len);
extern void do_istr_sort(char** string_arr, unsigned long string_arr_len);




/* safe malloc & strdup functions used by all others (actually aliased to malloc / strdup and used) */
extern void* safe_malloc(size_t size);
extern char* safe_strdup(const char* str);

/* utility functions to free memory */
extern void free_if_not_null(void* p);
extern void free_and_set_null(void** p);


/* other file utils */

extern int mkdir_p(const char* path, mode_t mode); /* returns 0 on success, 1 on error */
extern void rm_r(const char* path);
extern int create_tmp_dir(const char* tmp_root, char** tmp_dir); /* returns 0 on success, 1 on error */



#define PATH_DOES_NOT_EXIST  0
#define PATH_IS_REGULAR_FILE 1
#define PATH_IS_DIRECTORY    2
#define PATH_IS_SYMLINK      3
#define PATH_IS_OTHER        4

/*
returns:
 PATH_DOES_NOT_EXIST  (0) if path doesn't exist
 PATH_IS_REGULAR_FILE (1) if path is regular file
 PATH_IS_DIRECTORY    (2) if path is directory
 PATH_IS_SYMLINK      (3) if path is symbolic link
 PATH_IS_OTHER        (4) if path exists and is something else
 */
extern int path_exists(const char* path);

extern char** get_file_lines(char* file_path, unsigned long* lines_read);


#endif /* ERICS_TOOLS_H */
