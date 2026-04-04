#include <custom_numbers/helper.h>
#include <custom_numbers/linked_list.h>
#include <stdlib.h>

void linked_list_append(struct node **head, struct node **to_append)
{
    struct node **ptr_of_interest = head;
    struct node *prev_node = NULL;

    while(*ptr_of_interest != (struct node *)NULL) {
        prev_node = *ptr_of_interest;
        ptr_of_interest = &(*ptr_of_interest)->next;
    }

    *(ptr_of_interest) = *to_append;
    (*ptr_of_interest)->prev = prev_node;
    *to_append = NULL;
}

struct node *linked_list_make_node(void **data)
{
    struct node *tmp_node = calloc(1, sizeof(struct node));
    if(!tmp_node)
        return (struct node *)NULL;

    tmp_node->next = (struct node *)NULL;
    tmp_node->prev = (struct node *)NULL;
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
    tmp_node->prev = (struct node *)NULL;
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

int linked_list_replace_at(struct node **head, struct node **to_insert, int at)
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
        if(current_node->next == NULL) {
            current_node->next = linked_list_make_node(NULL);

            if(current_node->next == NULL)
                return 1;

            current_node->next->prev = current_node;
        }

        current_node = current_node->next;

        i++;
    }
    node_before_at = current_node;

    if(node_before_at->next != NULL) {
        (*to_insert)->next = move((void **)&node_before_at->next->next);
        free(node_before_at->next);
    }

    (*to_insert)->prev = node_before_at;
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

void linked_list_push_front(struct node **head, struct node **elem)
{
    (*elem)->next = move((void **)head);

    *head = move((void **)elem);
}

void linked_list_push_front_ref(struct node **head, struct node **elem)
{
    (*elem)->next = *head;

    *head = *elem;
}

struct node *linked_list_get_tail(struct node *head)
{
    if(!head)
        return NULL;
    if(!head->next)
        return head;

    struct node *current_node = head;
    struct node *next_node = head->next;

    while(next_node) {
        current_node = next_node;
        next_node = next_node->next;
    }
    return current_node;
}

struct node *
linked_list_clone(struct node *self, void *(*private_clone)(void *))
{
    struct node *cloned_current_node = NULL;
    void *cloned_private_data = NULL;
    struct node *clone_head = NULL;
    struct node *self_current_node = self;

    while(self_current_node) {
        if(private_clone) {
            cloned_private_data = private_clone(self_current_node->data);
            if(!cloned_private_data)
                goto fail;
        } else
            cloned_private_data = self_current_node->data;

        cloned_current_node = linked_list_make_node(&cloned_private_data);
        if(!cloned_current_node)
            goto fail;

        linked_list_append(&clone_head, &cloned_current_node);
        self_current_node = self_current_node->next;
    }
    return clone_head;
fail:
    // TODO: free if error
    return NULL;
}
