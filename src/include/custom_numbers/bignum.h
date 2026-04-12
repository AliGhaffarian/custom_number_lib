#pragma once

#include <custom_numbers/generic.h>
#include <stdbool.h>
#include <stdint.h>

struct bignum_instance {
    bool sign;
    struct node *bignum_elem_linked_list;
};

typedef char bignum_elem_t;

#define BIGNUM_ELEM_GET_DIGITS(higher, lower, elem)                            \
    lower = elem & 0x0f;                                                       \
    higher = (elem & 0xf0) >> 4

#define BIGNUM_ELEM_SET_DIGITS(higher, lower, elem)                            \
    elem = lower;                                                              \
    elem |= (higher << 4)

struct number *bignum_new();
struct number *bignum_from_int(int, ...);
struct number *bignum_from_str(char *);
struct number *bignum_clone(struct number *self);
int bignum_print(FILE *stream, struct number *self);

int bignum_add(struct number *first, struct number *second);
int bignum_sub(struct number *first, struct number *second);
static int _bignum_sub(struct number *first, struct number *second);
int bignum_mul(struct number *first, struct number *second);
int bignum_div(struct number *first, struct number *second);
int bignum_rem(struct number *first, struct number *second);
int bignum_flip_sign(struct number *self);
int bignum_get_sign(struct number *self);
int bignum_is_zero(struct number *self);
int bignum_cmp(struct number *first, struct number *second);
int bignum_u_cmp(struct number *first, struct number *second);
struct number *bignum_gcd(struct number *first, struct number *second);

int bignum_to_int(struct number *self);

void bignum_free_private(struct number *self);

extern struct number_type_ops bignum_ops;

static void cut_bignum_linked_list(struct node *tail);

void __attribute__((constructor)) _custom_numbers_bignum_init();
