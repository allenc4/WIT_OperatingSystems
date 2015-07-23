#include <stdio.h>
#include <stdlib.h>
#include "LinkedList.h"


/**
 * Insert node at the head of the list.
 */
void push_head(Node_t **head, List_t data)
{
	if ((*head) -> data.child_num == EMPTY)
	{
		(*head) -> data = data;
	} 
	else
	{
		// Create a new node to insert at the head
		Node_t *newNode;
		newNode = malloc(sizeof(Node_t));
		newNode -> data = data;
		newNode -> next = *head;

		*head = newNode;
	}
}

/**
 * Insert node at the end of the list.
 */
void push_tail(Node_t *head, List_t data)
{
	Node_t *current = head;

	while (current -> next != NULL)
	{
		current = current -> next;
	}

	current -> next = malloc(sizeof(Node_t));
	current -> next -> data = data;
	current -> next -> next = NULL;
	return;
}

/**
 * Traverse and print the list
 */
void print(Node_t *head)
{
	Node_t *current = head;

	while (current != NULL)
	{
		if (current == head)
		{
			printf("%d", current -> data.child_num);
		}
		else
		{
			printf(" -> %d", current -> data.child_num);
		}	
		current = current -> next;
	}

	printf("\n");
}

/**
 * Pop (remove) head of the list. The element the current head
 * points to will now the head.
 * 
 * Returns: Data value of deleted node, or -1 if node was not removed.
 */
List_t pop(Node_t **head)
{
	List_t retVal;
        //retVal = malloc(sizeof(List_t));
	
	retVal.child_num = EMPTY;

	if (*head == NULL)
	{
		return retVal;
	}

	retVal = (*head) -> data;

	Node_t *new_head = (*head) -> next;
	*head = new_head;

	return retVal;
}

/**
 * Removes the element in the list at position "position".
 * If position is 0, removes the head. 
 * 
 * Returns: TRUE if pop succeeded. If the position is greater than
 * the list size, returns FALSE.
 */
List_t remove_at(Node_t **head, int position)
{
	Node_t *current = *head;
	List_t data;

	data.child_num = EMPTY;

	int iteration = 0;

	if (position == 0)
	{
		return(pop(head));
	} 
	
	for (iteration = 0; iteration < position; iteration++)
	{
		if (iteration == position - 1)
		{
			
			data = current -> next -> data;
			current -> next = current -> next -> next;
			return data;
		}

		if (current -> next == NULL)
		{
			return data;
		} 
		else
		{
			current = current -> next;
		}
	}

	return data;
}

/**
 * Removes the last node of the linked list.
 *
 * Returns: Instance of List_t structure of the data removed from the list *
 */
List_t remove_last(Node_t **head)
{
	if ((*head) -> next == NULL)
	{
		return (pop(head));
	}

	List_t data;
	Node_t *temp = *head;
	Node_t *current = temp -> next;
	data.child_num = EMPTY;

	while (current -> next != NULL)
	{
		temp = current;
		current = current -> next;
	}

	data = current -> data;
	temp -> next = current -> next;
	free(current);

	return data;
}	

/**
 * Search the list for a specific value (data). If found,
 * returns the position of the element. 
 *
 * Returns: element number in the list if element is found.
 * EMPTY if the element was not found.
 */
int search(Node_t *head, List_t data)
{
	int child_num = data.child_num;
	int position = 0;

	Node_t *current = head;
	while (current != NULL)
	{
		if (current -> data.child_num == child_num)  
		{
			return position;
		}

		current = current -> next;
		position += 1;
	}

	return EMPTY;
}

/**
 * Inserts a data element into the list at position "position".
 * 
 * Returns: TRUE if insert succeeded. If the position is greater than
 * the list size, returns FALSE
 */
int insert_at(Node_t **head, int position, List_t data)
{
	Node_t *newNode;
	newNode = malloc(sizeof(Node_t));
	newNode -> data = data;

	int iteration = 0;
	Node_t *current = *head;

	if (position == 0)
	{
		push_head(head, data);
		return TRUE;
	}

	for(iteration = 0; iteration < position; iteration++)
	{
		if (iteration == position - 1)
		{
			newNode -> next = current -> next;
			current -> next = newNode;
			return TRUE;
		}

		if (current -> next == NULL)
		{
			return FALSE;
		}
		
		current = current -> next;	
	}

	return FALSE;
}

/**
 * Traverses through the linked list until the position "position"
 * is reached.
 *
 * Returns: An instance of the Node_t structure representing the node
 * at position "position", or a dummy Node_t pointer with data of EMPTY
 * if the position was not valid.
 */
Node_t get_node(Node_t *head, int position)
{
	int iteration = 0;
	Node_t *current = head;
	List_t error_data;

	error_data.child_num = EMPTY;

	for (iteration = 0; iteration <= position; iteration++)
	{
		if (iteration == position)
		{
			return *current;
		}

		if (current -> next == NULL)
		{
			current -> data = error_data;
			current -> next = NULL;
			return *current;
		}
		
		current = current -> next;
	}
	
	return *current;
}

/**
 * Traverses through the linked list until the position "position"
 * is reached.
 *
 * Returns: An instance of List_t data type with contents equal to the
 * data of the node at position "position" in the list.
 */
List_t get_data(Node_t *head, int position)
{
	Node_t temp;
	temp = get_node(head, position);

	return temp.data;
}

/**
 * Returns the length of the list starting with head
 */
int list_size(Node_t *head)
{
	int length = 0;

	Node_t *current = head;

	while (current != NULL)
	{
		length++;
		current = current -> next;
	}

	return length;
}

/**
 * Traverses through the linked list and checks if the data of one node
 * is equal to the data of the other node.
 *
 * Returns: TRUE if a duplicate data value is found.
 *          FALSE if no duplicate data values were found.
 */
int check_for_duplicates(Node_t *head)
{
	Node_t *start = head;
	Node_t *cur;

	List_t data;

	while (start -> next != NULL)
	{
		data = start -> data;
		cur = start -> next;

		while (cur != NULL)
		{
			if (data.child_num == cur -> data.child_num)
			{
				return TRUE;
			}
			else
			{	
				cur = cur -> next;
			}

		}

		start = start -> next;
	}

	return FALSE;

}


/**
 * Clears the linked list.
 * Sets the data at the head node to EMPTY. 
 */
void clear(Node_t **head)
{
	(*head) -> next = NULL;
	(*head) -> data.child_num = EMPTY;
	
	/*
	Node_t *current = (*head) -> next;

	// Free all but head node
	while (current != NULL);
	{
		(*head) -> next = current -> next;
		free(current);
		current = (*head) -> next;
	}

	// Empty head node
	(*head) -> data.child_num = EMPTY;
	*/
}
