#pragma once

#include <custom_numbers/generic.h>
#include <stdint.h>

struct complex {
    struct number *re;
    struct number *img;
};

struct number *complex_new();
struct number *complex_from_int(int, ...);
struct number *complex_from_str(char *);
struct number *complex_clone(struct number *self);
int complex_print(FILE *stream, struct number *self);

int complex_add(struct number *first, struct number *second);
int complex_sub(struct number *first, struct number *second);
int complex_mul(struct number *first, struct number *second);
int complex_div(struct number *first, struct number *second);
int complex_flip_sign(struct number *self);
int complex_is_zero(struct number *self);

int complex_to_int(struct number *self);

void complex_free_private(struct number **self);

extern struct number_type_ops complex_ops;

void __attribute__((constructor)) _custom_numbers_complex_init();
