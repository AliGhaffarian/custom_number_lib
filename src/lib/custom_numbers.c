#include <custom_numbers/bignum.h>
#include <custom_numbers/builtin_types.h>
#include <custom_numbers/complex.h>
#include <custom_numbers/custom_numbers.h>
#include <custom_numbers/rational.h>

void custom_numbers_init()
{
    _custom_numbers_builtin_types_init();
    _custom_numbers_complex_init();
    _custom_numbers_bignum_init();
    _custom_numbers_rational_init();
}
