/* PC One parent process NPROC children       *
 *                                            *
 * To compile:				                  *
 *   run 'make' on parent level directory     *
 *                                            *
 **********************************************/

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "LinkedList.h"

#define NPROC           2    /* number of child processes to run */
#define MARKED          1     /* lock not available (a child process already 
								 owns this */
#define NOTMARKED       0     /* lock available (no child process owns this 
								 lock) */
#define MAXCHILDLOCKS	5     /* max resource a child can hold before requesting
                               'release locks' from the LockManager */

#define NO_DEADLOCK             0       /* there is no deadlock */
#define DEADLOCK_DETECTED       1       /* Deadlock detected    */
#define MAXLOCKS                10      /* Total available resources (size of 
										   the lock table) */

/* 
 * Children send this message to request a lock.
 * (What resource they want, and whether to lock or release the resource.)
 */
#define LOCK            100     /* Child requests to lock a resource */
#define RELEASE         200     /* Child requests to release all its resources */
struct msg_requestLock 
{
	int lockID;     /* this a number from 0 up to (MAXLOCKS-1) */
	int Action;     /* LOCK, RELEASE */
};


/* 
 * LockManager sends status of request to children
 */
#define GRANTED         0
#define NOT_GRANTED     1
#define YOU_OWN_IT      2
#define PREVENT         3
struct msg_LockStatus {
	int status;
	int by_child;           /* if not granted, who owns it */
};


/* 
 * Structure the LockManager holds (this is a single lock) 
 */
struct lock {
	int marked;
	int by_child;
};


/*
 * 'lock' holds all the resources 
 */
struct lock locks[MAXLOCKS];   /* MAXLOCKS locks for the manager */

int deadlock = NO_DEADLOCK;     /* When deadlock occurs, exit     */
int pid [NPROC];                /* Process ids                    */

Node_t *waiting_list[MAXLOCKS];  /* Array of linked-lists. One list for 
									each lock resource. */



// Function prototypes
int LockManager( int q, struct msg_requestLock ChildRequest, int respond[NPROC]);
void search_and_add(Node_t **root, List_t dataToLookFor, int startingPos);
int CheckForDeadLock();
void child (int pid, int req, int ack);
void finish();


/******************************************************************
 *
 ******************************************************************/
int main(int argc, char** argv) {
	int i;
	int listen[NPROC];
	int respond[NPROC];
	int ii;

	struct msg_requestLock ChildRequest;


	/* 
	 * Arrange for termination to call the cleanup procedure 
	 */
	signal( SIGTERM, finish );


	/*
	 * initialize, don't care about child  
	 */
	for( ii = 0; ii < MAXLOCKS; ii++) {
		// Initialize waiting_list[]
		Node_t *head = malloc(sizeof(Node_t));
		List_t data;
		data.child_num = EMPTY;
		head -> data = data;
		head -> next = NULL;
		waiting_list[ii] = head;

		// Initialize lock table[]
		locks[ii].marked = NOTMARKED;
	}


	/* 
	 * Initialize pipes and fork the children processes 
	 */
	for( i = 0; i < NPROC; i++ ) {
		int parent_to_child[2];
		int child_to_parent[2];

		/* 
	  	 * Create the child -> parent pipe. 
		 */
    		pipe(child_to_parent);
    		listen[i] = child_to_parent[0];
    		fcntl (listen[i], F_SETFL, O_NONBLOCK);

    		/* 
		 * Create the parent -> child pipe. 
		 */
    		pipe(parent_to_child);
    		respond[i] = parent_to_child[1];
    		fcntl (respond[i], F_SETFL, O_NONBLOCK);

    		/* 
		 * Create child process. 
		 */

		if ((pid[i] = fork()) == 0) {
  			/* 
	 		 * *********** Child process code. ***********
	 		 */
			signal (SIGTERM, SIG_DFL);
			close (parent_to_child[1]);
			close (child_to_parent[0]);

			child (i, child_to_parent[1], parent_to_child[0]);

			_exit(0);
    		}

  		close (child_to_parent[1]);
  		close (parent_to_child[0]);
  	}




	/* 
	 * For this assignment there will never be a deadlock because our
	 * LockManager prevents deadlocks
	 */

	struct timeval tt;

	while( deadlock != DEADLOCK_DETECTED ) { 
		int q;
		for( q = 0; q < NPROC; q++) {
			(void) gettimeofday(&tt, NULL);
			srand(tt.tv_sec * 1000000 + tt.tv_usec);   // Seed random number generator

			long rand_num = ((rand() % 3)) * 100000;   // Random number between 0 and 0.3 second to sleep
			usleep(rand_num);  // Sleep to give a better chance for process intermixing
			
			int nb = read(listen[q], (char *) &ChildRequest, 
					sizeof(ChildRequest));
			if(nb == sizeof(ChildRequest) ) 
			{
				deadlock = LockManager(q, ChildRequest, respond);
			}
		} 
	}

	/* 
	 * Just be nice if your LockManager detects but does not
         * prevent a deadlocks, kill all children processes.
	 */
	finish();
	return 0;	
} 



/* 
 * Called at the end to cleanup
 */
void finish() {
	int i;
	for(i = 0; i < NPROC; i++) 
		kill( pid[i], 9);
	exit(0);
}



/* 
 * Code for the child processes 
 */
void child (int pid, int req, int ack) { 
	int count = 0;		/* It is used for num of locks  */

	struct msg_requestLock MSG;	/* message from child (me) to parent */
	struct msg_LockStatus  STAT; 	/* message from parent to child (me) */

	struct timeval tt;

	(void) gettimeofday(&tt, NULL);
	srand(tt.tv_sec * 1000000 + tt.tv_usec);

	for(;;) {
		MSG.lockID  = rand() % MAXLOCKS;
		MSG.Action  = LOCK;

		printf("\tChild %d: Requesting lock %d . . .\n", pid, MSG.lockID);
		fflush(stdout);

		/*
	 	 * Both calls are blocked if there is nothing to do.
	 	 */

		write( req, (char *) &MSG,  sizeof(MSG));
		read ( ack, (char *) &STAT, sizeof(STAT));

		if( STAT.status == GRANTED ) { 	    /* Success got lock */
			count++;
			printf("\tChild %d: Got lock %d (%d).\n", pid, MSG.lockID, count);
			fflush(stdout);
		}

#ifdef TRACE
		if( STAT.status == GRANTED ) 	
			printf("\tChild %d: Got lock.\n", pid);
		else if( STAT.status == NOT_GRANTED)
			printf("\tChild %d: Child %d owns this lock.\n", pid, STAT.by_child);
		else if( STAT.status == YOU_OWN_IT)
			printf("\tChild %d: I own this lock.\n", pid);

		printf("\tChild %d: Owns %d locks now.\n", pid, count);
		fflush(stdout);
#endif

		if( STAT.status == NOT_GRANTED ) {
			printf("Child %d waiting for lock %d\n", pid, MSG.lockID);
			fflush(stdout);

			/* 
			 * I will get it shortly or the LockManager
		  	 * will NOT give it to me to prevent a deadlock.
			 */
			read ( ack, (char *) &STAT, sizeof(STAT));
			if( STAT.status == GRANTED ) {
				count++;
				printf("\tChild %d: Got lock %d (%d).\n", pid, MSG.lockID, count);
			}
			else if( STAT.status == PREVENT ) {
				printf("CHILD: %d Will try again, (preventing)\n", pid);
				fflush(stdout);
			}
			else {
				printf("CHILD: %d    FATAL ERROR\n", pid);
				fflush(stdout);
				exit(-1);
			}
		}

		if( count >= MAXCHILDLOCKS ) {
			/*
			 * Child sends request to release all its locks
			 */
			printf("\tChild %d: Requesting RELEASE locks.\n", pid);
			fflush(stdout);

			MSG.Action=RELEASE;  
			write(req,(char *) &MSG,sizeof(struct msg_requestLock));

			count = 0;
			sleep(1);
		}

	} /* for(;;) */
} /* child */



/**
 * Method to determine if a deadlock exists.
 *
 * @return DEADLOCK_DETECTED if a deadlock exists
 * @return NO_DEADLOCK if a deadlock does not exist
 */
int CheckForDeadLock() {

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

	/*
	//print lock table first
	for (i = 0; i < MAXLOCKS; i++)
	{
		printf("Resource %d waiting list: ", i);
		print(waiting_list[i]);
	}
	*/
	for (i = 0; i < MAXLOCKS; i++)
	{
		root -> data = data;
		search_and_add(&root, data, i);
		if (deadlock == DEADLOCK_DETECTED)
		{
			break;
		}
		clear(&root);
	}

	return deadlock;
	
}

void search_and_add(Node_t **root, List_t dataToLookFor, int startingPos)
{

	int i, first_iteration, lSize, continue_loop, dataFound;
	List_t data;
	Node_t *start, *cur;

	//If first_iteration, check if element contains children	
	if (dataToLookFor.child_num != EMPTY)
	{
		first_iteration = FALSE;
	}
	else if (startingPos != EMPTY)
	{
		if (list_size(waiting_list[startingPos]) <= 1)
		{
			//The element does not contain any children, so skip 
			return;
		}
	}
	else	
	{
		first_iteration = TRUE;
		
	}

	for (i = 0; i < MAXLOCKS; i++)
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
					break; /* Exit while loop, continue searching 
							  waiting_list for data */
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
			search_and_add(root, data, EMPTY);
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
					printf("Duplicates in list: ");
					print(*root);
					deadlock = DEADLOCK_DETECTED;
					return;
				}
				else 
				{

					data.child_num = start -> data.child_num;
					search_and_add(root, data, EMPTY);
					return; 
				}
			}
			else
			{
				// If this code block is reached, there is no deadlock...
				deadlock = NO_DEADLOCK;
				return;
			}

		}
	
	}

}


/*****************************************************************
 *                                                               
 * Task to determine a deadlock. Also releases and gives locks.
 * Terminates when LiveLock is detected.  
 *                                                     
 * param q: child number (pid[q] is the pid of the process)                      
 * param ChildRequest: instance of msg_RequestLock structure 
 *                     containing the lockID of desired resource
 *                     and Action to take on lockID
 * param respond: file descriptor (array) to write to
 *
 *****************************************************************/

int LockManager( int q, struct msg_requestLock ChildRequest, int respond[NPROC] ) {
	int i;
	struct msg_LockStatus  STAT;
	int deadlock=NO_DEADLOCK;

	if( ChildRequest.Action == RELEASE ) {
		/*      
		 * Release child's resources.
		 * Give resources to children that might be waiting 
		 * for these resources.
		 */
		for (i=0; i< MAXLOCKS; i++)
		{
			if ((locks[i].by_child == q) && (locks[i].marked == MARKED))
			{
				// If another child is waiting for the resource, give the lock
				// to the waiting process
				if (waiting_list[i] -> next != NULL)
				{
					// There is a process waiting
					pop(&waiting_list[i]);
					locks[i].by_child = waiting_list[i] -> data.child_num;
					printf("Gave lock ID %d to child proc %d...\n", 
							i, locks[i].by_child);
					fflush(stdout);
					STAT.status = GRANTED;
					write(respond[locks[i].by_child], (char *) &STAT, sizeof(STAT));
				}
				else
				{
					// No process waiting for resource
					waiting_list[i] -> data.child_num = EMPTY;
					locks[i].marked = NOTMARKED;
					locks[i].by_child = EMPTY;		
					printf("Released lock ID %d...\n", i);		
					fflush(stdout);
				}
			}	

		} // End for 
	}
	
	if( ChildRequest.Action == LOCK ) {
		int t_lock;
		t_lock = ChildRequest.lockID;
		if( locks[t_lock].marked == NOTMARKED ) {
			/*      
		 	 * Give requested lock to child and write status response to respond 
		 	 */
			List_t data;
			data.child_num = q;

			locks[t_lock].marked = MARKED;
			locks[t_lock].by_child = q;
			push_head(&waiting_list[t_lock], data);
			
			STAT.status = GRANTED;
			write(respond[q], (char *) &STAT, sizeof(STAT));
		}
		else { /* lock is not free */
			if( locks[t_lock].by_child == q ) {
				/* 	
				 * Tell child that this lock is already owned
			 	 * by this (the requestor) child
				 */
				STAT.status=YOU_OWN_IT;
				write(respond[q], (char *) &STAT, sizeof(STAT));
			}
			else { /* lock taken by another child */
				/*      
				 * Lock is owned by another child, need to wait!
				 * Set up the waiting list.
				 */
				List_t data;
				data.child_num = q;

				if (waiting_list[t_lock] -> data.child_num == EMPTY)
				{
					// No items in list (head is empty)
					// Replace empty head data with child ID
					push_head(&waiting_list[t_lock], data);
				}
				else
				{
					// Already items in list. Add child ID to the tail
					push_tail(waiting_list[t_lock], data);
				}

				
				/* 
				 * Now tell the child that the Lock will 
				 * not be given (because it's owned by 
				 * someone else.
				 */
				STAT.status=NOT_GRANTED;		
				STAT.by_child = locks[t_lock].by_child;
				write(respond[q], (char *) &STAT, sizeof(STAT));

				/*      
				 * Print the lock table and the waiting list
				 * so that YOU see what your program is doing
				 */
				printf("Lock ID %d added to queue. Current list is: ", t_lock);
				print(waiting_list[t_lock]);
				fflush(stdout);

				/*      
				 * Implement the 'CheckForDeadLock()' 
				 * function.  If you see a deadlock return
				 * DEADLOCK_DETECTED as shown below.
				 */
				if( CheckForDeadLock(q) == DEADLOCK_DETECTED ) {
					
					printf("\nDeadlock Detected! Rolling back...\n");
					fflush(stdout);
					/*      
				 	 * Detected Deadlock! you need to
					 * prevent it.  Rollback so that
					 * you will not give the lock to 
					 * the requestor.
					 */
					List_t rollbackData = remove_last(&waiting_list[t_lock]);
					
					if (rollbackData.child_num == EMPTY)
					{
						// There was a problem with removing the last element
						printf("Error rolling back. Killing program.\n");
						fflush(stdout);
						_exit(-1);
					}

					/* 
					 * OK we rolledback, now notify the 
					 * child that the lock will not be
					 * given to it in order to prevent
					 * the deadlock.
					 */
					STAT.status=PREVENT;		
					write(respond[q], (char *) &STAT, sizeof(STAT));

					
					/*      
					 * Now that we prevented the deadlock
					 * check to see if there are any free
					 * locks.  If not, we will be
					 * preventing deadlocks for ever.
					 * If there are no more free 
					 * locks/resources then we got a 
					 * LiveLock and we should
					 * 1. Print a message on the screen
					 * 2. Print the lock table and the 
					 *    waiting list/graph.
					 * 3. Terminate.
					 */
				
					// Check for any free locks
					int index;
					int freeLocks = FALSE;
					for (index = 0; index < MAXLOCKS; index++)
					{
						if (locks[index].marked == NOTMARKED)
						{
							freeLocks = TRUE;
							break;
						}
					}

					if (freeLocks == FALSE)
					{	
						printf("*****************************************\n");
						printf("  LIVELOCK DETECTED. NO MORE FREE LOCKS\n");
						printf("*****************************************\n");
						for (index = 0; index < MAXLOCKS; index++)
						{
							printf("Lock ID: %d taken by child number: %d\n", 
									index, locks[index].by_child);
						}

						printf("Current waiting list:\n");
						for (index = 0; index < MAXLOCKS; index++)
						{
							printf("Processes waiting for resource %d: ", index);
							print(waiting_list[index]);
						}
						fflush(stdout);

						finish();
						exit(-1);

					} // End if
		
				}
			}
		}
	}


	//
	// if ChildRequest.Action is neither RELEASE nor LOCK, you got a protocol error.
	//

	return(deadlock);
} 
