#include "dlist.h"

#include <stdlib.h>
#include <stdio.h>

Node *
Node_Init()
{
	Node * node = (Node *)malloc(sizeof(Node));
	
	node->next = NULL;
	node->prev = NULL;
	node->data = NULL;
	
	return node;
}

void
Node_Destroy(Node * head)
{
	Node * temp = head;
	
	while(temp->next) {
		head = head->next;
		free(temp);
		temp = head;
	}
	
	free(temp);
	temp = NULL;
}

void
Node_Append(Node * pointer, Node * new_node)
{
	// iterate through the list till we encounter the last Node
	while(pointer->next != NULL) {
		pointer = pointer->next;
	}

	pointer->next = new_node;
	(pointer->next)->prev = pointer;
	pointer = pointer->next;
	pointer->next = NULL;
}

void
Node_Delete(Node * pointer)
{
	if(pointer == NULL) {
		printf("Cannot delete null node.\n");
		return;
	}
	
	pointer->prev->next = pointer->next;
	if(pointer->next) {
		(pointer->next)->prev = pointer->prev;
	}
	free(pointer);
	pointer = NULL;
}
