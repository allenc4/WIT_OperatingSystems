#include "LinkedList.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DEADLOCK 1

#define MAXNUMBER 9
#define MAXRES 9

Node_t *waiting_list[MAXNUMBER];
int DEADLOCK_DETECTED;


void search_and_add(Node_t **root, List_t dataToLookFor);


int main()
{
	int index;
	
	for(index = 0; index <= MAXNUMBER; index++)
	{
		Node_t *temp;
		temp = malloc(sizeof(Node_t));
		List_t t_data;

		t_data.child_num = EMPTY;
		temp -> data = t_data;
		temp -> next = NULL;

		waiting_list[index] = temp;
	}
	
	waiting_list[0] -> data.child_num = 0;
	waiting_list[1] -> data.child_num = 0;
	waiting_list[2] -> data.child_num = 1;
	waiting_list[3] -> data.child_num = 1;
	waiting_list[4] -> data.child_num = EMPTY;
	waiting_list[5] -> data.child_num = EMPTY;
	waiting_list[6] -> data.child_num = 0;
	waiting_list[7] -> data.child_num = 1;
	waiting_list[8] -> data.child_num = 1;
	waiting_list[9] -> data.child_num = EMPTY;
	
	List_t data;
	
	// List of array index 1
	//data.child_num = 1;
	//push_tail(waiting_list[1], data);
	//data.child_num = 4;
	//push_tail(waiting_list[1], data);

	// List of array index 2
	data.child_num = 0;
	push_tail(waiting_list[2], data);

	// List of array index 6
	data.child_num = 1;
	push_tail(waiting_list[6], data);

	// List of array index 8
	//data.child_num = 2;
	//push_tail(waiting_list[8], data);

	// Print the lists to ensure proper data
	for (index = 0; index <= MAXNUMBER; index++)
	{
		printf("Linked-List of index: %d\n", index);
		print(waiting_list[index]);
		printf("\n");
	}
	
	Node_t *temp = malloc(sizeof(Node_t));

	data.child_num = 2;
	temp -> data = data;
	data.child_num = 2;
	push_tail(temp, data);
	data.child_num = 5;
	push_tail(temp, data);
	

	print(temp);
	printf("\nRemove last:\n");
	data = remove_last(&temp);
	print(temp);
	printf("\nData removed: %d\n", data.child_num);


	// Check for deadlock

	for (index = 0; index < 5; index++)	
	{
		int deadlock = detect_deadlock();
		if (deadlock == TRUE)
			printf("\n\nDEADLOCK DETECTED!!!!\n");
		else
			printf("\n\nNo deadlock detected :)\n");
	}
	return 0;
}

int detect_deadlock()
{
	/**
	 * Algorithm is as follows:
	 * 
	 * Disregard all lists with only 1 element (head) or no data.
	 * Declare last_head variable
	 * for a = 0 to array_size
	 * 	for b = 1 to current_individual_list_size
	 * 		store element[b] in list
	 * 		for c = a+1 to array_size
	 * 			for d = 1 to current_individual_list_size
	 * 				if element[d] == head_data from b
	 * 					store element[d] in list
	 * 					store head data in last_head
	 * 				end if
	 * 			end for
	 * 		end for
	 * 		
	 * 		if list contains duplicates
	 * 			return DEADLOCK
	 * 		end if
	 * 	end for
	 * end for
	 *
	 * return NODEADLOCK
	 */

	
	List_t data;
	Node_t *root = malloc(sizeof(Node_t));

	data.child_num = EMPTY;


	int i;

	for (i = 0; i <= MAXRES; i++)
	{
		//clear(root);
		root -> data = data;
		search_and_add(&root, data);
		printf("List: \n");
		print(root);
		if (DEADLOCK_DETECTED == TRUE)
		{
			break;
		}
		clear(&root);
	}

	return DEADLOCK_DETECTED;

	
	/*
	
	int a, b, c, d, size;
	List_t data, t_head, last_head;
	Node_t *root;

	for (a = 0; a <= MAXNUMBER; a++) // Traverse through array holding linked lists
	{
		root = malloc(sizeof(Node_t));

		data.child_num = EMPTY;
		t_head.child_num = EMPTY;
		last_head.child_num = EMPTY;
		root -> data = data;
		root -> next = NULL;

		size = list_size(waiting_list[a]);

		if (size <= 1)
		{
			continue;
		}

		for (b = 1; b < size; b++) // Traverse through each individual linked list
		{
			data = get_data(waiting_list[a], b); // Store data in list
			if (root -> data.child_num == EMPTY)
			{
				root -> data = data;
			}
			else
			{
				push_tail(root, data);
			}
			t_head = waiting_list[a] -> data; // Hold onto head data, but not in list

			for (c = a+1; c <= MAXNUMBER; c++) // Traverse through rest of array
			{
				int t_size = list_size(waiting_list[c]);
				if (t_size <= 1)
				{
					continue;
				}

				for (d = 1; d <= t_size; d++)
				{
					data = get_data(waiting_list[c], d);
					if (data.child_num == t_head.child_num)
					{
						push_tail(root, data);
						last_head = waiting_list[c] -> data;
					}

				} // End for d

			} // End for c

			push_tail(root, last_head);
			
			// Check for duplicates
			Node_t *start = root;
		
			printf("\n\n\nList:\n");
			print(root);

			int exit_early = FALSE;
			
			while (start != NULL)
			{
				Node_t *current = start -> next;
				while (current != NULL)
				{
					if (start -> data.child_num == current -> data.child_num)
					{
						printf("Duplicates detected\n"); //return DEADLOCK;
						exit_early = TRUE;
						break;
					}
					else
					{
						current = current -> next;
					}
				}

				if (exit_early == TRUE)
				{
					break;
				}

				start = start -> next;
			}

		} // End for b

	} // End for a
		
	return FALSE;

	*/
}

void search_and_add(Node_t **root, List_t dataToLookFor)
{

	int i, first_iteration, lSize, continue_loop, dataFound;
	List_t data;
	Node_t *start, *cur;

	if (dataToLookFor.child_num != EMPTY)
	{
		first_iteration = FALSE;
	}
	else
	{
		first_iteration = TRUE;
	}

	for (i = 0; i <= MAXRES; i++)
	{	
		continue_loop = FALSE;
		dataFound = FALSE;

		start = waiting_list[i];
		lSize = list_size(start);
		
		if (lSize <= 1)
			continue;

		cur = start -> next;

		while (cur != NULL)
		{
			if (first_iteration == TRUE)
			{	
				data = cur -> data;
				push_head(root,data);
				//printf("\nPushing first iteration data: %d\n\n", data.child_num);
				cur = cur -> next;
			}

			else
			{
				// Search for dataToLookFor as a child data
				int position = search(waiting_list[i], dataToLookFor);
				if (position <= 0)
				{
					continue_loop = TRUE;
					break; // Exit while loop, continue searching waiting_list for data
				}
				else
				{
					dataFound = TRUE;
					data = cur -> data;
					//printf("\nPushing data: %d\n\n", data.child_num);
					push_head(root, data);

					if (data.child_num == dataToLookFor.child_num)
					{
						break; // Exit while loop
					}
					else
					{
						cur = cur -> next;
					}
					
				}

			}	
		} // Exit while

		//printf(".....\n\n");
		//print(*root);
		//printf("\n");
		if (first_iteration == TRUE)
		{	
			data.child_num = start -> data.child_num;
			search_and_add(root, data);
			return;
		}
		else
		{	
			if (continue_loop == TRUE)
				continue;

			if (dataFound == TRUE)
			{
				// Search for duplicates
				if (check_for_duplicates(*root) == TRUE)
				{
					DEADLOCK_DETECTED = TRUE;
					return;
				} 
				else 
				{

					data.child_num = start -> data.child_num;
					search_and_add(root, data);
					return; 
				}
			}
			else
			{
				// If this code block is reached, there is no deadlock...
				DEADLOCK_DETECTED = FALSE;
				return;
			}

		}
	
	}

}
