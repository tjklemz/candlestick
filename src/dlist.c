/*************************************************************************
 * dlist.c (Double Linked List)
 *
 * Candlestick App: Just Write. A minimalist, cross-platform writing app.
 * Copyright (C) 2013 Thomas Klemz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

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
