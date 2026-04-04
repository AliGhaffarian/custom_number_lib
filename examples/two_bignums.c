#include <custom_numbers/bignum.h>
#include <custom_numbers/custom_numbers.h>
#include <custom_numbers/logger.h>

int main()
{
    current_log_level = LOG_DEBUG;
    custom_numbers_init();
    _free_custom_number_ struct number *first =
        make_number_from_int(NUMBER_TYPE_BIGNUM, 1);
    first->ops->print(stdout, first);
    puts("");

    _free_custom_number_ struct number *clone_first = generic_clone(first);
    puts("clone of first:");
    clone_first->ops->print(stdout, clone_first);
    puts("");

    _free_custom_number_ struct number *second =
        make_number_from_str(NUMBER_TYPE_BIGNUM, "999");
    second->ops->print(stdout, second);
    puts("");

    _free_custom_number_ struct number *clone_second = generic_clone(second);
    puts("clone of second:");
    clone_second->ops->print(stdout, clone_second);
    puts("");

    generic_add(first, second);

    generic_cmp(second, first);
    generic_cmp(first, second);
    generic_cmp(second, second);

    generic_sub(generic_clone(first), second);
    generic_sub(generic_clone(second), first);

    generic_is_zero(first);

    generic_sub(
        make_number_from_str(NUMBER_TYPE_BIGNUM, "100"),
        make_number_from_str(NUMBER_TYPE_BIGNUM, "1"));
}
