//dlist.h

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
