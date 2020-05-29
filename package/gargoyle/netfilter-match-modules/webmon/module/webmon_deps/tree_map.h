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


#if __KERNEL__
	#define malloc(foo)	kmalloc(foo,GFP_ATOMIC)
	#define free(foo)	kfree(foo)
	#define printf(format,args...)	printk(format,##args)

	/* kernel strdup */
	static inline char *kernel_strdup(const char *str);
	static inline char *kernel_strdup(const char *str)
	{
		char *tmp;
		long int s;
		s=strlen(str) + 1;
		tmp = kmalloc(s, GFP_ATOMIC);
		if (tmp != NULL)
		{
			memcpy(tmp, str, s);
		}
		return tmp;
	}
	#define strdup kernel_strdup

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
long_map* initialize_long_map(void);
void* get_long_map_element(long_map* map, unsigned long key);
void* get_smallest_long_map_element(long_map* map, unsigned long* smallest_key);
void* get_largest_long_map_element(long_map* map, unsigned long* largest_key);
void* remove_smallest_long_map_element(long_map* map, unsigned long* smallest_key);
void* remove_largest_long_map_element(long_map* map, unsigned long* largest_key);
void* set_long_map_element(long_map* map, unsigned long key, void* value);
void* remove_long_map_element(long_map* map, unsigned long key);
unsigned long* get_sorted_long_map_keys(long_map* map, unsigned long* num_keys_returned);
void** get_sorted_long_map_values(long_map* map, unsigned long* num_values_returned);
void** destroy_long_map(long_map* map, int destruction_type, unsigned long* num_destroyed);
void apply_to_every_long_map_value(long_map* map, void (*apply_func)(unsigned long key, void* value));

/* string map functions */
string_map* initialize_string_map(unsigned char store_keys);
void* get_string_map_element(string_map* map, const char* key);
void* set_string_map_element(string_map* map, const char* key, void* value);
void* remove_string_map_element(string_map* map, const char* key);
char** get_string_map_keys(string_map* map, unsigned long* num_keys_returned); 
void** get_string_map_values(string_map* map, unsigned long* num_values_returned);
void** destroy_string_map(string_map* map, int destruction_type, unsigned long* num_destroyed);
void apply_to_every_string_map_value(string_map* map, void (*apply_func)(char* key, void* value));


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


/* internal utility structures/ functions */
typedef struct stack_node_struct
{
	long_map_node** node_ptr;
	signed char direction;
	struct stack_node_struct* previous;
} stack_node;

static void free_stack(stack_node* stack);
static void** destroy_long_map_values(long_map* map, int destruction_type, unsigned long* num_destroyed);
static void apply_to_every_long_map_node(long_map_node* node, void (*apply_func)(unsigned long key, void* value));
static void apply_to_every_string_map_node(long_map_node* node, unsigned char has_key, void (*apply_func)(char* key, void* value));
static void get_sorted_node_keys(long_map_node* node, unsigned long* key_list, unsigned long* next_key_index, int depth);
static void get_sorted_node_values(long_map_node* node, void** value_list, unsigned long* next_value_index, int depth);
static signed char rebalance (long_map_node** n, signed char direction, signed char update_op);
static void rotate_right (long_map_node** parent);
static void rotate_left (long_map_node** parent);

/* internal for string map */
typedef struct 
{
	char* key;
	void* value;
} string_map_key_value;
static unsigned long sdbm_string_hash(const char *key);




/***************************************************
 * For testing only
 ***************************************************/
/*
void print_list(stack_node *l);

void print_list(stack_node *l)
{
	if(l != NULL)
	{
		printf(" list key = %ld, dir=%d, \n", (*(l->node_ptr))->key, l->direction);
		print_list(l->previous);
	}
}
*/
/******************************************************
 * End testing Code
 *******************************************************/




/***************************************************
 * string_map function definitions
 ***************************************************/

string_map* initialize_string_map(unsigned char store_keys)
{
	string_map* map = (string_map*)malloc(sizeof(string_map));
	if(map != NULL)
	{
		map->store_keys = store_keys;
		map->lm.root = NULL;
		map->lm.num_elements = 0;
		map->num_elements = map->lm.num_elements;
	}
	return map;
}

void* get_string_map_element(string_map* map, const char* key)
{
	unsigned long hashed_key = sdbm_string_hash(key);
	void* return_value =  get_long_map_element( &(map->lm), hashed_key);
	if(return_value != NULL && map->store_keys)
	{
		string_map_key_value* r = (string_map_key_value*)return_value;
		return_value = r->value;
	}
	map->num_elements = map->lm.num_elements;
	return return_value;
}

void* set_string_map_element(string_map* map, const char* key, void* value)
{
	unsigned long hashed_key = sdbm_string_hash(key);
	void* return_value = NULL;
	if(map->store_keys)
	{
		string_map_key_value* kv = (string_map_key_value*)malloc(sizeof(string_map_key_value));
		if(kv == NULL) /* deal with malloc failure */
		{
			return NULL;
		}
		kv->key = strdup(key);
		if(kv->key == NULL) /* deal with malloc failure */
		{
			free(kv);
			return NULL;
		}
		kv->value = value;
		return_value = set_long_map_element(  &(map->lm), hashed_key, kv);
		if(return_value != NULL)
		{
			string_map_key_value* r = (string_map_key_value*)return_value;
			return_value = r->value;
			free(r->key);
			free(r);
		}
	}
	else
	{
		return_value = set_long_map_element( &(map->lm), hashed_key, value);
	}
	map->num_elements = map->lm.num_elements;
	return return_value;
}

void* remove_string_map_element(string_map* map, const char* key)
{
	unsigned long hashed_key = sdbm_string_hash(key);
	void* return_value =  remove_long_map_element( &(map->lm), hashed_key);
	
	if(return_value != NULL && map->store_keys)
	{
		string_map_key_value* r = (string_map_key_value*)return_value;
		return_value = r->value;
		free(r->key);
		free(r);
	}
	map->num_elements = map->lm.num_elements;
	return return_value;
}

char** get_string_map_keys(string_map* map, unsigned long* num_keys_returned)
{
	char** str_keys;
	str_keys = (char**)malloc((map->num_elements+1)*sizeof(char*));
	if(str_keys == NULL) /* deal with malloc failure */
	{
		return NULL;
	}
	str_keys[0] = NULL;
	*num_keys_returned = 0;
	if(map->store_keys && map->num_elements > 0)
	{
		unsigned long list_length;
		void** long_values = get_sorted_long_map_values( &(map->lm),  &list_length);
		unsigned long key_index;
		/*list_length will be 0 on malloc failure in get_sorted_long_map_values, so this code shouldn't seg fault if that happens */
		for(key_index = 0; key_index < list_length; key_index++) 
		{
			str_keys[key_index] = strdup( ((string_map_key_value*)(long_values[key_index]))->key);
			if(str_keys[key_index] == NULL) /* deal with malloc failure */
			{
				//just return the incomplete list (hey, it's null terminated...)
				free(long_values);
				return str_keys;
			}
			*num_keys_returned = *num_keys_returned + 1;
		}
		str_keys[list_length] = NULL;
		free(long_values);
	}
	return str_keys;
}


void** get_string_map_values(string_map* map, unsigned long* num_values_returned)
{
	void** values = NULL;
	if(map != NULL)
	{
		values = get_sorted_long_map_values ( &(map->lm), num_values_returned );
	}
	return values;
}


void** destroy_string_map(string_map* map, int destruction_type, unsigned long* num_destroyed)
{
	void** return_values = NULL;
	if(map != NULL)
	{
		if(map->store_keys)
		{
			void** kvs = destroy_long_map_values( &(map->lm), DESTROY_MODE_RETURN_VALUES, num_destroyed );
			unsigned long kv_index = 0;
			for(kv_index=0; kv_index < *num_destroyed; kv_index++)
			{
				string_map_key_value* kv = (string_map_key_value*)kvs[kv_index];
				void* value = kv->value;
				
				free(kv->key);
				free(kv);
				if(destruction_type == DESTROY_MODE_FREE_VALUES)
				{
					free(value);
				}
				if(destruction_type == DESTROY_MODE_RETURN_VALUES)
				{
					kvs[kv_index] = value;
				}
			}
			if(destruction_type == DESTROY_MODE_RETURN_VALUES)
			{
				return_values = kvs;
			}
			else
			{
				free(kvs);
			}
		}
		else
		{
			return_values = destroy_long_map_values( &(map->lm), destruction_type, num_destroyed );
		}
		free(map);
	}
	return return_values;
}




/***************************************************
 * long_map function definitions
 ***************************************************/

long_map* initialize_long_map(void)
{
	long_map* map = (long_map*)malloc(sizeof(long_map));
	if(map != NULL) /* test for malloc failure */
	{
		map->root = NULL;
		map->num_elements = 0;
	}
	return map;
}

void* get_long_map_element(long_map* map, unsigned long key)
{
	void* value = NULL;

	if(map->root != NULL)
	{
		long_map_node* parent_node = map->root;
		long_map_node* next_node;	
		while( key != parent_node->key && (next_node = (long_map_node *)(key < parent_node->key ? parent_node->left : parent_node->right))  != NULL)
		{
			parent_node = next_node;
		}
		if(parent_node->key == key)
		{
			value = parent_node->value;
		}
	}
	return value;
}

void* get_smallest_long_map_element(long_map* map, unsigned long* smallest_key)
{
	void* value = NULL;
	if(map->root != NULL)
	{
		long_map_node* next_node = map->root;	
		while( next_node->left != NULL)
		{
			next_node = next_node->left;
		}
		value = next_node->value;
		*smallest_key = next_node->key;
	}
	return value;
}

void* get_largest_long_map_element(long_map* map, unsigned long* largest_key)
{
	void* value = NULL;
	if(map->root != NULL)
	{
		long_map_node* next_node = map->root;	
		while( next_node->right != NULL)
		{
			next_node = next_node->right;
		}
		value = next_node->value;
		*largest_key = next_node->key;
	}
	return value;
}

void* remove_smallest_long_map_element(long_map* map, unsigned long* smallest_key)
{
	get_smallest_long_map_element(map, smallest_key);
	return remove_long_map_element(map, *smallest_key);
}

void* remove_largest_long_map_element(long_map* map, unsigned long* largest_key)
{
	get_largest_long_map_element(map, largest_key);
	return remove_long_map_element(map, *largest_key);
}


/* if replacement performed, returns replaced value, otherwise null */
void* set_long_map_element(long_map* map, unsigned long key, void* value)
{
	stack_node* parent_list = NULL;
	void* old_value = NULL;
	int old_value_found = 0;

	long_map_node* parent_node;
	long_map_node* next_node;
	stack_node* next_parent;
	stack_node* previous_parent;
	signed char new_balance;


	long_map_node* new_node = (long_map_node*)malloc(sizeof(long_map_node));
	if(new_node == NULL)
	{
		return NULL;
	}
	new_node->value = value;
	new_node->key = key;
	new_node->left = NULL;
	new_node->right = NULL;
	new_node->balance = 0;

	

	if(map->root == NULL)
	{
		map->root = new_node;	
	}
	else
	{
		parent_node = map->root;
			
		next_parent = (stack_node*)malloc(sizeof(stack_node));
		if(next_parent == NULL) /* deal with malloc failure */
		{
			free(new_node);
			return NULL; /* won't insert but won't seg fault */
		}
		next_parent->node_ptr =  &(map->root);
		next_parent->previous = parent_list;
		parent_list = next_parent;	
			
		while( key != parent_node->key && (next_node = (key < parent_node->key ? parent_node->left : parent_node->right) )  != NULL)
		{
			next_parent = (stack_node*)malloc(sizeof(stack_node));
			if(next_parent == NULL) /* deal with malloc failure */
			{
				/* free previous stack nodes to prevent memory leak */
				free_stack(parent_list);
				free(new_node);
				return NULL;
			}
			next_parent->node_ptr = key < parent_node->key ? &(parent_node->left) : &(parent_node->right);
			next_parent->previous = parent_list;
			next_parent->previous->direction = key < parent_node->key ? -1 : 1;
			parent_list = next_parent;

			parent_node = next_node;
		}
		
		
		if(key == parent_node->key)
		{
			old_value = parent_node->value;
			old_value_found = 1;
			parent_node->value = value;
			free(new_node);
			/* we merely replaced a node, no need to rebalance */
		}
		else
		{	
			if(key < parent_node->key)
			{
				parent_node->left = (void*)new_node;
				parent_list->direction = -1;
			}
			else
			{
				parent_node->right = (void*)new_node;
				parent_list->direction = 1;
			}
			
			
			/* we inserted a node, rebalance */
			previous_parent = parent_list;
			new_balance  = 1; /* initial value is not used, but must not be 0 for initial loop condition */
			
			
			while(previous_parent != NULL && new_balance != 0)
			{
				new_balance = rebalance(previous_parent->node_ptr, previous_parent->direction, 1);
				previous_parent = previous_parent->previous;
			}
		}
	}

	free_stack(parent_list);

	if(old_value_found == 0)
	{
		map->num_elements = map->num_elements + 1;
	}

	return old_value;
}


void* remove_long_map_element(long_map* map, unsigned long key)
{

	void* value = NULL;
	
	long_map_node* root_node = map->root;	
	stack_node* parent_list = NULL;


	long_map_node* remove_parent;
	long_map_node* remove_node;
	long_map_node* next_node;

	long_map_node* replacement;
	long_map_node* replacement_parent;
	long_map_node* replacement_next;

	stack_node* next_parent;
	stack_node* previous_parent;
	stack_node* replacement_stack_node;


	signed char new_balance;



	if(root_node != NULL)
	{
		remove_parent = root_node;
		remove_node = key < remove_parent->key ? remove_parent->left : remove_parent->right;
		
		if(remove_node != NULL && key != remove_parent->key)
		{
			next_parent = (stack_node*)malloc(sizeof(stack_node));
			if(next_parent == NULL) /* deal with malloc failure */
			{
				return NULL;
			}
			next_parent->node_ptr =  &(map->root);
			next_parent->previous = parent_list;
			parent_list = next_parent;	
			while( key != remove_node->key && (next_node = (key < remove_node->key ? remove_node->left : remove_node->right))  != NULL)
			{
				next_parent = (stack_node*)malloc(sizeof(stack_node));
				if(next_parent == NULL) /* deal with malloc failure */
				{
					/* free previous stack nodes to prevent memory leak */
					free_stack(parent_list);
					return NULL;
				}
				next_parent->node_ptr = key < remove_parent->key ? &(remove_parent->left) : &(remove_parent->right);
				next_parent->previous = parent_list;
				next_parent->previous->direction = key < remove_parent->key ? -1 : 1; 
				parent_list = next_parent;
				
				
				remove_parent = remove_node;
				remove_node = next_node;
			}
			parent_list->direction = key < remove_parent-> key ? -1 : 1;
		}
		else
		{
			remove_node = remove_parent;
		}


		if(key == remove_node->key)
		{
			
			/* find replacement for node we are deleting */
			if( remove_node->right == NULL )
			{
				replacement = remove_node->left;
			}
			else if( remove_node->right->left == NULL)
			{

				replacement = remove_node->right;
				replacement->left = remove_node->left;
				replacement->balance = remove_node->balance;

				/* put pointer to replacement node into list for balance update */
				replacement_stack_node = (stack_node*)malloc(sizeof(stack_node));
				if(replacement_stack_node == NULL) /* deal with malloc failure */
				{
					/* free previous stack nodes to prevent memory leak */
					free_stack(parent_list);
					return NULL;
				}
				replacement_stack_node->previous = parent_list;
				replacement_stack_node->direction = 1; /* replacement is from right */
				if(remove_node == remove_parent) /* special case for root node */
				{
					replacement_stack_node->node_ptr = &(map->root);
				}
				else
				{
					replacement_stack_node->node_ptr = key < remove_parent-> key ? &(remove_parent->left) : &(remove_parent->right);
				}
				parent_list = replacement_stack_node;

			}
			else
			{
				/* put pointer to replacement node into list for balance update */
				replacement_stack_node = (stack_node*)malloc(sizeof(stack_node));
				if(replacement_stack_node == NULL) /* deal with malloc failure */
				{
					/* free previous stack nodes to prevent memory leak */
					free_stack(parent_list);
					return NULL;
				}

				replacement_stack_node->previous = parent_list;
				replacement_stack_node->direction = 1; /* we always look for replacement on right */
				if(remove_node == remove_parent) /* special case for root node */
				{
					replacement_stack_node->node_ptr = &(map->root);
				}
				else
				{
					replacement_stack_node->node_ptr = key < remove_parent-> key ? &(remove_parent->left) : &(remove_parent->right);
				}

				parent_list = replacement_stack_node;
				

				/*
				 * put pointer to replacement node->right into list for balance update
				 * this node will have to be updated with the proper pointer
				 * after we have identified the replacement
				 */
				replacement_stack_node = (stack_node*)malloc(sizeof(stack_node));
				if(replacement_stack_node == NULL) /* deal with malloc failure */
				{
					/* free previous stack nodes to prevent memory leak */
					free_stack(parent_list);
					return NULL;
				}

				replacement_stack_node->previous = parent_list;
				replacement_stack_node->direction = -1; /* we always look for replacement to left of this node */
				parent_list = replacement_stack_node;
				
				/* find smallest node on right (large) side of tree */
				replacement_parent = remove_node->right;
				replacement = replacement_parent->left;
				
				while((replacement_next = replacement->left)  != NULL)
				{
					next_parent = (stack_node*)malloc(sizeof(stack_node));
					if(next_parent == NULL) /* deal with malloc failure */
					{
						/* free previous stack nodes to prevent memory leak */
						free_stack(parent_list);
						return NULL;
					}

					next_parent->node_ptr = &(replacement_parent->left);
					next_parent->previous = parent_list;
					next_parent->direction = -1; /* we always go left */
					parent_list = next_parent;

					replacement_parent = replacement;
					replacement = replacement_next;

				}

				replacement_parent->left = replacement->right;
				
				replacement->left = remove_node->left;
				replacement->right = remove_node->right;
				replacement->balance = remove_node->balance;
				replacement_stack_node->node_ptr = &(replacement->right);
			}
			
			/* insert replacement at proper location in tree */
			if(remove_node == remove_parent)
			{
				map->root = replacement;
			}
			else
			{
				remove_parent->left = remove_node == remove_parent->left ? replacement : remove_parent->left;
				remove_parent->right = remove_node == remove_parent->right ? replacement : remove_parent->right;
			}
		

			/* rebalance tree */
			previous_parent = parent_list;
			new_balance = 0;
			while(previous_parent != NULL && new_balance == 0)
			{
				new_balance = rebalance(previous_parent->node_ptr, previous_parent->direction, -1);
				previous_parent = previous_parent->previous;
			}
			
			


			/* 
			 * since we found a value to remove, decrease number of elements in map
			 *  set return value to the deleted node's value and free the node
			 */
			map->num_elements = map->num_elements - 1;
			value = remove_node->value;
			free(remove_node);
		}
	}

	free_stack(parent_list);
	
	return value;
}


/* note: returned keys are dynamically allocated, you need to free them! */
unsigned long* get_sorted_long_map_keys(long_map* map, unsigned long* num_keys_returned)
{
	unsigned long* key_list = (unsigned long*)malloc((map->num_elements)*sizeof(unsigned long));
	unsigned long next_key_index;
	if(key_list == NULL)
	{
		*num_keys_returned = 0;
		return NULL;
	}
	next_key_index = 0;
	get_sorted_node_keys(map->root, key_list, &next_key_index, 0);
	
	*num_keys_returned = map->num_elements;

	return key_list;
}


void** get_sorted_long_map_values(long_map* map, unsigned long* num_values_returned)
{
	void** value_list = (void**)malloc((map->num_elements+1)*sizeof(void*));
	unsigned long next_value_index;

	if(value_list == NULL)
	{
		*num_values_returned = 0;
		return NULL;
	}
	next_value_index = 0;
	get_sorted_node_values(map->root, value_list, &next_value_index, 0);
	value_list[map->num_elements] = NULL; /* since we're dealing with pointers make list null terminated */

	*num_values_returned = map->num_elements;
	return value_list;

}



void** destroy_long_map(long_map* map, int destruction_type, unsigned long* num_destroyed)
{
	void** return_values = destroy_long_map_values(map, destruction_type, num_destroyed);
	free(map);
	return return_values;
}



void apply_to_every_long_map_value(long_map* map, void (*apply_func)(unsigned long key, void* value))
{
	apply_to_every_long_map_node(map->root, apply_func);
}
void apply_to_every_string_map_value(string_map* map, void (*apply_func)(char* key, void* value))
{
	apply_to_every_string_map_node( (map->lm).root, map->store_keys, apply_func);
}


/***************************************************
 * internal utility function definitions
 ***************************************************/
static void free_stack(stack_node* stack)
{
	while(stack != NULL)
	{
		stack_node* prev_node = stack;
		stack = prev_node->previous;
		free(prev_node);
	}

}

static void** destroy_long_map_values(long_map* map, int destruction_type, unsigned long* num_destroyed)
{
	void** return_values = NULL;
	unsigned long return_index = 0;

	*num_destroyed = 0;

	if(destruction_type == DESTROY_MODE_RETURN_VALUES)
	{
		return_values = (void**)malloc((map->num_elements+1)*sizeof(void*));
		if(return_values == NULL) /* deal with malloc failure */
		{
			destruction_type = DESTROY_MODE_IGNORE_VALUES; /* could cause memory leak, but there's no other way to be sure we won't seg fault */
		}
		else
		{
			return_values[map->num_elements] = NULL;
		}
	}
	while(map->num_elements > 0)
	{
		unsigned long smallest_key;
		void* removed_value = remove_smallest_long_map_element(map, &smallest_key);
		if(destruction_type == DESTROY_MODE_RETURN_VALUES)
		{
			return_values[return_index] = removed_value;
		}
		if(destruction_type == DESTROY_MODE_FREE_VALUES)
		{
			free(removed_value);
		}
		return_index++;
		*num_destroyed = *num_destroyed + 1;
	}
	return return_values;
}

static void apply_to_every_long_map_node(long_map_node* node, void (*apply_func)(unsigned long key, void* value))
{
	if(node != NULL)
	{
		apply_to_every_long_map_node(node->left,  apply_func);
		
		apply_func(node->key, node->value);

		apply_to_every_long_map_node(node->right, apply_func);
	}
}
static void apply_to_every_string_map_node(long_map_node* node, unsigned char has_key, void (*apply_func)(char* key, void* value))
{
	if(node != NULL)
	{
		apply_to_every_string_map_node(node->left, has_key,  apply_func);
		
		if(has_key)
		{
			string_map_key_value* kv = (string_map_key_value*)(node->value);
			apply_func(kv->key, kv->value);
		}
		else
		{
			apply_func(NULL, node->value);
		}
		apply_to_every_string_map_node(node->right, has_key, apply_func);
	}
}



static void get_sorted_node_keys(long_map_node* node, unsigned long* key_list, unsigned long* next_key_index, int depth)
{
	if(node != NULL)
	{
		get_sorted_node_keys(node->left, key_list, next_key_index, depth+1);
		
		key_list[ *next_key_index ] = node->key;
		(*next_key_index)++;

		get_sorted_node_keys(node->right, key_list, next_key_index, depth+1);
	}
}

static void get_sorted_node_values(long_map_node* node, void** value_list, unsigned long* next_value_index, int depth)
{
	if(node != NULL)
	{
		get_sorted_node_values(node->left, value_list, next_value_index, depth+1);
		
		value_list[ *next_value_index ] = node->value;
		(*next_value_index)++;

		get_sorted_node_values(node->right, value_list, next_value_index, depth+1);
	}
}



/*
 * direction = -1 indicates left subtree updated, direction = 1 for right subtree
 * update_op = -1 indicates delete node, update_op = 1 for insert node
 */
static signed char rebalance (long_map_node** n, signed char direction, signed char update_op)
{
	/*
	printf( "original: key = %ld, balance = %d, update_op=%d, direction=%d\n", (*n)->key, (*n)->balance, update_op, direction); 
	*/

	(*n)->balance = (*n)->balance + (update_op*direction);
	
	if( (*n)->balance <  -1)
	{
		if((*n)->left->balance < 0)
		{
			rotate_right(n);
			(*n)->right->balance = 0;
			(*n)->balance = 0;
		}
		else if((*n)->left->balance == 0)
		{
			rotate_right(n);
			(*n)->right->balance = -1;
			(*n)->balance = 1;
		}
		else if((*n)->left->balance > 0)
		{
			rotate_left( &((*n)->left) );
			rotate_right(n);
			/*
			if( (*n)->balance < 0 )
			{
				(*n)->left->balance = 0;
				(*n)->right->balance = 1;
			}
			else if( (*n)->balance == 0 )
			{
				(*n)->left->balance = 0;
				(*n)->right->balance = 0;
			}
			else if( (*n)->balance > 0 )
			{
				(*n)->left->balance = -1;
				(*n)->right->balance = 0;
			}
			*/
			(*n)->left->balance  = (*n)->balance > 0 ? -1 : 0;
			(*n)->right->balance = (*n)->balance < 0 ?  1 : 0;
			(*n)->balance = 0;
		}
	}
	if( (*n)->balance >  1)
	{
		if((*n)->right->balance > 0)
		{
			rotate_left(n);
			(*n)->left->balance = 0;
			(*n)->balance = 0;
		}
		else if ((*n)->right->balance == 0)
		{
			rotate_left(n);
			(*n)->left->balance = 1;
			(*n)->balance = -1;
		}
		else if((*n)->right->balance < 0)
		{
			rotate_right( &((*n)->right) );
			rotate_left(n);
			/*
			if( (*n)->balance < 0 )
			{
				(*n)->left->balance = 0;
				(*n)->right->balance = 1;
			}
			else if( (*n)->balance == 0 )
			{
				(*n)->left->balance = 0;
				(*n)->right->balance = 0;
			}
			else if( (*n)->balance > 0 )
			{
				(*n)->left->balance = -1;
				(*n)->right->balance = 0;
			}
			*/
			(*n)->left->balance   = (*n)->balance > 0 ? -1 : 0;
			(*n)->right->balance  = (*n)->balance < 0 ?  1 : 0;
			(*n)->balance = 0;
		}
	}

	/*
	printf( "key = %ld, balance = %d\n", (*n)->key, (*n)->balance);
	*/

	return (*n)->balance;
}


static void rotate_right (long_map_node** parent)
{
	long_map_node* old_parent = *parent;
	long_map_node* pivot = old_parent->left;
	old_parent->left = pivot->right;
	pivot->right  = old_parent;
	
	*parent = pivot;
}

static void rotate_left (long_map_node** parent)
{
	long_map_node* old_parent = *parent;
	long_map_node* pivot = old_parent->right;
	old_parent->right = pivot->left;
	pivot->left  = old_parent;
	
	*parent = pivot;
}



/***************************************************************************
 * This algorithm was created for the sdbm database library (a public-domain 
 * reimplementation of ndbm) and seems to work relatively well in 
 * scrambling bits
 *
 *
 * This code was derived from code found at:
 * http://www.cse.yorku.ca/~oz/hash.html
 ***************************************************************************/
static unsigned long sdbm_string_hash(const char *key)
{
	unsigned long hashed_key = 0;

	int index = 0;
	unsigned int nextch;
	while(key[index] != '\0')
	{
		nextch = key[index];
		hashed_key = nextch + (hashed_key << 6) + (hashed_key << 16) - hashed_key;
		index++;
	}
	return hashed_key;
}


