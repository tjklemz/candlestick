/*************************************************************************
 * list.c (Double Linked List)
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

typedef struct node_type {
	struct node_type * next;
	struct node_type * prev;
	void * data;
} Node;

Node *
Node_Init();

void
Node_Destroy(Node * head);

void 
Node_Append(Node * pointer, Node * new_node);

void
Node_Delete(Node * pointer);
