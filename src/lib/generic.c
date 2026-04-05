#include <custom_numbers/generic.h>
#include <custom_numbers/logger.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct node *number_type_ops_linked_list = NULL;

struct _number_type_ops_linked_list_info number_type_ops_linked_list_info = {
    .len = 0,
    .registered_type_arr = NULL,
};
/*
 * @brief find first available typeid to register a number type
 *
 * @return positive integer on success, zero otherwise
 * since we reserve 0 for a builtin type, it would never be available, therefore
 * it can be used as an invalid value
 */
uint32_t first_available_typeid()
{
    void *alloc_result = NULL;
    uint32_t ret;
    for(; ret < number_type_ops_linked_list_info.len &&
          number_type_ops_linked_list_info.registered_type_arr[ret] == 0;
        ret++)
        ;

    if(ret == number_type_ops_linked_list_info.len)
        alloc_result = realloc(
            number_type_ops_linked_list_info.registered_type_arr,
            (number_type_ops_linked_list_info.len + 1) *
                sizeof(*number_type_ops_linked_list_info.registered_type_arr));

    if(alloc_result == number_type_ops_linked_list_info.registered_type_arr)
        return 0;

    return ret;
}

int is_registered(uint32_t type)
{

    if(type >= number_type_ops_linked_list_info.len)
        goto not_registered;

    if(number_type_ops_linked_list_info.registered_type_arr[type] == 0)
        goto not_registered;

    return 1;

not_registered:
    logger(LOG_DEBUG, stdout, "type %d is not registered\n", type);
    return 0;
}

int register_number_type_ops(struct number_type_ops *ops)
{
    struct node *node_ops = linked_list_make_node_ref(ops);
    int err = 0;
    int did_insert = 0;
    void *alloc_result = NULL;
    int available_typeid = 0;
    int inserted_at = 0;

    if(node_ops == NULL)
        goto fail;

    if(ops->type < _RESERVERD_NUM_SIZE) {
        err = linked_list_replace_at(
            &number_type_ops_linked_list, &node_ops, ops->type);
        if(err) {
            logger(
                LOG_DEBUG,
                stdout,
                "failed to insert at number_type_ops_linked_list %d\n",
                ops->type);
            goto fail;
        }
        did_insert = 1;
        inserted_at = ops->type;
    } else {
        available_typeid = first_available_typeid();
        if(available_typeid == 0) {
            logger(LOG_DEBUG, stdout, "failed to find an available_typeid\n");
            goto fail;
        }

        err = linked_list_replace_at(
            &number_type_ops_linked_list, &node_ops, available_typeid);
        if(err) {
            logger(
                LOG_DEBUG,
                stdout,
                "failed to insert at number_type_ops_linked_list %d\n",
                ops->type);
            goto fail;
        }
        did_insert = 1;
        inserted_at = available_typeid;
    }

    if(ops->type >= number_type_ops_linked_list_info.len) {
        alloc_result = realloc(
            number_type_ops_linked_list_info.registered_type_arr,
            (ops->type + 1) *
                sizeof(*number_type_ops_linked_list_info.registered_type_arr));

        if(alloc_result ==
               number_type_ops_linked_list_info.registered_type_arr &&
           errno) {
            logger(
                LOG_DEBUG,
                stdout,
                "failed to realloc "
                "number_type_ops_linked_list_info.registered_type_arr for "
                "type: %d: %s\n",
                ops->type,
                strerror(errno));
            goto fail;
        }
        number_type_ops_linked_list_info.registered_type_arr = alloc_result;

        number_type_ops_linked_list_info.len = ops->type + 1;
    }

    number_type_ops_linked_list_info.registered_type_arr[ops->type] = 1;
    return 0;

fail:
    logger(LOG_DEBUG, stdout, "failed to register type: %d\n", ops->type);
    struct node *empty_node = linked_list_make_node(NULL);
    if(empty_node == NULL)
        return 1;

    if(did_insert)
        linked_list_replace_at(
            &number_type_ops_linked_list, &empty_node, inserted_at);
    free(node_ops);

    return 1;
}

void generic_free(struct number **n)
{
    if(n == NULL)
        return;
    if(*n == NULL)
        return;

    if((*n)->ops->free_private)
        (*n)->ops->free_private(*n);

    free((*n));

    *n = NULL;
}

struct number *generic_clone(struct number *n)
{
    struct number *ret = NULL;

    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "cloning:");
        n->ops->print(stdout, n);
        puts("");
    }

    if(n->ops->clone)
        ret = n->ops->clone(n);
    else {
        ret = calloc(1, sizeof(struct number));
        if(ret == NULL)
            return NULL;
        memcpy(ret, n, sizeof(struct number));
    }

    return ret;
}

struct number *make_number_from_int(uint32_t type, int n)
{
    struct number_type_ops *ops = NULL;

    if(is_registered(type) == 0) {
        return NULL;
    }

    ops = lookup_type_ops(type);

    if(ops->from_int == NULL)
        return NULL;

    return ops->from_int(n);
}

struct number *make_number_from_two_ints(uint32_t type, int a, int b)
{
    struct number_type_ops *ops = NULL;

    if(is_registered(type) == 0) {
        return NULL;
    }

    ops = lookup_type_ops(type);

    if(ops->from_int == NULL)
        return NULL;

    return ops->from_int(a, b);
}

struct number *make_number_from_number(uint32_t type, struct number *n)
{
    struct number_type_ops *ops = NULL;

    if(is_registered(type) == 0) {
        return NULL;
    }

    ops = lookup_type_ops(type);

    if(ops->from_int == NULL)
        return NULL;

    return ops->from_number(n);
}
struct number *
make_number_from_two_numbers(uint32_t type, struct number *a, struct number *b)
{
    struct number_type_ops *ops = NULL;

    if(is_registered(type) == 0) {
        return NULL;
    }

    ops = lookup_type_ops(type);

    if(ops->from_int == NULL)
        return NULL;

    return ops->from_number(a, b);
}

struct number_type_ops *lookup_type_ops(uint32_t type)
{
    if(type > number_type_ops_linked_list_info.len)
        return NULL;
    if(number_type_ops_linked_list_info.registered_type_arr[type] == 0)
        return NULL;
    return linked_list_get_at(number_type_ops_linked_list, type)->data;
}

int generic_add(struct number *first, struct number *second)
{
    int err = 0;
    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "(");
        first->ops->print(stdout, first);
        printf(") + (");
        first->ops->print(stdout, second);
        printf(")");
    }
    err = first->ops->add(first, second);
    if(current_log_level == LOG_DEBUG) {
        printf(" = ");
        first->ops->print(stdout, first);
        puts("");
    }
    return err;
}
int generic_sub(struct number *first, struct number *second)
{
    int err = 0;
    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "(");
        first->ops->print(stdout, first);
        printf(") - (");
        first->ops->print(stdout, second);
        printf(")");
    }
    err = first->ops->sub(first, second);
    if(current_log_level == LOG_DEBUG) {
        printf(" = ");
        first->ops->print(stdout, first);
        puts("");
    }
    return err;
}
int generic_mul(struct number *first, struct number *second)
{
    int err = 0;
    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "(");
        first->ops->print(stdout, first);
        printf(") * (");
        first->ops->print(stdout, second);
        printf(")");
    }
    err = first->ops->mul(first, second);
    if(current_log_level == LOG_DEBUG) {
        printf(" = ");
        first->ops->print(stdout, first);
        puts("");
    }
    return err;
}
int generic_div(struct number *first, struct number *second)
{
    int err = 0;
    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "(");
        first->ops->print(stdout, first);
        printf(") / (");
        first->ops->print(stdout, second);
        printf(")");
    }
    err = first->ops->div(first, second);
    if(current_log_level == LOG_DEBUG) {
        printf(" = ");
        first->ops->print(stdout, first);
        puts("");
    }
    return err;
}
int generic_flip_sign(struct number *self)
{
    int err = 0;
    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "(");
        self->ops->print(stdout, self);
        printf(") * (");
        printf("-1 )");
    }
    err = self->ops->flip_sign(self);
    if(current_log_level == LOG_DEBUG) {
        printf(" = ");
        self->ops->print(stdout, self);
        puts("");
    }
    return err;
}
int generic_get_sign(struct number *self)
{
    int sign = 0;
    sign = self->ops->get_sign(self);
    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "sign of ");
        self->ops->print(stdout, self);
        printf(" is %d\n", sign);
    }
    return sign;
}
int generic_is_zero(struct number *self)
{
    int is_zero = 0;
    is_zero = self->ops->is_zero(self);
    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "(");
        self->ops->print(stdout, self);
        printf(")");
        printf(is_zero ? " is zero\n" : " is not zero\n");
    }
    return is_zero;
}
int generic_to(struct number *self, uint32_t type)
{
    int err = 1;
    struct number_type_ops *type_ops = NULL;

    if(self->type == type) {
        if(current_log_level) {
            logger(LOG_DEBUG, stdout, "does not need conversion: (");
            self->ops->print(stdout, self);
            puts("");
        }
        return 0;
    }

    if(self->ops->to[type] == NULL) {
        errno = ENOTSUP;
        err = errno;
        goto after_convertion;
    }

    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "(");
        self->ops->print(stdout, self);
        printf(") to type %d", type);
    }
    if(type >= self->ops->to_arr_len)
        goto after_convertion;
    if(self->ops->to[type] == NULL)
        goto after_convertion;

    err = self->ops->to[type](self);
    if(err)
        goto after_convertion;

    type_ops = lookup_type_ops(type);
    // type_ops can't be NULL here, since self->ops->to[type] is not NULL
    self->ops = type_ops;
    self->type = type;

after_convertion:
    if(current_log_level == LOG_DEBUG) {
        printf(" = (");
        self->ops->print(stdout, self);
        printf(") err = %d\n", err);
    }
    return err;
}

int generic_cmp(struct number *first, struct number *second)
{
    int cmp = -2;
    char *cmp_char = "?";
    if(first->type != second->type) {
        logger(
            LOG_ERROR,
            stdout,
            "signed comparison not supported between distinct types\n");
        errno = ENOTSUP;
        return -2;
    }

    if(!first->ops->cmp) {
        logger(
            LOG_ERROR,
            stdout,
            "comparison not supported for type: %d\n",
            first->type);
        errno = ENOTSUP;
        return -2;
    }

    cmp = first->ops->cmp(first, second);

    switch(cmp) {
    case -1: {
        cmp_char = ">";
        break;
    }
    case 0: {
        cmp_char = "=";
        break;
    }
    case 1: {
        cmp_char = "<";
        break;
    }
    default: {
        cmp_char = "error";
        break;
    }
    }

    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "");
        first->ops->print(stdout, first);
        printf(" %s ", cmp_char);
        second->ops->print(stdout, second);
        puts("");
    }

    return cmp;
}

int generic_u_cmp(struct number *first, struct number *second)
{
    int cmp = -2;
    char *cmp_char = "?";
    if(first->type != second->type) {
        logger(
            LOG_ERROR,
            stdout,
            "unsigned comparison not supported between distinct types\n");
        errno = ENOTSUP;
        return -2;
    }

    if(!first->ops->cmp) {
        logger(
            LOG_ERROR,
            stdout,
            "comparison not supported for type: %d\n",
            first->type);
        errno = ENOTSUP;
        return -2;
    }

    cmp = first->ops->u_cmp(first, second);

    switch(cmp) {
    case -1: {
        cmp_char = ">";
        break;
    }
    case 0: {
        cmp_char = "=";
        break;
    }
    case 1: {
        cmp_char = "<";
        break;
    }
    default: {
        cmp_char = "error";
        break;
    }
    }

    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "");
        first->ops->print(stdout, first);
        printf(" %s ", cmp_char);
        second->ops->print(stdout, second);
        puts("");
    }

    return cmp;
}

struct number *make_number_from_str(uint32_t type, char *n)
{
    struct number_type_ops *ops = lookup_type_ops(type);
    if(!ops->from_str)
        return NULL;
    return ops->from_str(n);
}
