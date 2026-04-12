#include <custom_numbers/generic.h>
#include <custom_numbers/helper.h>
#include <custom_numbers/logger.h>
#include <custom_numbers/rational.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>

struct number *rational_new()
{
    struct number *ret = calloc(1, sizeof(*ret));
    struct rational *ret_rational = calloc(1, sizeof(*ret_rational));
    _free_custom_number_ struct number *numerator =
        make_number_from_int(NUMBER_TYPE_BIGNUM, 0);
    _free_custom_number_ struct number *denominator =
        make_number_from_int(NUMBER_TYPE_BIGNUM, 1);

    if(!(ret && ret_rational && denominator && numerator))
        return NULL;

    ret_rational->numerator = move((void **)&numerator);
    ret_rational->denominator = move((void **)&denominator);
    ret_rational->sign = 0;

    ret->type = NUMBER_TYPE_RATIONAL;
    ret->ops = &rational_ops;
    ret->private_data = move((void **)&ret_rational);

    return ret;
}
/*
 * @brief create rational number based on int args
 * @note this function takes two arguments, the signature is weird because it
 * needs to fit in the generic API
 */
struct number *rational_from_int(int n, ...)
{
    va_list ap;
    int tmp_arg = 0;
    struct number *ret = rational_new();
    struct rational *ret_rational = calloc(1, sizeof(*ret_rational));
    _free_custom_number_ struct number *denominator =
        make_number_from_int(NUMBER_TYPE_BIGNUM, n >= 0 ? n : -n);
    _free_custom_number_ struct number *numerator = NULL;
    bool sign = 0;

    va_start(ap, n);
    tmp_arg = va_arg(ap, int);
    va_end(ap);

    numerator = make_number_from_int(
        NUMBER_TYPE_BIGNUM, tmp_arg >= 0 ? tmp_arg : -tmp_arg);

    sign = (n < 0) ^ (tmp_arg < 0);

    if(!(ret && ret_rational && denominator && numerator))
        return NULL;

    ret->ops->free_private(ret);

    ret_rational->numerator = move((void **)&numerator);
    ret_rational->denominator = move((void **)&denominator);
    ret_rational->sign = sign;

    ret->private_data = move((void **)&ret_rational);

    return ret;
}
/*
 * @brief create rational number based on int args
 * @note this function takes two arguments, the signature is weird because it
 * needs to fit in the generic API
 */
struct number *rational_from_number(struct number **n, ...)
{
    va_list ap;
    struct number **tmp_arg = 0;
    struct number *ret = rational_new();
    struct rational *ret_rational = calloc(1, sizeof(*ret_rational));
    _free_custom_number_ struct number *denominator = NULL;
    _free_custom_number_ struct number *numerator = move((void **)n);
    bool sign = 0;

    va_start(ap, n);
    tmp_arg = va_arg(ap, struct number **);
    va_end(ap);

    denominator = move((void **)tmp_arg);

    sign = (generic_get_sign(denominator)) ^ (generic_get_sign(denominator));

    if(generic_get_sign(numerator))
        generic_flip_sign(numerator);
    if(generic_get_sign(denominator))
        generic_flip_sign(denominator);

    if(!(ret && ret_rational && denominator && numerator))
        return NULL;

    ret->ops->free_private(ret);

    ret_rational->numerator = move((void **)&numerator);
    ret_rational->denominator = move((void **)&denominator);
    ret_rational->sign = sign;

    ret->private_data = move((void **)&ret_rational);

    return ret;
}
struct number *rational_from_str(char *);
struct number *rational_clone(struct number *self)
{
    struct rational *self_rational = self->private_data;
    struct number *ret = rational_new();
    struct rational *ret_rational = NULL;
    _free_custom_number_ struct number *numerator =
        generic_clone(self_rational->numerator);
    _free_custom_number_ struct number *denominator =
        generic_clone(self_rational->denominator);

    if(!(ret && denominator && numerator))
        return NULL;

    ret_rational = ret->private_data;
    ret_rational->numerator = move((void **)&numerator);
    ret_rational->denominator = move((void **)&denominator);
    ret_rational->sign = self_rational->sign;

    ret->private_data = move((void **)&ret_rational);

    return ret;
}
int rational_print(FILE *stream, struct number *self)
{
    int ret = 0;
    struct rational *self_rational = self->private_data;
    struct number *numerator = self_rational->numerator;
    struct number *denominator = self_rational->denominator;

    printf("(");
    ret += numerator->ops->print(stream, numerator);
    printf(")");
    printf(" / ");
    printf("(");
    ret += denominator->ops->print(stream, denominator);
    printf(")");

    return ret;
}

void _free_denominator(struct rational *self)
{
    self->denominator->ops->free_private(self->denominator);
    free(self->denominator);
    self->denominator = NULL;
}

int _turn_to_common_denominator(struct number *first, struct number *second)
{
    int err = 0;
    struct rational *first_rational = first->private_data;
    struct number *first_numerator = first_rational->numerator;
    struct number *first_denominator = first_rational->denominator;
    struct rational *second_rational = second->private_data;
    struct number *second_numerator = second_rational->numerator;
    struct number *second_denominator = second_rational->denominator;
    _free_custom_number_ struct number *common_denominator =
        generic_clone(first_denominator);

    if(!common_denominator)
        return 1;

    err = generic_mul(common_denominator, second_denominator);
    if(err)
        return err;

    err = generic_mul(first_numerator, second_denominator);
    if(err)
        return err;

    err = generic_mul(second_numerator, first_denominator);
    if(err)
        return err;

    _free_denominator(second_rational);
    second_rational->denominator = generic_clone(common_denominator);
    if(!second_rational->denominator)
        return 1;
    _free_denominator(first_rational);
    first_rational->denominator = generic_clone(common_denominator);
    if(!first_rational->denominator)
        return 1;

    return err;
}

int rational_add(struct number *first, struct number *second)
{
    int err = 0;
    struct number *clone_second = generic_clone(second);
    struct rational *first_rational = first->private_data;
    struct number *first_numerator = first_rational->numerator;

    if(!clone_second)
        return 1;

    struct rational *clone_second_rational = clone_second->private_data;
    struct number *clone_second_numerator = clone_second_rational->numerator;

    err = _turn_to_common_denominator(first, clone_second);
    if(err)
        return err;

    err = generic_add(first_numerator, clone_second_numerator);
    if(err)
        return err;

    err = _simplify(first);

    return err;
}
int rational_sub(struct number *first, struct number *second)
{
    int err = 0;
    struct number *clone_second = generic_clone(second);
    struct rational *first_rational = first->private_data;
    struct number *first_numerator = first_rational->numerator;

    if(!clone_second)
        return 1;

    struct rational *clone_second_rational = clone_second->private_data;
    struct number *clone_second_numerator = clone_second_rational->numerator;

    err = _turn_to_common_denominator(first, clone_second);
    if(err)
        return err;

    err = generic_sub(first_numerator, clone_second_numerator);

    if(err)
        return err;

    err = _simplify(first);

    return err;
}
int rational_mul(struct number *first, struct number *second)
{
    int err = 0;
    struct rational *first_rational = first->private_data;
    struct number *first_numerator = first_rational->numerator;
    struct number *first_denominator = first_rational->denominator;
    struct rational *second_rational = second->private_data;
    struct number *second_numerator = second_rational->numerator;
    struct number *second_denominator = second_rational->denominator;
    bool numerator_sign =
        generic_get_sign(first_numerator) ^ generic_get_sign(second_numerator);
    bool denominator_sign = generic_get_sign(first_denominator) ^
                            generic_get_sign(second_denominator);
    bool sign = numerator_sign ^ denominator_sign;

    err = generic_mul(first_numerator, second_numerator);
    if(err)
        return err;
    err = generic_mul(first_denominator, second_denominator);

    if(numerator_sign)
        generic_flip_sign(first_numerator);
    if(denominator_sign)
        generic_flip_sign(first_denominator);
    first_rational->sign = sign;

    if(err)
        return err;

    err = _simplify(first);

    return err;
}
int rational_div(struct number *first, struct number *second)
{
    int err = 0;
    struct rational *first_rational = first->private_data;
    struct number *first_numerator = first_rational->numerator;
    struct number *first_denominator = first_rational->denominator;
    struct rational *second_rational = second->private_data;
    struct number *second_numerator = second_rational->numerator;
    struct number *second_denominator = second_rational->denominator;
    bool numerator_sign = generic_get_sign(first_numerator) ^
                          generic_get_sign(second_denominator);
    bool denominator_sign = generic_get_sign(first_denominator) ^
                            generic_get_sign(second_numerator);
    bool sign = numerator_sign ^ denominator_sign;

    err = generic_mul(first_numerator, second_denominator);
    if(err)
        return err;
    err = generic_mul(first_denominator, second_numerator);

    if(numerator_sign)
        generic_flip_sign(first_numerator);
    if(denominator_sign)
        generic_flip_sign(first_denominator);
    first_rational->sign = sign;

    if(err)
        return err;

    err = _simplify(first);

    return err;
}

static int _simplify(struct number *self)
{
    int err = 0;
    struct rational *self_rational = self->private_data;
    struct number *self_numerator = self_rational->numerator;
    struct number *self_denominator = self_rational->denominator;
    _free_custom_number_ struct number *gcd =
        generic_gcd(self_numerator, self_denominator);

    if(!gcd)
        return 1;

    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "before simplification: ");
        self->ops->print(stdout, self);
        puts("");
    }

    err = generic_div(self_numerator, gcd);
    if(err)
        return err;
    err = generic_div(self_denominator, gcd);

    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "after simplification: ");
        self->ops->print(stdout, self);
        puts("");
    }

    return err;
}

int rational_flip_sign(struct number *self)
{
    struct rational *self_rational = self->private_data;
    self_rational->sign = !self_rational->sign;
    return 0;
}
int rational_get_sign(struct number *self)
{
    struct rational *self_rational = self->private_data;
    return self_rational->sign;
}
int rational_is_zero(struct number *self)
{
    struct rational *self_rational = self->private_data;
    return generic_is_zero(self_rational->numerator) &&
           (!generic_is_zero(self_rational->denominator));
}
int rational_cmp(struct number *first, struct number *second);
int rational_u_cmp(struct number *first, struct number *second);

int rational_to_int(struct number *self);

void rational_free_private(struct number *self)
{
    struct rational *self_rational = self->private_data;
    generic_free(&self_rational->numerator);
    generic_free(&self_rational->denominator);
    free(self_rational);
}

struct number_type_ops rational_ops = {
    .type = NUMBER_TYPE_RATIONAL,

    .custom_new = rational_new,
    .from_int = rational_from_int,
    .from_number = rational_from_number,
    // .from_str = rational_from_str,
    .clone = rational_clone,

    .print = rational_print,

    // .cmp = rational_cmp,
    // .u_cmp = rational_u_cmp,

    .add = rational_add,
    .sub = rational_sub,
    .mul = rational_mul,
    .div = rational_div,

    .flip_sign = rational_flip_sign,
    .get_sign = rational_get_sign,
    .is_zero = rational_is_zero,

    .to_arr_len = _RESERVERD_NUM_SIZE,

    .free_private = rational_free_private,
};

void __attribute__((constructor)) _custom_numbers_rational_init()
{
    logger(LOG_DEBUG, stdout, "registering rational\n");
    register_number_type_ops(&rational_ops);
}
