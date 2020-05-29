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




#include "erics_tools.h"
#include <time.h>


void get_max_depth(long_map_node* n, unsigned long* max_depth, unsigned long current_depth);
void get_min_depth(long_map_node* n, unsigned long* min_depth, unsigned long current_depth);
void print_map(long_map_node* n, int depth);

int main (void)
{
	unsigned long  num_insertions = 2500;
	unsigned long  max_insertion = 5000;
	int num_repeats = 3;
	
	unsigned int seed = (unsigned int)time(NULL);
	srand (seed);

	printf("TESTING LONG MAP....\n\n");


	printf("initializing map....\n");
	long_map* lm = initialize_long_map();
	printf("initialized!\n\n");





	int repeat = 0;
	for(repeat =0; repeat < num_repeats; repeat++)
	{

		printf("randomly inserting %ld integers randomly selected (not in order) between 0 and %ld\n", num_insertions, max_insertion);
	

		unsigned long i;
		unsigned long  dupes = 0;
		for(i=0; i<num_insertions; i++)
		{
			unsigned long r = (unsigned long)(max_insertion*((double)rand()/(double)RAND_MAX));
			void* old;
			if( (old = set_long_map_element(lm, r, "a")) != NULL)
			{
				dupes++;
			}
		}

		unsigned long right_depth = 0;
		unsigned long left_depth = 0;
		get_max_depth(lm->root->left, &left_depth, 0);
		get_max_depth(lm->root->right, &right_depth, 0);

		unsigned long original_size = lm->num_elements;
		printf("insertion complete\n");
		printf("after insertion, tree size = %ld \n", original_size);
		printf("(note this may be less than %ld because the same numbers can be selected for insertion more than once, replacing the original node)\n", num_insertions); 
		printf("dupes = %ld\n", dupes);	
		printf("depth of left branch = %ld, depth of right branch = %ld\n\n", left_depth, right_depth);


		if(repeat+1 < num_repeats)
		{

			printf("randomly selecting %ld numbers between 0 and %ld to remove from tree.\n", num_insertions, max_insertion);
			printf("note that these keys will not necessarily be present in tree -- the selection process is entirely random.\n");
	
			int j;
			int found = 0;
			for(j=0; j < num_insertions; j++)
			{
				
				unsigned long r = (unsigned long)(max_insertion*((double)rand()/(double)RAND_MAX));
				if( remove_long_map_element(lm, r) != NULL)
				{
					found++;
				}
			}
			right_depth = 0;
			left_depth = 0;
			get_max_depth(lm->root->left, &left_depth, 0);
			get_max_depth(lm->root->right, &right_depth, 0);
	
			printf("removal complete\n");
			printf("after removal, tree size = %ld \n", lm->num_elements);
			printf("depth of left branch = %ld, depth of right branch = %ld\n", left_depth, right_depth);
			if(original_size - found == lm->num_elements)
			{
				printf("size consistent with number of nodes successfully removed\n\n");
			}
			else
			{
				printf("SIZE IS BAD -- IS NOT CONSISTENT WITH NUMBER OF NODES REMOVED !!!!\n\n");
			}

			printf("removing remaining nodes in tree in random order\n");
			unsigned long length;
			unsigned long *keys = get_sorted_long_map_keys(lm, &length);
			while(lm->root != NULL && lm->num_elements > 0)
			{
				unsigned long r = (unsigned long)(length*((double)rand()/(double)RAND_MAX));
				if( remove_long_map_element(lm, keys[r]) != NULL)
				{
					found++;
					if(original_size - found != lm->num_elements)
					{
						printf("SIZE IS BAD!!!!\n");
					}
				}
			}
			printf("done removing remaining nodes\n");
			printf("tree size is now %ld, and root is %s\n\n", lm->num_elements, lm->root == NULL ? "null" : "not null");

			free(keys);
			
			printf("repeating insertion/deletion\n\n");
		}
		else
		{
			unsigned long num_destroyed;
			printf("destroying map...\n");
			void** values = destroy_long_map(lm, DESTROY_MODE_RETURN_VALUES, &num_destroyed);
			printf("map destroyed.\n");
			int v=0;
			for(v=0; values[v] != NULL; v++){}
			free(values);
			printf("number of values returned after map destruction = %d\n", v);

		}
	}


	printf("LONG MAP TESTING COMPLETE.  TESTING STRING MAP (WITH KEY STORAGE) \n\n");



	printf("initializing map....\n");
	string_map* sm = initialize_string_map(1);
	printf("initialized!\n\n");


	repeat = 0;
	for(repeat =0; repeat < num_repeats; repeat++)
	{

		printf("randomly inserting %ld integer strings randomly selected (not in order) between 0 and %ld\n", num_insertions, max_insertion);

		
		unsigned long i;
		unsigned long  dupes = 0;
		for(i=0; i<num_insertions; i++)
		{
			unsigned long r = (unsigned long)(max_insertion*((double)rand()/(double)RAND_MAX));
			char* new_str = (char*)malloc(40*sizeof(char));
			sprintf(new_str, "%ld", r);
			void* old;
			if( (old = set_string_map_element(sm, new_str, "a")) != NULL )
			{
				dupes++;
			}
			free(new_str);
		}

		unsigned long right_depth = 0;
		unsigned long left_depth = 0;
		get_max_depth(sm->lm.root->left, &left_depth, 0);
		get_max_depth(sm->lm.root->right, &right_depth, 0);
		

		unsigned long original_size = sm->num_elements;
		printf("insertion complete\n");
		printf("after insertion, tree size = %ld \n", original_size);
		printf("(note this may be less than %ld because the same numbers can be selected for insertion more than once, replacing the original node)\n", num_insertions); 
		printf("dupes = %ld\n", dupes);	
		printf("depth of left branch = %ld, depth of right branch = %ld\n\n", left_depth, right_depth);


		if(repeat+1 < num_repeats)
		{
			printf("randomly selecting %ld numbers between 0 and %ld to remove from tree.\n", num_insertions, max_insertion);
			printf("note that these keys will not necessarily be present in tree -- the selection process is entirely random.\n");
	
			int j;
			int found = 0;
			for(j=0; j < num_insertions; j++)
			{
				unsigned long r = (unsigned long)(max_insertion*((double)rand()/(double)RAND_MAX));
				char* new_str = (char*)malloc(40*sizeof(char));
				sprintf(new_str, "%ld", r);
				void* old;
				if( (old = remove_string_map_element(sm, new_str)) != NULL)
				{
					found++;
				}
				free(new_str);
			}
			right_depth = 0;
			left_depth = 0;
			get_max_depth(sm->lm.root->left, &left_depth, 0);
			get_max_depth(sm->lm.root->right, &right_depth, 0);

			printf("removal complete\n");
			printf("after removal, tree size = %ld \n", sm->num_elements);
			printf("depth of left branch = %ld, depth of right branch = %ld\n", left_depth, right_depth);
			if(original_size - found == sm->num_elements)
			{
				printf("size consistent with number of nodes successfully removed\n\n");
			}
			else
			{
				printf("SIZE IS BAD -- IS NOT CONSISTENT WITH NUMBER OF NODES REMOVED !!!!\n\n");
			}

			printf("removing remaining nodes in tree in random order\n");
			unsigned long length = sm->num_elements;
			char** keys = get_string_map_keys(sm, &length);
			while(sm->lm.root != NULL && sm->num_elements > 0)
			{
				unsigned long r = (unsigned long)(length*((double)rand()/(double)RAND_MAX));
				if( remove_string_map_element(sm, keys[r]) != NULL)
				{
					found++;
					if(original_size - found != sm->num_elements)
					{
						printf("SIZE IS BAD!!!!\n");
					}
				}
			}
			
			int k;
			for(k=0; k<length; k++)
			{
				free(keys[k]);
			}
			free(keys);

			printf("done removing remaining nodes\n");
			printf("tree size is now %ld, and root is %s\n\n", sm->num_elements, sm->lm.root == NULL ? "null" : "not null");

			printf("repeating insertion/deletion\n\n");
		}
		else
		{
			unsigned long num_destroyed;
			printf("destroying map...\n");
			void** values = destroy_string_map(sm, DESTROY_MODE_RETURN_VALUES, &num_destroyed);
			printf("map destroyed.\n");
			int v=0;
			for(v=0; values[v] != NULL; v++){  }
			free(values);
			printf("number of values returned after map destruction = %d\n", v);
			

		
		}
	}


	
	
	return(0);
}
void get_max_depth(long_map_node* n, unsigned long* max_depth, unsigned long current_depth)
{
	if(n == NULL)
	{
		*max_depth = current_depth > *max_depth ? current_depth : *max_depth;
		return;
	}
	else
	{
		*max_depth = current_depth+1 > *max_depth ? current_depth+1 : *max_depth;
		get_max_depth(n->left, max_depth, current_depth+1);
		get_max_depth(n->right, max_depth, current_depth+1);
	}
}
void get_min_depth(long_map_node* n, unsigned long* min_depth, unsigned long current_depth)
{
	if(n == NULL)
	{
		return;
	}
	else if(n->left == NULL && n->right == NULL)
	{
		*min_depth = current_depth < *min_depth ? current_depth : *min_depth;
	}
	else
	{
		
		get_min_depth(n->left, min_depth, current_depth+1);
		get_min_depth(n->right, min_depth, current_depth+1);
	}
}


void print_map(long_map_node* n, int depth)
{
	if(n == NULL)
	{
		return;
	}
	int i;
	for(i=0; i < depth; i++){ printf("\t");}
	printf("%ld (%d)\n", n->key, n->balance);
	for(i=0; i < depth; i++){ printf("\t");}
	printf("left:\n");
	print_map(n->left, depth+1);
	for(i=0; i < depth; i++){ printf("\t");}
	printf("right:\n");
	print_map(n->right, depth+1);
}

