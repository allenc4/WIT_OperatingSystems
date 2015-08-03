#ifndef LINKED_LIST
#define LINKED_LIST

#define TRUE   0
#define FALSE  1
#define EMPTY -1

typedef struct List_Data {
	int child_num;
} List_t;

typedef struct Node {
	List_t data;
	struct Node *next;
} Node_t;

void push_head(Node_t **head, List_t data);
void push_tail(Node_t *head, List_t data);
void print(Node_t *head);

List_t pop(Node_t **head);
List_t remove_at(Node_t **head, int position);
List_t remove_last(Node_t **head);

int search(Node_t *head, List_t data);
int insert_at(Node_t **head, int position, List_t data);

Node_t get_node(Node_t *head, int position);
List_t get_data(Node_t *head, int position);

int list_size(Node_t *head);
int check_for_duplicates(Node_t *head);

void clear(Node_t **head);

#endif
