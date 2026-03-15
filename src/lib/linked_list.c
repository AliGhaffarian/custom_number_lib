#include <custom_numbers/helper.h>
#include <custom_numbers/linked_list.h>
#include <stdlib.h>

void linked_list_append(struct node **head, struct node **to_append)
{
    struct node **ptr_of_interest = head;

    while(*ptr_of_interest != (struct node *)NULL)
        ptr_of_interest = &(*ptr_of_interest)->next;

    *(ptr_of_interest) = *to_append;
    *to_append = NULL;
}

struct node *linked_list_make_node(void **data)
{
    struct node *tmp_node = calloc(1, sizeof(struct node));
    if(!tmp_node)
        return (struct node *)NULL;

    tmp_node->next = (struct node *)NULL;
    if(data) {
        tmp_node->data = move(data);
    } else
        tmp_node->data = NULL;

    return tmp_node;
}

struct node *linked_list_make_node_ref(void *data)
{
    struct node *tmp_node = calloc(1, sizeof(struct node));
    if(!tmp_node)
        return (struct node *)NULL;

    tmp_node->next = (struct node *)NULL;
    if(data) {
        tmp_node->data = data;
    } else
        tmp_node->data = NULL;

    return tmp_node;
}

void linked_list_free(struct node **head)
{
    struct node *current = *head;
    struct node *next = current->next;
    while(current != (struct node *)NULL) {
        next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    *head = NULL;
}

int linked_list_insert_at(struct node **head, struct node **to_insert, int at)
{
    int i = 0;
    struct node *current_node = NULL;
    struct node *node_before_at = NULL;

    if(at == 0) {
        if(*head)
            (*to_insert)->next = move((void **)&(*head)->next);
        else
            (*to_insert)->next = NULL;

        free(*head);
        *head = move((void **)to_insert);
        return 0;
    }

    if(*head == NULL)
        *head = linked_list_make_node(NULL);
    current_node = *head;
    while(i < at - 1) {
        if(current_node->next == NULL)
            current_node->next = linked_list_make_node(NULL);
        if(current_node->next == NULL)
            return 1;

        current_node = current_node->next;

        i++;
    }
    node_before_at = current_node;

    if(node_before_at->next != NULL) {
        (*to_insert)->next = move((void **)&node_before_at->next->next);
        free(node_before_at->next);
    }

    node_before_at->next = move((void **)to_insert);

    return 0;
}

struct node *linked_list_get_at(struct node *head, int at)
{
    int cur = 0;
    struct node *current_node = head;
    while(cur < at) {
        if(current_node->next == NULL)
            return NULL;
        current_node = current_node->next;
        cur++;
    }
    return current_node;
}
