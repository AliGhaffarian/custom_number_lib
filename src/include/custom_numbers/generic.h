#pragma once

#include <custom_numbers/linked_list.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

enum RESERVED_NUM_TYPES {
    NUMBER_TYPE_INT,
    NUMBER_TYPE_FLOAT,
    NUMBER_TYPE_COMPLEX,
    NUMBER_TYPE_BIGNUM,
    NUMBER_TYPE_RATIONAL,
    _RESERVERD_NUM_SIZE,
};

/**
 * @brief specify operations on the type. Mark not implemented with NULL.
 */
struct number_type_ops {
    // clang-format off
    uint32_t type;
    struct number *(*custom_new)();                             /** construct a zero instance */
    struct number *(*from_int)(int, ...);                       /** construct from int. each type can require different amount of arguments */
    struct number *(*from_number)(struct number **, ...);        /** construct from number object. each type can require different amount of arguments */
    struct number *(*from_str)(char *str);                      /** parse str into an instance */
    struct number *(*clone)(struct number *self);
    int (*print)(FILE *stream, struct number *self);

    int (*add)(struct number *first, struct number *second);    /** add second to first, store in first */
    int (*sub)(struct number *first, struct number *second);    /** subtract second from first, store in first */
    int (*mul)(struct number *first, struct number *second);    /** multiply first and second, store in first */
    int (*div)(struct number *first, struct number *second);    /** divide first and second, store in first */
    int (*rem)(struct number *first, struct number *second);    /** divide first and second, store remainder in first */
    int (*flip_sign)(struct number *self);
    int (*get_sign)(struct number *self);                       /** 0 if '+', 1 if '-', error otherwise */
    int (*is_zero)(struct number *self);
    int (*cmp)(struct number *first, struct number *second);    /** -1 if first is greater, 0 if equal, 1 if second is greater */
    int (*u_cmp)(struct number *first, struct number *second);  /** -1 if first is above, 0 if equal, 1 if second is above */
    int (*to[_RESERVERD_NUM_SIZE])(struct number *self);        /** convert private_data to the type specified by the offset */
    uint32_t to_arr_len;

    void (*free_private)(struct number *n);
    // clang-format on
};

struct number {
    uint32_t type;
    void *private_data;
    struct number_type_ops *ops;
};

void generic_free(struct number **);
struct number *generic_clone(struct number *n);

#define _free_custom_number_ __attribute__((__cleanup__(generic_free)))

uint32_t first_available_typeid();
int is_registered(uint32_t);
int register_number_type_ops(struct number_type_ops *);

extern struct node *number_type_ops_linked_list;

struct number *make_number(uint32_t type);
struct number *make_number_from_int(uint32_t type, int n);
struct number *make_number_from_two_ints(uint32_t type, int a, int b);
struct number *make_number_from_str(uint32_t type, char *n);
struct number *make_number_from_number(uint32_t type, struct number **n);
struct number *make_number_from_two_numbers(
    uint32_t type, struct number **a, struct number **b);

// this helps track what types are registered, in case we want to restrict
// registering types. this should've been stored in the head and i should've
// implemented the linked list to have a head, but silly me.
extern struct _number_type_ops_linked_list_info {
    uint32_t len;
    uint32_t *registered_type_arr;
} number_type_ops_linked_list_info;

struct number_type_ops *lookup_type_ops(uint32_t type);
int generic_add(struct number *first, struct number *second);
int generic_sub(struct number *first, struct number *second);
int generic_mul(struct number *first, struct number *second);
int generic_div(struct number *first, struct number *second);
int generic_rem(struct number *first, struct number *second);
int generic_flip_sign(struct number *self);
int generic_get_sign(struct number *self);
int generic_is_zero(struct number *self);
int generic_to(struct number *self, uint32_t type);
int generic_cmp(struct number *first, struct number *second);
int generic_u_cmp(struct number *first, struct number *second);
struct number *generic_gcd(struct number *first, struct number *second);
