#include "erics_tools.h"

int main(void)
{
	list* l = initialize_list();

	list_node* c_node = create_list_node("c");
	list_node* a_node = create_list_node("a");
	list_node* z_node = create_list_node("z");
	list_node* x_node = create_list_node("x");

	push_list_node(l, a_node);
	remove_internal_list_node(l, a_node);
	
	push_list_node(l, a_node);
	push_list(l, "b");
	push_list_node(l, c_node);
	unshift_list_node(l, z_node);
	unshift_list(l, "y");
	unshift_list_node(l, x_node);

	remove_internal_list_node(l, z_node);
	remove_internal_list_node(l, a_node);
	remove_internal_list_node(l, x_node);
	
	free_list_node(z_node);
	free_list_node(a_node);
	free_list_node(x_node);

	while(l->length > 0)
	{
		printf("%s\n", (char*)pop_list(l));
	}
	
	
	unsigned long dl;
	destroy_list(l, DESTROY_MODE_IGNORE_VALUES, &dl);


	printf("-------queue test-------------\n");	
	unsigned long priority;
	char* id;
	
	
	priority_queue* pq = initialize_priority_queue();
	push_priority_queue(pq, 30, "id_1", "value_1");
	push_priority_queue(pq, 10, "id_2", "value_2");
	push_priority_queue(pq, 10, "id_3", "value_3");
	push_priority_queue(pq, 40, "id_4", "value_4");
	push_priority_queue(pq,  5, "id_5", "value_5");
	push_priority_queue(pq, 30, "id_6", "value_6");
	push_priority_queue(pq, 30, "id_7", "value_7");

	
	printf("queue length = %ld\n", pq->length);
	unsigned long num_destroyed;

	char* tmp = peek_priority_queue(pq, &priority, &id, 0);
	printf("first is \"%s\"\n", tmp);


	set_priority_for_id_in_priority_queue(pq, "id_5", 35);
	
	tmp = peek_priority_queue(pq, &priority, &id, 0);
	printf("first is \"%s\"\n", tmp);
	
	set_priority_for_id_in_priority_queue(pq, "id_2", 36);

	tmp = peek_priority_queue(pq, &priority, &id, 0);
	printf("first is \"%s\"\n", tmp);

	/*
	char** values = (char**)destroy_priority_queue(pq, DESTROY_MODE_RETURN_VALUES, &num_destroyed);
	int index = 0;
	for(index = 0; values[index] != NULL; index++)
	{
		printf("%s\n", values[index]);
	}
	*/

	
	while(pq->length > 0)
	{
		char* value = (char*)shift_priority_queue(pq, &priority, &id);
		printf("%s\n", value);
		free(id);
	}
	destroy_priority_queue(pq, DESTROY_MODE_FREE_VALUES, &dl);
	
	return 0;
}
