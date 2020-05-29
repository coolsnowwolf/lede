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


list* initialize_list()
{
	list* l = (list*)malloc(sizeof(list));
	l->length = 0;
	l->head = NULL;
	l->tail = NULL;
	return l;
}

list_node* create_list_node(void* value)
{
	list_node* new_node = (list_node*)malloc(sizeof(list_node));
	new_node->value = value;
	new_node->previous = NULL;
	new_node->next = NULL;
	return new_node;
}
void* free_list_node(list_node* delete_node)
{
	void* value = NULL;
	if(delete_node != NULL)
	{
		value = delete_node->value;
		free(delete_node);
	}
	return value;
}


list_node* shift_list_node(list* l)
{
	list_node* return_node = NULL;
	if(l != NULL)
	{
		if(l->head != NULL)
		{
			return_node = l->head;

			l->head = l->head->next;
			if(l->head != NULL) { l->head->previous = NULL; }
			l->tail = l->tail == return_node ? NULL : l->tail;
			l->length = l->length -1;
			
			return_node->previous = NULL;
			return_node->next = NULL;
		}
	}
	return return_node;
}
void unshift_list_node(list* l, list_node* new_node)
{
	if(l != NULL && new_node != NULL)
	{
		new_node->previous = NULL;
		if(l->head == NULL) /* list is empty */
		{
			new_node->next = NULL;
			l->tail = new_node;
		}
		else
		{
			new_node->next = l->head;
			l->head->previous = new_node;

		}
		l->head = new_node;
		l->length = l->length +1;
	}
}
list_node* pop_list_node(list* l)
{
	list_node* return_node = NULL;
	if(l != NULL)
	{
		if(l->tail != NULL)
		{
			return_node = l->tail;
			l->tail = l->tail->previous;
			if(l->tail != NULL) { l->tail->next = NULL; }
			l->head = l->head == return_node ? NULL : l->head;
			l->length = l->length -1;

			return_node->previous = NULL;
			return_node->next = NULL;
		}
	}
	return return_node;

}
void  push_list_node(list* l, list_node* new_node)
{
	if(l != NULL && new_node != NULL)
	{
		new_node->next = NULL;
		if(l->tail == NULL) /* list is empty */
		{
			new_node->previous = NULL;
			l->head = new_node;
		}
		else
		{
			new_node->previous = l->tail;
			l->tail->next = new_node;
		}
		l->tail = new_node;
		l->length = l->length +1;
	}
}


void* shift_list(list* l)
{
	return free_list_node ( shift_list_node(l) );
}
void  unshift_list(list* l, void* value)
{
	list_node* new_node = create_list_node(value);
	unshift_list_node(l, new_node);
}
void* pop_list(list* l)
{
	return free_list_node ( pop_list_node(l) );
}

void  push_list(list*l, void* value)
{
	list_node* new_node = create_list_node(value);
	push_list_node(l, new_node);
}



void remove_internal_list_node(list* l, list_node* internal)
{
	/* note we assume internal is in l, otherwise everything gets FUBAR! */
	if(l != NULL && internal != NULL)
	{
		list_node* next = internal->next;
		list_node* previous = internal->previous;
		if(previous == NULL) /* internal is head */
		{
			l->head = next;
			if(l->head != NULL) { l->head->previous = NULL; }
		}
		if(next == NULL) /* internal is tail */
		{
			l->tail = previous;
			if(l->tail != NULL) { l->tail->next = NULL; }
		}
		if(previous != NULL && next != NULL)
		{
			previous->next = next;
			next->previous = previous;
		}
		internal->next = NULL;
		internal->previous = NULL;

		l->length = l->length - 1;
	}
}



void**  destroy_list(list* l, int destruction_type, unsigned long* num_values)
{
	void** values = NULL;
	unsigned long value_index = 0;
	if(l != NULL)
	{
		if(destruction_type == DESTROY_MODE_RETURN_VALUES)
		{
			values = (void**)malloc((1+l->length)*sizeof(void*));
		}


		for(value_index=0; l->length > 0; value_index++)
		{
			void* value = shift_list(l);
			if(destruction_type == DESTROY_MODE_RETURN_VALUES)
			{
				values[value_index] = value;
			}
			else if(destruction_type == DESTROY_MODE_FREE_VALUES)
			{
				free(value);
			}
		}
		if(destruction_type == DESTROY_MODE_RETURN_VALUES)
		{
			values[value_index] = NULL;
		}
		free(l);
	}
	*num_values = value_index;
	return values;
}

void* list_element_at(list* l, unsigned long index)
{
	void* return_value = NULL;
	if(l != NULL)
	{
		unsigned long current_index = index - (unsigned long)((l->length)/2) > 0 ? l->length -1 : 0;
		list_node* current_node = current_index == 0 ? l->head : l->tail;

		while(current_index != index && current_node != NULL)
		{
			current_node = current_index > index ? current_node->previous : current_node->next;
			current_index = current_index > index ? current_index -1 : current_index + 1;
		}
		if(current_node != NULL)
		{
			return_value = current_node->value;
		}
	}
	return return_value;
}
void** get_list_values(list* l, unsigned long* num_values) /* returns null terminated array */
{
	void** values = NULL;
	unsigned long value_index = 0;
	if(l != NULL)
	{
		
		list_node* current_node = l->head;
		values = (void**)malloc((1+l->length)*sizeof(void*));

		for(value_index = 0; value_index < l->length; value_index++)
		{
			values[value_index] = current_node->value;
			current_node = current_node->next;
		}
		values[value_index] = NULL;
	}
	*num_values = value_index;
	return values;
}

