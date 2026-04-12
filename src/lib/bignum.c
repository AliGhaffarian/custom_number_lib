#include <custom_numbers/bignum.h>
#include <custom_numbers/helper.h>
#include <custom_numbers/linked_list.h>
#include <custom_numbers/logger.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct number *bignum_new()
{
    bignum_elem_t elem;
    struct node *bignum_head = linked_list_make_node_ref(NULL);
    struct bignum_instance *instance = calloc(1, sizeof(*instance));
    struct number *self = calloc(1, sizeof(*self));

    if(!(self && instance && bignum_head))
        goto fail;

    BIGNUM_ELEM_SET_DIGITS(0, 0, elem);

    *(bignum_elem_t *)&bignum_head->data =
        elem; // not necessary, since we are allocating it with calloc() and
              // calloc() will zero out the allocated memory
    instance->bignum_elem_linked_list = bignum_head;
    self->private_data = instance;

    self->ops = &bignum_ops;
    self->type = NUMBER_TYPE_BIGNUM;

    return self;

fail:
    free(self);
    free(instance);
    free(bignum_head);
    return NULL;
}

struct number *bignum_from_int(int num, ...)
{
    bignum_elem_t elem;
    struct node *bignum_head = NULL;
    struct bignum_instance *instance = calloc(1, sizeof(*instance));
    struct number *self = bignum_new();
    int num_copy = num;
    int current_higher;
    int current_lower;
    struct node *current_elem_node = NULL;

    if(!(self && instance))
        goto fail;

    self->ops->free_private(self);

    if(num < 0)
        instance->sign = 1;

    do {
        current_lower = num_copy % 10;
        num_copy /= 10;
        current_higher = num_copy % 10;
        num_copy /= 10;

        BIGNUM_ELEM_SET_DIGITS(current_higher, current_lower, elem);

        current_elem_node = linked_list_make_node_ref(NULL);
        *(bignum_elem_t *)&current_elem_node->data = elem;
        linked_list_append(&bignum_head, &current_elem_node);
    } while(num_copy);

    instance->bignum_elem_linked_list = bignum_head;
    self->private_data = instance;

    return self;

fail:
    free(self);
    free(instance);
    free(bignum_head);
    return NULL;
}
struct number *bignum_from_str(char *str)
{
    bignum_elem_t elem;
    struct node *bignum_head = NULL;
    struct bignum_instance *instance = calloc(1, sizeof(*instance));
    struct number *self = bignum_new();
    char *str_ref = str + strlen(str) - 1;
    int current_higher;
    int current_lower;
    struct node *current_elem_node = bignum_head;

    if(!(self && instance))
        goto fail;

    self->ops->free_private(self);

    if(*str == '-') {
        instance->sign = 1;
        str++;
    }

    while(str_ref >= str) {
        current_lower = *str_ref - '0';
        str_ref--;
        if(str_ref >= str) {
            current_higher = *str_ref - '0';
            str_ref--;
        } else {
            current_higher = 0;
        }

        BIGNUM_ELEM_SET_DIGITS(current_higher, current_lower, elem);

        current_elem_node = linked_list_make_node_ref(NULL);
        if(!current_elem_node)
            goto fail;
        *(bignum_elem_t *)&current_elem_node->data = elem;
        linked_list_append(&bignum_head, &current_elem_node);
    }

    instance->bignum_elem_linked_list = bignum_head;
    self->private_data = instance;

    return self;

fail:
    free(self);
    free(instance);
    free(bignum_head);
    return NULL;
}
struct number *bignum_clone(struct number *self)
{
    struct bignum_instance *self_instance = self->private_data;
    struct bignum_instance *ret_instance = NULL;
    struct number *ret = bignum_new();
    if(!ret)
        return NULL;
    ret_instance = ret->private_data;

    free(ret_instance->bignum_elem_linked_list);

    ret_instance->bignum_elem_linked_list =
        linked_list_clone(self_instance->bignum_elem_linked_list, NULL);
    if(!ret_instance->bignum_elem_linked_list)
        return NULL;

    ret_instance->sign = self_instance->sign;

    return ret;
}
int bignum_print(FILE *stream, struct number *self)
{
    bignum_elem_t current_elem;
    int ret = 0;
    struct bignum_instance *current_instance = self->private_data;
    struct node *current_elem_node = current_instance->bignum_elem_linked_list;
    struct node *stack = NULL;
    struct node *current_stack_node = NULL;
    struct node *current_stack_node_next = NULL;
    int current_higher;
    int current_lower;

    // get digits in reverse
    while(current_elem_node) {
        current_stack_node = linked_list_make_node_ref(NULL);
        if(!current_stack_node) {
            logger(
                LOG_ERROR, stdout, "error making node: %s\n", strerror(errno));
            return ret;
        }
        current_stack_node->data =
            current_elem_node->data; // since the data is not actually a
                                     // pointer, this is fine

        linked_list_push_front(&stack, &current_stack_node);
        current_elem_node = current_elem_node->next;
    }

    if(current_instance->sign) {
        putc('-', stream);
        ret++;
    }
    // stack = current_instance->bignum_elem_linked_list; //remove
    current_stack_node = stack;
    while(current_stack_node) {
        BIGNUM_ELEM_GET_DIGITS(
            current_higher,
            current_lower,
            *(bignum_elem_t *)&current_stack_node->data);

        putc(current_higher + '0', stream);
        putc(current_lower + '0', stream);

        current_stack_node_next = current_stack_node->next;
        free(current_stack_node);
        current_stack_node = current_stack_node_next;

        ret += 2;
    }

    return ret;
}

int bignum_add(struct number *first, struct number *second)
{
    int err = 0;
    struct bignum_instance *first_instance = first->private_data;
    struct bignum_instance *second_instance = second->private_data;
    struct node *current_node_first = first_instance->bignum_elem_linked_list;
    struct node *first_tail = current_node_first;
    struct node *second_node_copy = NULL;
    struct node *tmp_node = NULL;
    struct node *current_node_second = second_instance->bignum_elem_linked_list;
    _free_custom_number_ struct number *second_clone = NULL;
    int higher_first = 0, lower_first = 0;
    int new_lower = 0, new_higher = 0;
    int higher_second = 0, lower_second = 0;
    bool carry = 0;

    if(first_instance->sign != second_instance->sign) {
        second_clone = generic_clone(second);
        if(!second_clone)
            return 1;
        generic_flip_sign(second_clone);
        return bignum_sub(first, second_clone);
    }

    while(current_node_second && current_node_first) {
        BIGNUM_ELEM_GET_DIGITS(
            higher_first,
            lower_first,
            *(bignum_elem_t *)&current_node_first->data);
        BIGNUM_ELEM_GET_DIGITS(
            higher_second,
            lower_second,
            *(bignum_elem_t *)&current_node_second->data);

        new_lower = (lower_first + lower_second + carry) % 10;
        carry = (lower_first + lower_second + carry) / 10;
        lower_first = new_lower;

        new_higher = (higher_first + higher_second + carry) % 10;
        carry = (higher_first + higher_second + carry) / 10;
        higher_first = new_higher;

        BIGNUM_ELEM_SET_DIGITS(
            higher_first,
            lower_first,
            *(bignum_elem_t *)&current_node_first->data);

        first_tail = current_node_first;
        current_node_first = current_node_first->next;
        current_node_second = current_node_second->next;
    }

    while(current_node_first) {
        BIGNUM_ELEM_GET_DIGITS(
            higher_first,
            lower_first,
            *(bignum_elem_t *)&current_node_first->data);

        new_lower = (lower_first + carry) % 10;
        carry = (lower_first + carry) / 10;
        lower_first = new_lower;

        new_higher = (higher_first + carry) % 10;
        carry = (higher_first + carry) / 10;
        higher_first = new_higher;

        BIGNUM_ELEM_SET_DIGITS(
            higher_first,
            lower_first,
            *(bignum_elem_t *)&current_node_first->data);

        first_tail->next = current_node_first;
        first_tail = first_tail->next;

        current_node_first = current_node_first->next;
    }

    if(carry) {
        tmp_node = linked_list_make_node_ref(NULL);
        if(!tmp_node)
            return 1;

        BIGNUM_ELEM_SET_DIGITS(0, 1, *(bignum_elem_t *)&tmp_node->data);

        first_tail->next = tmp_node;
        first_tail = first_tail->next;

        carry = 0;
    }

    return err;
}

static int _bignumber_subtract_digit(int first, int second, bool *require_carry)
{
    int ret = 0;
    if(first >= second) {
        ret = first - second;
        if(require_carry)
            *require_carry = 0;
    }
    if(first < second) {
        ret = (first + 10) - second;
        if(require_carry)
            *require_carry = 1;
    }
    return ret;
}

static void cut_bignum_linked_list(struct node *tail)
{
    struct node *current_node = tail;
    struct node *prev_node = current_node->prev;

    // prev node should not be NULL so we have at least one node in the linked
    // list
    while((current_node->data == 0) && (prev_node != NULL)) {
        prev_node = current_node->prev;
        free(current_node);
        current_node = prev_node;
        current_node->next = NULL;
    }
}

int _bignumber_subtract_a_carry(struct node *starting_node)
{
    int err = 0;
    struct node *current_node = NULL;
    int higher = 0, lower = 0;
    bool done = 0;

    if(!starting_node->next)
        return 1; /** caller must ensure this doesn't happen */
    current_node = starting_node->next;

    while(current_node && !done) {
        BIGNUM_ELEM_GET_DIGITS(
            higher, lower, *(bignum_elem_t *)&current_node->data);
        if(lower) {
            lower--;
            done = 1;
            goto done;
        } else
            lower = 9;

        if(higher) {
            higher--;
            done = 1;
            goto done;
        } else
            higher = 9;

    done:
        BIGNUM_ELEM_SET_DIGITS(
            higher, lower, *(bignum_elem_t *)&current_node->data);
        current_node = current_node->next;
    }

    if(!done)
        return 1; /** no carry to be found */

    return err;
}

int bignum_sub(struct number *first, struct number *second)
{
    struct bignum_instance *first_instance = first->private_data;
    struct bignum_instance *second_instance = second->private_data;
    _free_custom_number_ struct number *second_clone = NULL;
    _free_custom_number_ struct number *first_clone = NULL;
    _free_custom_number_ struct number *zero = NULL;
    int first_lower = 0, first_higher = 0;
    int second_lower = 0, second_higher = 0;
    int ret = 0;

    if(first_instance->sign != second_instance->sign) {
        second_clone = generic_clone(second);
        if(!second_clone)
            return 1;
        generic_flip_sign(second_clone);
        return generic_add(first, second_clone);
    }

    switch(generic_u_cmp(first, second)) {
    // equal (didn't need this but since we already did the comparison, this act
    // as a small optimization)
    case 0: {
        first->ops->free_private(first);
        zero = make_number_from_int(NUMBER_TYPE_BIGNUM, 0);
        if(!zero) {
            logger(LOG_DEBUG, stdout, "failed to allocate helper var\n");
            ret = 1;
            break;
        }
        first->private_data = move(&zero->private_data);
        generic_free(&zero);
        ret = 0;
        break;
    }
    // second is above
    case 1: {
        second_clone = generic_clone(second);
        if(!second_clone)
            return 1;

        first_clone = generic_clone(first);
        if(!first_clone)
            return 1;

        second = first_clone;

        // we can't just do first = second_clone, since we want *first to be
        // modified
        first->ops->free_private(first);
        first->private_data = move(&second_clone->private_data);

        if(current_log_level == LOG_DEBUG) {
            logger(LOG_DEBUG, stdout, "changing to: ");
            first->ops->print(stdout, first);
            printf(" - ");
            second->ops->print(stdout, second);
            puts("");
        }
        ret = _bignum_sub(first, second);
        ret |= generic_flip_sign(first);
        break;
    }
    // first > second
    default: {
        ret = _bignum_sub(first, second);
        break;
    }
    }
    return ret;
}

static int _bignum_sub(struct number *first, struct number *second)
{
    struct bignum_instance *first_instance = first->private_data;
    struct bignum_instance *second_instance = second->private_data;
    struct node *first_current_node = first_instance->bignum_elem_linked_list;
    struct node *second_current_node = second_instance->bignum_elem_linked_list;
    int first_lower = 0, first_higher = 0;
    int second_lower = 0, second_higher = 0;
    bool require_carry = 0;

    // now first > second
    // first.sign == second.sign

    while(second_current_node) {
        BIGNUM_ELEM_GET_DIGITS(
            first_higher,
            first_lower,
            *(bignum_elem_t *)&first_current_node->data);
        BIGNUM_ELEM_GET_DIGITS(
            second_higher,
            second_lower,
            *(bignum_elem_t *)&second_current_node->data);

        // subtract lower
        first_lower = _bignumber_subtract_digit(
            first_lower, second_lower, &require_carry);
        if(require_carry && first_higher) {
            first_higher--;
            require_carry = 0;
        }
        if(require_carry) {
            first_higher = 9;
            _bignumber_subtract_a_carry(first_current_node);
        }

        // subtract higher
        first_higher = _bignumber_subtract_digit(
            first_higher, second_higher, &require_carry);
        if(require_carry)
            _bignumber_subtract_a_carry(first_current_node);

        // go forward
        BIGNUM_ELEM_SET_DIGITS(
            first_higher,
            first_lower,
            *(bignum_elem_t *)&first_current_node->data);
        first_current_node = first_current_node->next;
        second_current_node = second_current_node->next;
    }

    cut_bignum_linked_list(
        linked_list_get_tail(first_instance->bignum_elem_linked_list));

    return 0;
}
int bignum_mul(struct number *first, struct number *second)
{
    struct bignum_instance *first_instance = first->private_data;
    struct bignum_instance *second_instance = second->private_data;
    _free_custom_number_ struct number *clone_second = generic_clone(second);
    _free_custom_number_ struct number *clone_first = generic_clone(first);
    struct bignum_instance *clone_second_instance = clone_second->private_data;
    _free_custom_number_ struct number *one =
        make_number_from_int(NUMBER_TYPE_BIGNUM, 1);

    if(!(clone_first && clone_second && one))
        return 1;

    clone_second_instance->sign = 0;
    generic_sub(clone_second, one);
    while(!generic_is_zero(clone_second)) {
        generic_add(first, clone_first);
        generic_sub(clone_second, one);
    }

    first_instance->sign ^= second_instance->sign;

    return 0;
}
int bignum_div(struct number *first, struct number *second)
{
    struct bignum_instance *first_instance = first->private_data;
    struct bignum_instance *second_instance = second->private_data;
    _free_custom_number_ struct number *clone_second = generic_clone(second);
    _free_custom_number_ struct number *clone_first = generic_clone(first);
    _free_custom_number_ struct number *one =
        make_number_from_int(NUMBER_TYPE_BIGNUM, 1);
    _free_custom_number_ struct number *zero =
        make_number_from_int(NUMBER_TYPE_BIGNUM, 0);
    _free_custom_number_ struct number *result =
        make_number_from_int(NUMBER_TYPE_BIGNUM, 0);
    struct bignum_instance *clone_second_instance = clone_second->private_data;
    struct bignum_instance *clone_first_instance = clone_first->private_data;
    struct bignum_instance *result_instance = result->private_data;

    if(!(clone_first && clone_second && one && zero && result))
        return 1;

    clone_first_instance->sign = clone_second_instance->sign = 0;
    while(generic_cmp(clone_first, zero) == -1) {
        generic_sub(clone_first, clone_second);
        generic_add(result, one);
    }

    result_instance->sign = first_instance->sign ^ second_instance->sign;
    first_instance = move(&result->private_data);

    first->ops->free_private(first);

    first->private_data = move((void **)&first_instance);

    return 0;
}
int bignum_rem(struct number *first, struct number *second)
{
    struct bignum_instance *first_instance = first->private_data;
    struct bignum_instance *second_instance = second->private_data;
    _free_custom_number_ struct number *clone_second = generic_clone(second);
    _free_custom_number_ struct number *one =
        make_number_from_int(NUMBER_TYPE_BIGNUM, 1);
    _free_custom_number_ struct number *zero =
        make_number_from_int(NUMBER_TYPE_BIGNUM, 0);
    struct bignum_instance *clone_second_instance = clone_second->private_data;
    int cmp = -2;

    if(!(clone_second && one && zero))
        return 1;

    if(generic_is_zero(second))
        return 1;

    first_instance->sign = clone_second_instance->sign = 0;
    while(generic_cmp(first, zero) == -1) {
        generic_sub(first, clone_second);
    }

    if(generic_cmp(first, zero) == 1)
        generic_add(first, clone_second);

    return 0;
}
int bignum_flip_sign(struct number *self)
{
    struct bignum_instance *self_instance = self->private_data;
    self_instance->sign = !self_instance->sign;
    return 0;
}
int bignum_get_sign(struct number *self)
{
    struct bignum_instance *self_instance = self->private_data;
    return self_instance->sign;
}
int bignum_is_zero(struct number *self)
{
    struct bignum_instance *self_instance = self->private_data;
    struct node *self_node = self_instance->bignum_elem_linked_list;

    return self_node->data == 0 && self_node->next == NULL;
}

int bignum_to_int(struct number *self);

int bignum_cmp(struct number *first, struct number *second)
{
    int cmp = 0;
    struct bignum_instance *instance_first = first->private_data;
    struct bignum_instance *instance_second = second->private_data;
    struct node *first_current_node =
        linked_list_get_tail(instance_first->bignum_elem_linked_list);
    struct node *second_current_node =
        linked_list_get_tail(instance_second->bignum_elem_linked_list);

    if(instance_first->sign != instance_second->sign) {
        cmp = instance_first->sign == 1 ? 1 : -1;
        return cmp;
    }

    while(first_current_node && second_current_node) {
        if(first_current_node->data > second_current_node->data) {
            cmp = -1;
            break;
        }
        if(first_current_node->data < second_current_node->data) {
            cmp = 1;
            break;
        }

        first_current_node = first_current_node->prev;
        second_current_node = second_current_node->prev;
    }

    return cmp;
}

int bignum_u_cmp(struct number *first, struct number *second)
{
    int cmp = 0;
    struct bignum_instance *instance_first = first->private_data;
    struct bignum_instance *instance_second = second->private_data;
    struct node *first_current_node =
        linked_list_get_tail(instance_first->bignum_elem_linked_list);
    struct node *second_current_node =
        linked_list_get_tail(instance_second->bignum_elem_linked_list);

    while(first_current_node && second_current_node) {
        // we don't break on cmp = *; so we can also determine is the numbers
        // are the same length
        if(first_current_node->data > second_current_node->data)
            cmp = -1;
        if(first_current_node->data < second_current_node->data)
            cmp = 1;

        first_current_node = first_current_node->prev;
        second_current_node = second_current_node->prev;
    }

    if(first_current_node)
        cmp = -1;
    if(second_current_node)
        cmp = 1;

    return cmp;
}

struct number *bignum_gcd(struct number *first, struct number *second)
{

    int err = 0;
    _free_custom_number_ struct number *one =
        make_number_from_int(first->type, 1);
    _free_custom_number_ struct number *tmp_num = NULL;
    _free_custom_number_ struct number *two =
        make_number_from_int(first->type, 2);
    struct number *gcd = NULL;
    bool continue_looking_for_gcd = 0;
    int cmp = generic_cmp(first, second);

    switch(cmp) {
    case 0:
    case -1: {
        gcd = generic_clone(second);
        break;
    }
    case 1: {
        gcd = generic_clone(first);
        break;
    }
    default:
        return NULL;
    }

    if(generic_get_sign(gcd))
        generic_flip_sign(gcd);

    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "gcd starting point: ");
        gcd->ops->print(stdout, gcd);
        puts("");
    }

    while(1) {
        continue_looking_for_gcd = 0;

        tmp_num = generic_clone(second);
        if(!tmp_num)
            return NULL;
        err = generic_rem(tmp_num, gcd);
        if(err)
            return NULL;
        errno = 0;
        continue_looking_for_gcd |= (generic_is_zero(tmp_num) == 0);
        if(errno)
            return NULL;

        tmp_num = generic_clone(first);
        if(!tmp_num)
            return NULL;
        err = generic_rem(tmp_num, gcd);
        if(err)
            return NULL;
        errno = 0;
        continue_looking_for_gcd |= (generic_is_zero(tmp_num) == 0);
        if(errno)
            return NULL;

        if(!continue_looking_for_gcd)
            break;

        err = generic_sub(gcd, one);
        if(err)
            return NULL;
    }

    return gcd;
}

void bignum_free_private(struct number *self)
{
    struct bignum_instance *self_instance = self->private_data;
    struct node *self_current_node =
        self_instance ? self_instance->bignum_elem_linked_list : NULL;
    struct node *self_next_node;

    while(self_current_node) {
        self_next_node = self_current_node->next;
        free(self_current_node);
        self_current_node = self_next_node;
    }

    if(self_instance)
        self_instance->bignum_elem_linked_list = NULL;
    free(self->private_data);
    self->private_data = NULL;

    return;
}

struct number_type_ops bignum_ops = {
    .type = NUMBER_TYPE_BIGNUM,

    .custom_new = bignum_new,
    .from_int = bignum_from_int,
    .from_str = bignum_from_str,
    .clone = bignum_clone,

    .print = bignum_print,

    .cmp = bignum_cmp,
    .u_cmp = bignum_u_cmp,
    .gcd = bignum_gcd,

    .add = bignum_add,
    .sub = bignum_sub,
    .mul = bignum_mul,
    .div = bignum_div,
    .rem = bignum_rem,

    .flip_sign = bignum_flip_sign,
    .get_sign = bignum_get_sign,
    .is_zero = bignum_is_zero,

    .to_arr_len = _RESERVERD_NUM_SIZE,

    .free_private = bignum_free_private,
};

void __attribute__((constructor)) _custom_numbers_bignum_init()
{
    logger(LOG_DEBUG, stdout, "registering bignum\n");
    register_number_type_ops(&bignum_ops);
}
