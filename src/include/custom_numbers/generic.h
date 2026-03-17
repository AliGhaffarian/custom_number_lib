#pragma once

#include <custom_numbers/linked_list.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

enum RESERVED_NUM_TYPES {
    NUMBER_TYPE_INT,
    NUMBER_TYPE_FLOAT,
    NUMBER_TYPE_COMPLEX,
    _RESERVERD_NUM_SIZE,
};

struct number_type_ops {
    uint32_t type;
    struct number *(*custom_new)();
    struct number *(*from_int)(int, ...);
    struct number *(*from_str)(char *);
    struct number *(*clone)(struct number *self);
    int (*print)(FILE *stream, struct number *self);

    int (*add)(struct number *first, struct number *second);
    int (*sub)(struct number *first, struct number *second);
    int (*mul)(struct number *first, struct number *second);
    int (*div)(struct number *first, struct number *second);
    int (*flip_sign)(struct number *self);
    int (*to[_RESERVERD_NUM_SIZE])(struct number *self);
    uint32_t to_arr_len;

    void (*free)(struct number **n);
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

// this helps track what types are registered, in case we want to restrict
// registering types. this should've been stored in the head and i should've
// implemented the linked list to have a head, but silly me.
extern struct _number_type_ops_linked_list_info {
    uint32_t len;
    uint32_t *registered_type_arr;
} number_type_ops_linked_list_info;

int generic_add(struct number *first, struct number *second);
int generic_sub(struct number *first, struct number *second);
int generic_mul(struct number *first, struct number *second);
int generic_div(struct number *first, struct number *second);
int generic_flip_sign(struct number *self);
int generic_to(struct number *first, uint32_t type);
