#include <custom_numbers/bignum.h>
#include <custom_numbers/custom_numbers.h>
#include <custom_numbers/logger.h>

int main()
{
    current_log_level = LOG_DEBUG;
    custom_numbers_init();
    _free_custom_number_ struct number *first_num =
        make_number_from_int(NUMBER_TYPE_BIGNUM, 1);
    _free_custom_number_ struct number *first_deno =
        make_number_from_int(NUMBER_TYPE_BIGNUM, 2);

    _free_custom_number_ struct number *second_num =
        make_number_from_str(NUMBER_TYPE_BIGNUM, "2");
    _free_custom_number_ struct number *second_deno =
        make_number_from_str(NUMBER_TYPE_BIGNUM, "3");

    _free_custom_number_ struct number *first_rational =
        make_number_from_two_numbers(
            NUMBER_TYPE_RATIONAL, &first_num, &first_deno);
    first_rational->ops->print(stdout, first_rational);
    puts("");

    _free_custom_number_ struct number *second_rational =
        make_number_from_two_numbers(
            NUMBER_TYPE_RATIONAL, &second_num, &second_deno);
    second_rational->ops->print(stdout, second_rational);
    puts("");

    generic_add(first_rational, second_rational);
    first_rational->ops->print(stdout, first_rational);
    puts("");
    generic_sub(first_rational, second_rational);
    first_rational->ops->print(stdout, first_rational);
    puts("");
}
