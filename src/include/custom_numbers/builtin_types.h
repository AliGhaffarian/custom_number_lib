#pragma once

#include <custom_numbers/generic.h>

struct number *int_from_int(int n, ...);

int int_add(struct number *first, struct number *second);
int int_sub(struct number *first, struct number *second);
int int_mul(struct number *first, struct number *second);
int int_flip_sign(struct number *self);
int int_is_zero(struct number *self);
int int_div(struct number *first, struct number *second);

int int_print(FILE *stream, struct number *self);
struct number *int_new();
extern struct number_type_ops int_ops;

void __attribute__((constructor)) _custom_numbers_builtin_types_init();
