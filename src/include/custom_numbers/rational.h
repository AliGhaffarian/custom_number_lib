#pragma once
#include <custom_numbers/generic.h>
#include <custom_numbers/linked_list.h>
#include <stdbool.h>

struct rational {
    bool sign;
    struct number *numerator;
    struct number *denominator;
};

struct number *rational_new();
struct number *rational_from_int(int, ...);
struct number *rational_from_number(struct number **n, ...);
struct number *rational_from_str(char *);
struct number *rational_clone(struct number *self);
int rational_print(FILE *stream, struct number *self);

int rational_add(struct number *first, struct number *second);
int rational_sub(struct number *first, struct number *second);
int rational_mul(struct number *first, struct number *second);
int rational_div(struct number *first, struct number *second);
int rational_flip_sign(struct number *self);
int rational_get_sign(struct number *self);
int rational_is_zero(struct number *self);
int rational_cmp(struct number *first, struct number *second);
int rational_u_cmp(struct number *first, struct number *second);

int rational_to_int(struct number *self);

static int _simplify(struct number *self);

void rational_free_private(struct number *self);

extern struct number_type_ops rational_ops;

void __attribute__((constructor)) _custom_numbers_rational_init();
