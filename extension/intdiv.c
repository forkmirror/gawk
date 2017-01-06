#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "gawkapi.h"

#ifdef HAVE_MPFR
#include <gmp.h>
#include <mpfr.h>
#endif

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t *ext_id;
static const char *ext_version = "intdiv extension: version 1.0";

int plugin_is_GPL_compatible;

static double
double_to_int(double d)
{
	if (d >= 0)
		d = floor(d);
	else
		d = ceil(d);
	return d;
}

static void
array_set_number(awk_array_t array, const char *sub, size_t sublen, double num)
{
	awk_value_t index, tmp;

	set_array_element(array, make_const_string(sub, sublen, & index), make_number(num, & tmp));
}

#ifdef HAVE_MPFR

static mpz_ptr
mpz_conv(const awk_value_t *arg, mpz_ptr tmp)
{
	switch (arg->num_type) {
	case AWK_NUMBER_TYPE_MPZ:
		return arg->num_ptr;
	case AWK_NUMBER_TYPE_MPFR:
		if (! mpfr_number_p(arg->num_ptr))
			return NULL;
		mpz_init(tmp);
		mpfr_get_z(tmp, arg->num_ptr, MPFR_RNDZ);
		return tmp;
	case AWK_NUMBER_TYPE_DOUBLE:	/* can this happen? */
		mpz_init(tmp);
		mpz_set_d(tmp, double_to_int(arg->num_value));
		return tmp;
	default:	/* should never happen */
		fatal(ext_id, _("intdiv: invalid numeric type `%d'"), arg->num_type);
		return NULL;
	}
}

static void
array_set_mpz(awk_array_t array, const char *sub, size_t sublen, mpz_ptr num)
{
	awk_value_t index, tmp;

	set_array_element(array, make_const_string(sub, sublen, & index), make_number_mpz(num, & tmp));
}

#endif

/* do_intdiv --- do integer division, return quotient and remainder in dest array */

/*
 * We define the semantics as:
 * 	numerator = int(numerator)
 *	denominator = int(denonmator)
 *	quotient = int(numerator / denomator)
 *	remainder = int(numerator % denomator)
 */

static awk_value_t *
do_intdiv(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t nv, dv, array_param;
	awk_array_t array;

	if (! get_argument(0, AWK_NUMBER, & nv)) {
		warning(ext_id, _("intdiv: first argument must be numeric"));
		return make_number(-1, result);
	}
	if (! get_argument(1, AWK_NUMBER, & dv)) {
		warning(ext_id, _("intdiv: second argument must be numeric"));
		return make_number(-1, result);
	}
	if (! get_argument(2, AWK_ARRAY, & array_param)) {
		warning(ext_id, _("intdiv: third argument must be an array"));
		return make_number(-1, result);
	}
	array = array_param.array_cookie;
	clear_array(array);

#ifdef HAVE_MPFR
	if (nv.num_type == AWK_NUMBER_TYPE_DOUBLE && dv.num_type == AWK_NUMBER_TYPE_DOUBLE) {
#endif
		/* regular precision */
		double num, denom, quotient, remainder;

		num = double_to_int(nv.num_value);
		denom = double_to_int(dv.num_value);

		if (denom == 0.0)
			fatal(ext_id, _("intdiv: division by zero attempted"));

		quotient = double_to_int(num / denom);
#ifdef HAVE_FMOD
		remainder = fmod(num, denom);
#else	/* ! HAVE_FMOD */
		(void) modf(num / denom, & remainder);
		remainder = num - remainder * denom;
#endif	/* ! HAVE_FMOD */
		remainder = double_to_int(remainder);

		array_set_number(array, "quotient", 8, quotient);
		array_set_number(array, "remainder", 9, remainder);
#ifdef HAVE_MPFR
	}
	else {
		/* extended precision */
		mpz_ptr numer, denom;
		mpz_t numer_tmp, denom_tmp;
		mpz_ptr quotient, remainder;

		/* convert numerator and denominator to integer */
		if (!(numer = mpz_conv(&nv, numer_tmp))) {
			warning(ext_id, _("intdiv: numerator is not finite"));
			return make_number(-1, result);
		}
		if (!(denom = mpz_conv(&dv, denom_tmp))) {
			warning(ext_id, _("intdiv: denominator is not finite"));
			if (numer == numer_tmp)
				mpz_clear(numer);
			return make_number(-1, result);
		}
		if (mpz_sgn(denom) == 0) {
			warning(ext_id, _("intdiv: division by zero attempted"));
			if (numer == numer_tmp)
				mpz_clear(numer);
			if (denom == denom_tmp)
				mpz_clear(denom);
			return make_number(-1, result);
		}

		/* ask gawk to allocate return values for us */
		quotient = get_mpz_ptr();
		remainder = get_mpz_ptr();

		/* do the division */
		mpz_tdiv_qr(quotient, remainder, numer, denom);

		array_set_mpz(array, "quotient", 8, quotient);
		array_set_mpz(array, "remainder", 9, remainder);

		/* release temporary variables */
		if (numer == numer_tmp)
			mpz_clear(numer);
		if (denom == denom_tmp)
			mpz_clear(denom);
	}
#endif

	return make_number(0, result);
}

static awk_ext_func_t func_table[] = {
	{ "api_intdiv", do_intdiv, 3, 3, awk_false, NULL },
};

/* init_intdiv --- initialization routine */

static awk_bool_t
init_intdiv(void)
{
#ifdef HAVE_MPFR
	check_mpfr_version(intdiv)
#endif
	return awk_true;
}

static awk_bool_t (*init_func)(void) = init_intdiv;

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, intdiv, "")
