#include <custom_numbers/builtin_types.h>
#include <custom_numbers/complex.h>
#include <custom_numbers/logger.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

struct number *complex_new()
{
    struct number *ret = calloc(1, sizeof(struct number));
    if(ret == NULL)
        return NULL;

    ret->type = NUMBER_TYPE_COMPLEX;
    ret->ops = &complex_ops;

    ret->private_data = calloc(1, sizeof(struct complex));

    if(ret->private_data == NULL) {
        free(ret);
        return NULL;
    }

    return ret;
}

/*
 * @brief create complex number based on int args
 * @note this function takes two arguments, the signature is weird because it
 * needs to fit in the generic API
 */
struct number *complex_from_int(int n, ...)
{
    int tmp_arg = 0;
    va_list ap;
    struct complex *self_value = NULL;
    struct number *ret = complex_new();

    if(ret == NULL)
        return NULL;
    self_value = (struct complex *)ret->private_data;

    self_value->re = make_number_from_int(NUMBER_TYPE_INT, n);

    va_start(ap, n);
    tmp_arg = va_arg(ap, int);
    va_end(ap);

    self_value->img = make_number_from_int(NUMBER_TYPE_INT, tmp_arg);

    if(current_log_level == LOG_DEBUG) {
        logger(LOG_DEBUG, stdout, "from %d and %d:", n, tmp_arg);
        ret->ops->print(stdout, ret);
        puts("");
    }

    return ret;
}

struct number *complex_clone(struct number *self)
{
    struct number *ret = complex_new();
    struct complex *ret_value = (struct complex *)ret->private_data;
    struct complex *self_value = (struct complex *)self->private_data;
    if(ret == NULL)
        return NULL;

    ret_value->img = generic_clone(self_value->img);
    ret_value->re = generic_clone(self_value->re);

    return ret;
}

int complex_add(struct number *first, struct number *second)
{
    int err = 0;
    struct complex *second_value = (struct complex *)second->private_data;
    struct complex *first_value = (struct complex *)first->private_data;

    err = generic_add(first_value->img, second_value->img);
    if(err)
        return err;

    err = generic_add(first_value->re, second_value->re);

    return err;
}

int complex_sub(struct number *first, struct number *second)
{
    int err = 0;
    struct complex *second_value = (struct complex *)second->private_data;
    struct complex *first_value = (struct complex *)first->private_data;

    err = generic_sub(first_value->img, second_value->img);
    if(err)
        return err;

    err = generic_sub(first_value->re, second_value->re);

    return err;
}

int complex_mul(struct number *first, struct number *second)
{
    int err = 0;

    struct complex *first_value = (struct complex *)first->private_data;
    struct complex *second_value = (struct complex *)second->private_data;

    _free_custom_number_ struct number *re1re2 = generic_clone(first_value->re);
    _free_custom_number_ struct number *img1img2 =
        generic_clone(first_value->img);
    _free_custom_number_ struct number *re1img2 =
        generic_clone(first_value->re);
    _free_custom_number_ struct number *img1re2 =
        generic_clone(first_value->img);

    // completing terms
    err = generic_mul(re1re2, second_value->re);
    if(err)
        return err;
    err = generic_mul(img1img2, second_value->img);
    if(err)
        return err;
    err = generic_mul(re1img2, second_value->img);
    if(err)
        return err;
    err = generic_mul(img1re2, second_value->re);
    if(err)
        return err;

    if(current_log_level == LOG_DEBUG) {
        printf("re1re2:");
        re1re2->ops->print(stdout, re1re2);
        puts("");

        printf("img1img2:");
        img1img2->ops->print(stdout, img1img2);
        puts("");

        printf("re1img2:");
        re1img2->ops->print(stdout, re1img2);
        puts("");

        printf("img1re2:");
        img1re2->ops->print(stdout, img1re2);
        puts("");
    }

    err = generic_sub(re1re2, img1img2);
    if(err)
        return err;
    generic_free(&first_value->re);
    first_value->re = generic_clone(re1re2);

    err = generic_add(re1img2, img1re2);
    if(err)
        return err;
    generic_free(&first_value->img);
    first_value->img = generic_clone(re1img2);

    logger(LOG_DEBUG, stdout, "success\n");
    return err;
}

// TODO:
int complex_div(struct number *first, struct number *second) {}

int complex_flip_sign(struct number *self)
{
    int err = 0;
    struct complex *self_value = (struct complex *)self->private_data;
    err = generic_flip_sign(self_value->img);
    if(err)
        return err;
    err = generic_flip_sign(self_value->re);
    if(err)
        return err;
    return 0;
}

int complex_is_zero(struct number *self)
{
    int is_zero = 0;
    struct complex *self_value = (struct complex *)self->private_data;
    is_zero = generic_is_zero(self_value->img);
    if(is_zero)
        return is_zero;

    is_zero |= generic_is_zero(self_value->re);

    return is_zero;
}

void complex_free(struct number **self)
{
    struct complex *self_value = (struct complex *)(*self)->private_data;

    generic_free(&self_value->img);
    generic_free(&self_value->re);

    free((*self)->private_data);
    free(*self);
    *self = NULL;
}

int complex_print(FILE *stream, struct number *self)
{
    int ret = 0;
    struct complex *self_value = (struct complex *)self->private_data;

    ret += self_value->re->ops->print(stdout, self_value->re);
    ret += printf(" + j");
    ret += self_value->img->ops->print(stdout, self_value->img);

    return ret;
}

int complex_to_int(struct number *self)
{
    int ret = 0;
    struct complex *self_value = (struct complex *)self->private_data;
    int converted = 0;

    if(self_value->img->ops->is_zero(self_value->img) != 1) {
        if(current_log_level == LOG_DEBUG) {
            logger(LOG_DEBUG, stdout, "cannot convert (");
            self->ops->print(stdout, self);
            printf(") to int, img is not zero\n");
        }
        return 1;
    }
    if(self_value->re->ops->to[NUMBER_TYPE_INT] == NULL)
        return 1;
    self_value->re->ops->to[NUMBER_TYPE_INT](self_value->re);

copy_re:
    converted = *(int *)&self_value->re->private_data;

    generic_free(&self_value->img);
    generic_free(&self_value->re);
    free(self->private_data);

    *(int *)&self->private_data = converted;

    self->ops = &int_ops;
    self->type = NUMBER_TYPE_INT;

    return 0;
}

struct number_type_ops complex_ops = {
    .type = NUMBER_TYPE_COMPLEX,
    .custom_new = complex_new,
    .from_int = complex_from_int,
    .from_str = NULL,
    .clone = complex_clone,
    .print = complex_print,

    .add = complex_add,
    .sub = complex_sub,
    .mul = complex_mul,
    .div = complex_div,
    .flip_sign = complex_flip_sign,
    .is_zero = complex_is_zero,

    .to =
        {
            [NUMBER_TYPE_INT] = complex_to_int,
        },
    .to_arr_len = _RESERVERD_NUM_SIZE,

    .free = complex_free,
};

void __attribute__((constructor)) _custom_numbers_complex_init()
{
    register_number_type_ops(&complex_ops);
}
