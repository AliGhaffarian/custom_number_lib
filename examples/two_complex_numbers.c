#include <custom_numbers/complex.h>
#include <custom_numbers/custom_numbers.h>
#include <custom_numbers/generic.h>
#include <custom_numbers/logger.h>

int main()
{
    custom_numbers_init();
    current_log_level = LOG_DEBUG;

    _free_custom_number_ struct number *one =
        make_number_from_two_ints(NUMBER_TYPE_COMPLEX, 1, 3);
    _free_custom_number_ struct number *two =
        make_number_from_two_ints(NUMBER_TYPE_COMPLEX, 2, 1);
    _free_custom_number_ struct number *three = generic_clone(one);
    generic_add(three, two);

    three->ops->print(stdout, three);
    puts("");
    generic_mul(three, two);

    puts("");

    three->ops->print(stdout, three);
    puts("");
}
