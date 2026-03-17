#include <custom_numbers/builtin_types.h>
#include <custom_numbers/generic.h>
#include <custom_numbers/linked_list.h>
#include <custom_numbers/logger.h>
#include <stdlib.h>

int int_add(struct number *first, struct number *second)
{
    *(int *)(&first->private_data) += *(int *)(&second->private_data);
    return 0;
}

int int_sub(struct number *first, struct number *second)
{
    *(int *)(&first->private_data) -= *(int *)(&second->private_data);
    return 0;
}

int int_mul(struct number *first, struct number *second)
{
    *(int *)(&first->private_data) *= *(int *)(&second->private_data);
    return 0;
}

int int_div(struct number *first, struct number *second)
{
    *(int *)(&first->private_data) /= *(int *)(&second->private_data);
    return 0;
}

int int_flip_sign(struct number *self)
{
    *(int *)(&self->private_data) *= -1;
    return 0;
}

int int_is_zero(struct number *self)
{
    return *(int *)(&self->private_data) == 0;
}

int int_print(FILE *stream, struct number *self)
{
    return fprintf(stream, "%d", *(int *)(&self->private_data));
}

struct number *int_new()
{
    struct number *n = calloc(1, sizeof(struct number));
    if(n == NULL)
        return NULL;

    n->type = NUMBER_TYPE_INT;

    n->ops = &int_ops;

    return n;
}

struct number *int_from_int(int n, ...)
{
    struct number *ret = int_new();
    if(ret == NULL)
        return NULL;
    *(int *)(&ret->private_data) = n;
    return ret;
}

struct number_type_ops int_ops = {
    .type = NUMBER_TYPE_INT,
    .custom_new = int_new,
    .from_int = int_from_int,
    .from_str = NULL,
    .print = int_print,

    .add = int_add,
    .sub = int_sub,
    .mul = int_mul,
    .div = int_div,
    .flip_sign = int_flip_sign,
    .is_zero = int_is_zero,

    .to = {0},
    .to_arr_len = 0,
    .free = NULL,
};

void __attribute__((constructor)) _custom_numbers_builtin_types_init()
{
    logger(LOG_DEBUG, stdout, "registering builtin int\n");

    register_number_type_ops(&int_ops);
}
