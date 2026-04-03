#ifndef K1_LINKED_LIST
#define K1_LINKED_LIST

struct node {
    struct node *next;
    void *data;
};

void linked_list_append(struct node **, struct node **);
struct node *linked_list_make_node(void **);
struct node *linked_list_make_node_ref(void *data);
void linked_list_free(struct node **);
int linked_list_insert_at(struct node **, struct node **, int);
struct node *linked_list_get_at(struct node *, int);
struct node *linked_list_get_tail(struct node *);
void linked_list_push_front(struct node **, struct node **);
void linked_list_push_front_ref(struct node **head, struct node **elem);

#define _free_linked_list_ __attribute__((__cleanup__(linked_list_free)))
#endif
