/*
 * Copyright © 2008 by Eric Bishop <eric@gargoyle-router.com>
 * 
 * This work ‘as-is’ we provide.
 * No warranty, express or implied.
 * We’ve done our best,
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
#define malloc safe_malloc
#define strdup safe_strdup

typedef struct id_map_node_struct
{
	list* id_list;
	list_node* id_node;
} id_map_node;


priority_queue* initialize_priority_queue(void)
{
	priority_queue* pq = (priority_queue*)malloc(sizeof(priority_queue));
	pq->priorities = initialize_long_map();
	pq->ids = initialize_string_map(0);
	pq->first = NULL;
	pq->length = 0;
	return pq;

}

priority_queue_node* create_priority_node(unsigned long priority, char* id, void* value)
{
	priority_queue_node* pn = (priority_queue_node*)malloc(sizeof(priority_queue_node));
	pn->priority = priority;
	pn->id = strdup(id);
	pn->value = value;
	return pn;
}
void* free_priority_queue_node(priority_queue_node* pn)
{
	void *return_value = NULL;
	if(pn != NULL)
	{
		return_value = pn->value;
		free(pn->id);
		free(pn);
	}
	return return_value;
}

void push_priority_queue(priority_queue* pq, unsigned long priority, char* id, void * value)
{
	if(pq != NULL && id != NULL)
	{
		priority_queue_node* pn = create_priority_node(priority, id, value);
		push_priority_queue_node(pq, pn);
	}
}

/* note id is ALWAYS dynamically allocated, need to free */
void* shift_priority_queue(priority_queue* pq, unsigned long* priority, char** id)
{
	void* return_value = NULL;
	priority_queue_node* pn = shift_priority_queue_node(pq);
	if(pn != NULL)
	{
		*priority = pn->priority;
		*id = strdup(pn->id);
		return_value = free_priority_queue_node(pn);
	}
	return return_value;
}

/* last param specified whether to dynamicall allocate id (otherwise pointer to id in priority_queue_node) */
void* peek_priority_queue(priority_queue* pq, unsigned long* priority, char** id, int dynamic_alloc_id)
{
	void* return_value = NULL;
	*priority = 0;
	*id = NULL;
	if(pq != NULL)
	{
		if(pq->first != NULL)
		{
			return_value = pq->first->value;
			*priority = pq->first->priority;
			if(dynamic_alloc_id)
			{
				*id = strdup(pq->first->id);
			}
			else
			{
				*id = pq->first->id;
			}
		}
	}
	return return_value;
}


void* get_priority_queue_element_with_id(priority_queue* pq, char* id, long* priority)
{
	void* return_value = NULL;
	priority_queue_node* pn = get_priority_queue_node_with_id(pq, id);
	if(pn != NULL)
	{
		*priority = pn->priority;
		return_value = free_priority_queue_node(pn);
	}
	else
	{
		*priority = 0;
	}
	return return_value;
}

void* remove_priority_queue_element_with_id(priority_queue* pq, char* id, long* priority)
{
	void* return_value = NULL;
	priority_queue_node* pn = remove_priority_queue_node_with_id(pq, id);
	if(pn != NULL)
	{
		*priority = pn->priority;
		return_value = free_priority_queue_node(pn);
	}
	else
	{
		*priority = 0;
	}
	return return_value;
}


void push_priority_queue_node(priority_queue* pq, priority_queue_node* pn)
{
	if(pq != NULL && pn != NULL)
	{
		/* assume that most of the time we won't have collisions */
		list_node* lpn = create_list_node(pn);
		list* new_list = initialize_list();
		list* old_list;
		id_map_node* idn;

		push_list_node(new_list, lpn);
		old_list = (list*)set_long_map_element(pq->priorities, pn->priority, new_list);
		if(old_list != NULL)
		{
			push_list_node(old_list, lpn);
			set_long_map_element(pq->priorities, pn->priority, old_list);
			free(new_list);
			new_list = old_list;
		}

		/* update first */
		if(pq->first == NULL)
		{
			pq->first = pn;
		}
		else if(pn->priority < pq->first->priority)
		{
			pq->first = pn;
		}
	



		/* save id */
		idn = (id_map_node*)malloc(sizeof(id_map_node));
		idn->id_list = new_list;
		idn->id_node = lpn;
		set_string_map_element(pq->ids, pn->id, idn);

		pq->length = pq->length + 1;;
	}
}

priority_queue_node* shift_priority_queue_node(priority_queue* pq)
{
	priority_queue_node* return_node  = NULL;
	if(pq != NULL)
	{
		if(pq->first != NULL)
		{
			list* next_list = (list*)remove_long_map_element(pq->priorities, pq->first->priority);
			list* next_first_list = NULL;
			list_node* smallest_list_node = shift_list_node(next_list);
			id_map_node* idn;

			if(next_list->length == 0)
			{
				unsigned long tmp;
				destroy_list(next_list, DESTROY_MODE_IGNORE_VALUES, &tmp);
				next_first_list = (list*)get_smallest_long_map_element(pq->priorities, &tmp);
			}
			else
			{
				set_long_map_element(pq->priorities, pq->first->priority, next_list);
				next_first_list = next_list;
			}
			return_node = free_list_node(smallest_list_node);
			idn = (id_map_node*)remove_string_map_element(pq->ids, return_node->id);
			free(idn);
		
			if(next_first_list != NULL)
			{
				list_node* next_first_node = shift_list_node(next_first_list);
				pq->first = (priority_queue_node*)next_first_node->value;
				unshift_list_node(next_first_list, next_first_node);
			}
			else
			{
				pq->first = NULL;
			}

			pq->length = pq->length -1;
		}
	}
		
	return return_node;
}


priority_queue_node* get_priority_queue_node_with_id(priority_queue* pq, char* id)
{
	priority_queue_node* return_node = NULL;
	if(pq != NULL && id != NULL)
	{
		id_map_node* idn = (id_map_node*)get_string_map_element(pq->ids, id);
		if(idn != NULL)
		{
			return_node = (priority_queue_node*)idn->id_node->value;
		}
	}
	return return_node;
}


priority_queue_node* remove_priority_queue_node_with_id(priority_queue* pq, char* id)
{
	priority_queue_node* return_node = NULL;
	if(pq != NULL && id != NULL)
	{
		id_map_node* idn = (id_map_node*)remove_string_map_element(pq->ids, id);
		if(idn != NULL)
		{
			/* remove relevant node from list */
			remove_internal_list_node(idn->id_list, idn->id_node);
			return_node = free_list_node(idn->id_node);

			/* if list is empty remove it from priority map */
			if(idn->id_list->length == 0)
			{
				unsigned long tmp;
				remove_long_map_element(pq->priorities, return_node->priority);
				destroy_list(idn->id_list, DESTROY_MODE_IGNORE_VALUES, &tmp);
			}
			free(idn);

			/* if we're removing first node, reset it */
			if(return_node == pq->first)
			{
				unsigned long tmp;
				list* next_first_list = (list*)get_smallest_long_map_element(pq->priorities, &tmp);
				if(next_first_list != NULL)
				{
					list_node* next_first_node = shift_list_node(next_first_list);
					pq->first = (priority_queue_node*)next_first_node->value;
					unshift_list_node(next_first_list, next_first_node);
				}
				else
				{
					pq->first = NULL;
				}
			}
			pq->length = pq->length -1;
		}
	}
	return return_node;
}

void set_priority_for_id_in_priority_queue(priority_queue* pq, char* id, unsigned long priority)
{
	if(pq != NULL && id != NULL)
	{
		priority_queue_node* id_pq_node = remove_priority_queue_node_with_id(pq, id);
		id_pq_node->priority = priority;
		push_priority_queue_node(pq, id_pq_node);
	}
}

priority_queue_node* peek_priority_queue_node(priority_queue* pq)
{
	return pq->first;
}

void** destroy_priority_queue(priority_queue* pq, int destroy_mode, unsigned long* num_elements_destroyed)
{
	void** values = NULL;
	unsigned long tmp;
	*num_elements_destroyed = 0;
	if(pq != NULL)
	{
		if(destroy_mode == DESTROY_MODE_RETURN_VALUES)
		{
			values = (void**)malloc((pq->length+1)*sizeof(void*));
		}
		while(pq->length > 0)
		{
			priority_queue_node* pqn = shift_priority_queue_node(pq);
			void* next_value = free_priority_queue_node(pqn);
			if(destroy_mode == DESTROY_MODE_RETURN_VALUES)
			{
				values[*num_elements_destroyed] = next_value;
			}
			else if(destroy_mode == DESTROY_MODE_FREE_VALUES)
			{
				free(next_value);
			}
			*num_elements_destroyed = *num_elements_destroyed + 1;
		}
		if(destroy_mode == DESTROY_MODE_RETURN_VALUES)
		{
			values[*num_elements_destroyed] = NULL;
		}

		destroy_long_map(pq->priorities, DESTROY_MODE_FREE_VALUES, &tmp);
		destroy_string_map(pq->ids, DESTROY_MODE_FREE_VALUES, &tmp);
		free(pq);
	}
	return values;
}


