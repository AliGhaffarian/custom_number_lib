#include <custom_numbers/builtin_types.h>
#include <custom_numbers/custom_numbers.h>
#include <custom_numbers/logger.h>

int main()
{
    custom_numbers_init();
    current_log_level = LOG_DEBUG;
    _free_custom_number_ struct number *one =
        make_number_from_int(NUMBER_TYPE_INT, 1);
    _free_custom_number_ struct number *two =
        make_number_from_int(NUMBER_TYPE_INT, 2);
    _free_custom_number_ struct number *three = generic_clone(one);
    generic_add(three, two);

    three->ops->print(stdout, three);
    puts("");

    _free_custom_number_ struct number *six = generic_clone(two);
    generic_mul(six, three);
    six->ops->print(stdout, six);
}
