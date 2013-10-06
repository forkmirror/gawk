/*
 * long_double.h - routines for AWKLDBL number support in gawk.
 */

/* 
 * Copyright (C) 2012 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


/*
 * XXX:	This is a only a template, and M*U**S***T N*O**T include any headers
 *	nor any reference to the C long double type. Please see long_double.c.
 */


static AWKLDBL *pow2d_table;	/* 2^n table for 0 <= n <= 128 */
static void init_pow2d_table(void);
static AWKLDBL pow2ld__p(int n);

static inline AWKLDBL
pow2ld(unsigned int n)
{
	return n <= 128 ? pow2d_table[n] : pow2ld__p(n);
}

/* Can declare these, since we always use the random shipped with gawk */
extern char *initstate(unsigned long seed, char *state, long n);
extern char *setstate(char *state);
extern long random(void);
extern void srandom(unsigned long seed);

/*
 * Since we supply the version of random(), we know what
 * value to use here.
 */
#define GAWK_RANDOM_MAX 0x7fffffffL


/* exported routines */

static NODE *make_awknum(AWKNUM);
static void free_awkldbl(NODE *tmp);
static int cmp_awkldbls(const NODE *, const NODE *);
static void negate_awkldbl(NODE *);
static NODE *str2awkldbl(char *, char **, int, bool);
static NODE *force_awkldbl(NODE *);
static NODE *format_awkldbl_val(const char *, int, NODE *);
static unsigned long awkldbl_toulong(const NODE *);
static long awkldbl_tolong(const NODE *);
static double awkldbl_todouble(const NODE *);
static uintmax_t awkldbl_touintmax_t(const NODE *);
static int awkldbl_sgn(const NODE *);
static bool awkldbl_isinteger(const NODE *);
static bool awkldbl_isnan(const NODE *);
static bool awkldbl_isinf(const NODE *);
static NODE *awkldbl_copy(const NODE *);
static int format_awkldbl_printf(NODE *, struct format_spec *, struct print_fmt_buf *);
static bool awkldbl_init(bltin_t **);
static NODE *awkldbl_add(const NODE *, const NODE *);
static NODE *awkldbl_sub(const NODE *, const NODE *);
static NODE *awkldbl_mul(const NODE *, const NODE *);
static NODE *awkldbl_div(const NODE *, const NODE *);
static NODE *awkldbl_mod(const NODE *, const NODE *);
static NODE *awkldbl_pow(const NODE *, const NODE *);
static NODE *awkldbl_add_long(const NODE *, long);
static NODE *awkldbl_update_var(NODE *);
static void awkldbl_set_var(const NODE *);
static long awkldbl_increment_var(const NODE *, long);
static void awkldbl_init_vars(void);

static NODE *do_and(int);
static NODE *do_atan2(int);
static NODE *do_compl(int);
static NODE *do_cos(int);
static NODE *do_exp(int);
static NODE *do_int(int);
static NODE *do_log(int);
static NODE *do_lshift(int);
static NODE *do_or(int);
static NODE *do_rand(int);
static NODE *do_rshift(int);
static NODE *do_sin(int);
static NODE *do_sqrt(int);
static NODE *do_srand(int);
static NODE *do_strtonum(int);
static NODE *do_xor(int);

/* private routines */
static int is_ieee_magic_val(const char *val);
static AWKLDBL get_ieee_magic_val(const char *val);
static AWKLDBL calc_exp(AWKLDBL x1, AWKLDBL x2);
static NODE *make_awkldbl(AWKLDBL);

static long MFNR;
static long MNR;

numbr_handler_t awkldbl_hndlr = {
	awkldbl_init,
	NULL,	/* version_str */
	NULL,	/* load_procinfo */
	make_awknum,
	str2awkldbl,
	awkldbl_copy,
	free_awkldbl,
	force_awkldbl,
	negate_awkldbl,
	cmp_awkldbls,
	awkldbl_sgn,
	awkldbl_isinteger,
	awkldbl_isnan,
	awkldbl_isinf,
	format_awkldbl_val,
	format_awkldbl_printf,
	awkldbl_todouble,
	awkldbl_tolong,
	awkldbl_toulong,
	awkldbl_touintmax_t,
	awkldbl_add,
	awkldbl_sub,
	awkldbl_mul,
	awkldbl_div,
	awkldbl_mod,
	awkldbl_pow,
	awkldbl_add_long,
	awkldbl_update_var,
	awkldbl_set_var,
	awkldbl_increment_var,
	awkldbl_init_vars,
};

/* awkldbl_init --- initialization routine */

static bool
awkldbl_init(bltin_t **numbr_bltins)
{
	static bltin_t awkldbl_bltins[] = {
		{ "and",	do_and },
		{ "atan2",	do_atan2 },
		{ "compl",	do_compl },
		{ "cos",	do_cos },
		{ "exp",	do_exp },
		{ "int",	do_int },
		{ "log",	do_log },
		{ "lshift",	do_lshift },
		{ "or",		do_or },
		{ "rand",	do_rand },
		{ "rshift",	do_rshift },
		{ "sin",	do_sin },
		{ "sqrt",	do_sqrt },
		{ "srand",	do_srand },
		{ "strtonum",	do_strtonum },
		{ "xor",	do_xor },
		{ NULL, NULL },
	};

	/* set the numeric value of null string */
	get_long_double(Nnull_string->qnumbr);
	LDBL_VAL(Nnull_string) = LDC(0.0);
	Nnull_string->flags |= (NUMCUR|NUMBER);

	/* initialize TRUE and FALSE nodes */
	false_node = make_awkldbl(LDC(0.0));
	true_node = make_awkldbl(LDC(1.0));
	false_node->flags |= NUMINT;
	true_node->flags |= NUMINT;

	init_pow2d_table();	/* FIXME -- initialize only if needed ? */

	*numbr_bltins = awkldbl_bltins;
	return true;
}

/* awkldbl_toulong --- conversion to unsigned long */

static unsigned long
awkldbl_toulong(const NODE *n)
{
	return LDBL_VAL(n);
}

/* awkldbl_tolong --- conversion to long */

static long
awkldbl_tolong(const NODE *n)
{
	return LDBL_VAL(n);
}

/* awkldbl_todouble --- conversion to double */

static double
awkldbl_todouble(const NODE *n)
{
	return LDBL_VAL(n);
}

/* awkldbl_touintmax_t --- conversion to uintmax_t */

static uintmax_t
awkldbl_touintmax_t(const NODE *n)
{
	return LDBL_VAL(n);
}

/* awkldbl_sgn --- return 1 if number > 0, zero if number == 0, and -1 if number < 0 */

static int
awkldbl_sgn(const NODE *n)
{
	AWKLDBL d = LDBL_VAL(n);
	return (d < LDC(0.0) ? -1 : d > LDC(0.0));
}

/* awkldbl_isinteger --- check if a number is an integer */

static bool
awkldbl_isinteger(const NODE *n)
{
	AWKLDBL d = LDBL_VAL(n);

	if (isnan(d) || isinf(d))
		return false;
	return double_to_int(d) == d;
}

/* awkldbl_isnan --- check if number is NaN */

static bool
awkldbl_isnan(const NODE *n)
{
	return isnan(LDBL_VAL(n));
}

/* awkldbl_isinf --- check if number is infinity */

static bool
awkldbl_isinf(const NODE *n)
{
	return isinf(LDBL_VAL(n));
}

/* negate_awkldbl --- negate number in NODE */

static void
negate_awkldbl(NODE *n)
{
	LDBL_VAL(n) = - LDBL_VAL(n);
}

/* awkldbl_add --- add two numbers */

static NODE *
awkldbl_add(const NODE *t1, const NODE *t2)
{
	return make_awkldbl(LDBL_VAL(t1) + LDBL_VAL(t2));
}

/* awkldbl_sub --- subtract two numbers */

static NODE *
awkldbl_sub(const NODE *t1, const NODE *t2)
{
	return make_awkldbl(LDBL_VAL(t1) - LDBL_VAL(t2));
}

/* awkldbl_mul --- multiply two numbers */

static NODE *
awkldbl_mul(const NODE *t1, const NODE *t2)
{
	return make_awkldbl(LDBL_VAL(t1) * LDBL_VAL(t2));
}

/* awkldbl_add --- quotient of two numbers */

static NODE *
awkldbl_div(const NODE *t1, const NODE *t2)
{
	AWKLDBL d = LDBL_VAL(t2);
	if (d == LDC(0.0))
		fatal(_("division by zero attempted"));
	return make_awkldbl(LDBL_VAL(t1) / d);
}

/* awkldbl_add_long --- add long value to a number */

static NODE *
awkldbl_add_long(const NODE *t1, long n)
{
	return make_awkldbl(LDBL_VAL(t1) + (AWKLDBL) n);
}

/* awkldbl_copy --- copy a number */

static NODE *
awkldbl_copy(const NODE *t1)
{
	return make_awkldbl(LDBL_VAL(t1));
}

/* awkldbl_update_var --- update a special variable from internal variables */

static NODE *
awkldbl_update_var(NODE *var)
{
	NODE *val = var->var_value;
	AWKLDBL d;

	d = LDBL_VAL(val);
	if (var == NR_node) {
		if (MNR == 0 && d != NR) {
			unref(val);
			val = NR_node->var_value = make_awkldbl((AWKLDBL) NR);
		} else if (MNR != 0) {
			unref(val);
			d = ((AWKLDBL) MNR) * LONG_MAX + (AWKLDBL) NR;
			val = var->var_value = make_awkldbl(d);
		}
		return val;
	}

	assert(var == FNR_node);
	if (MFNR == 0 && d != FNR) {
		unref(val);
		val = FNR_node->var_value = make_awkldbl((AWKLDBL) FNR);
	} else if (MFNR != 0) {
		unref(val);
		d = ((AWKLDBL) MFNR) * LONG_MAX + (AWKLDBL) FNR;
		val = var->var_value = make_awkldbl(d);
	}
	return val;
}

/*
 * awkldbl_set_var --- update internal variables for assignment
 *	to a special variable.
 */

static void
awkldbl_set_var(const NODE *var)
{
	NODE *val = var->var_value;
	AWKLDBL d;
	
	if (var == NR_node) {
		d = LDBL_VAL(val);
		MNR = d / LONG_MAX;
		NR = d - ((AWKLDBL) MNR) * LONG_MAX;
	} else if (var == FNR_node) {
		d = LDBL_VAL(val);
		MFNR = d / LONG_MAX;
		FNR = d - ((AWKLDBL) MFNR) * LONG_MAX;
	} else {
		/* PREC and ROUNMODE */
		if (do_lint)
			lintwarn(_("setting `%s' has no effect"),
				var == PREC_node ? "PREC" : "ROUNDMODE");
	}
}

/* awkldbl_increment_var --- increment NR or FNR */

static long
awkldbl_increment_var(const NODE *var, long nr)
{
	if (nr == LONG_MAX - 1) {
		/* increment quotient, set remainder(NR or FNR) to 0 */
		if (var == NR_node)
 			MNR++;
		else /* var == FNR_node */
			MFNR++;
		return 0;
	}
	return ++nr;
}

/* awkldbl_init_vars --- initialize special variables */

static void
awkldbl_init_vars()
{
	unref(PREC_node->var_value);
        PREC_node->var_value = make_awknum(GAWK_LDBL_FRAC_BITS);
	PREC_node->var_value->flags |= NUMINT;
        unref(ROUNDMODE_node->var_value);
        ROUNDMODE_node->var_value = make_string("N", 1);
}

/*
 * make_awkldbl --- allocate a node with defined number;
 *	this routine is not exported. 
 */

static NODE *
make_awkldbl(AWKLDBL x)
{
	NODE *r;
	getnode(r);
	r->type = Node_val;
	get_long_double(r->qnumbr);
	LDBL_VAL(r) = x;
	r->flags = MALLOC|NUMBER|NUMCUR;
	r->valref = 1;
	r->stptr = NULL;
	r->stlen = 0;
#if MBS_SUPPORT
	r->wstptr = NULL;
	r->wstlen = 0;
#endif /* defined MBS_SUPPORT */
	return r;
}

/* make_awknum --- allocate a node with defined AWKNUM */

static NODE *
make_awknum(AWKNUM x)
{
	NODE *r;
	getnode(r);
	r->type = Node_val;
	get_long_double(r->qnumbr);
	LDBL_VAL(r) = (AWKLDBL) x;
	r->flags = MALLOC|NUMBER|NUMCUR;
	r->valref = 1;
	r->stptr = NULL;
	r->stlen = 0;
#if MBS_SUPPORT
	r->wstptr = NULL;
	r->wstlen = 0;
#endif /* defined MBS_SUPPORT */
	return r;
}

/* free_awkldbl --- free all storage allocated for a AWKLDBL */

static void
free_awkldbl(NODE *tmp)
{
	assert((tmp->flags & (NUMBER|NUMCUR)) != 0);
        free_long_double(tmp->qnumbr);
}

/* make_integer --- Convert an integer to a number node. */

static inline NODE *
make_integer(uintmax_t n)
{
#ifdef HAVE_UINTMAX_T

	/* N.B: -- see adjust_uint() in floatcomp.c */

	/* XXX: is this really needed in this case ??? */
	
	if (GAWK_LDBL_FRAC_BITS < CHAR_BIT * sizeof (n)) {
		int i = CHAR_BIT * sizeof (n) - GAWK_LDBL_FRAC_BITS;

		/* strip leading `i' bits */

		n = (n << i) >> i;	/* XXX: equivalent code in floatcomp.c
		                         * generates compiler warning.
		                         */ 
	}
#endif /* HAVE_UINTMAX_T */

	return make_awkldbl(n);
}

/* do_lshift --- perform a << operation */

static NODE *
do_lshift(int nargs)
{
	NODE *s1, *s2;
	uintmax_t uval, ushift, res;
	AWKLDBL val, shift;

	s2 = POP_SCALAR();
	s1 = POP_SCALAR();
	if (do_lint) {
		if ((s1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("lshift: received non-numeric first argument"));
		if ((s2->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("lshift: received non-numeric second argument"));
	}

	(void) force_number(s1);
	(void) force_number(s2);
	val = LDBL_VAL(s1);
	shift = LDBL_VAL(s2);

	if (do_lint) {
		if (val < LDC(0.0) || shift < LDC(0.0))
			lintwarn(_("lshift(%f, %f): negative values will give strange results"),
					(double) val, (double) shift);
		if (double_to_int(val) != val || double_to_int(shift) != shift)
			lintwarn(_("lshift(%f, %f): fractional values will be truncated"),
					(double) val, (double) shift);
		if (shift >= sizeof(uintmax_t) * CHAR_BIT)
			lintwarn(_("lshift(%f, %f): too large shift value will give strange results"),
					(double) val, (double) shift);
	}

	DEREF(s1);
	DEREF(s2);
	uval = (uintmax_t) val;
	ushift = (uintmax_t) shift;
	res = uval << ushift;
	return make_integer(res);
}

/* do_rshift --- perform a >> operation */

static NODE *
do_rshift(int nargs)
{
	NODE *s1, *s2;
	uintmax_t uval, ushift, res;
	AWKLDBL val, shift;

	s2 = POP_SCALAR();
	s1 = POP_SCALAR();
	if (do_lint) {
		if ((s1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("rshift: received non-numeric first argument"));
		if ((s2->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("rshift: received non-numeric second argument"));
	}

	(void) force_number(s1);
	(void) force_number(s2);
	val = LDBL_VAL(s1);
	shift = LDBL_VAL(s2);
	if (do_lint) {
		if (val < LDC(0.0) || shift < LDC(0.0))
			lintwarn(_("rshift(%f, %f): negative values will give strange results"),
					(double) val, (double) shift);
		if (double_to_int(val) != val || double_to_int(shift) != shift)
			lintwarn(_("rshift(%f, %f): fractional values will be truncated"),
					(double) val, (double) shift);
		if (shift >= sizeof(uintmax_t) * CHAR_BIT)
			lintwarn(_("rshift(%f, %f): too large shift value will give strange results"),
					(double) val, (double) shift);
	}

	DEREF(s1);
	DEREF(s2);
	uval = (uintmax_t) val;
	ushift = (uintmax_t) shift;
	res = uval >> ushift;
	return make_integer(res);
}

/* do_and --- perform an & operation */

static NODE *
do_and(int nargs)
{
	NODE *s1;
	uintmax_t res, uval;
	AWKLDBL val;
	int i;

	res = ~0;	/* start off with all ones */
	if (nargs < 2)
		fatal(_("and: called with less than two arguments"));

	for (i = 1; nargs > 0; nargs--, i++) {
		s1 = POP_SCALAR();
		if (do_lint && (s1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("and: argument %d is non-numeric"), i);

		(void) force_number(s1);
		val = LDBL_VAL(s1);
		if (do_lint && val < LDC(0.0))
			lintwarn(_("and: argument %d negative value %g will give strange results"),
					i, (double) val);

		uval = (uintmax_t) val;
		res &= uval;
		DEREF(s1);
	}

	return make_integer(res);
}

/* do_or --- perform an | operation */

static NODE *
do_or(int nargs)
{
	NODE *s1;
	uintmax_t res, uval;
	AWKLDBL val;
	int i;

	res = 0;
	if (nargs < 2)
		fatal(_("or: called with less than two arguments"));

	for (i = 1; nargs > 0; nargs--, i++) {
		s1 = POP_SCALAR();
		if (do_lint && (s1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("or: argument %d is non-numeric"), i);

		(void) force_number(s1);
		val = LDBL_VAL(s1);
		if (do_lint && val < LDC(0.0))
			lintwarn(_("or: argument %d negative value %g will give strange results"),
					i, (double) val);

		uval = (uintmax_t) val;
		res |= uval;

		DEREF(s1);
	}

	return make_integer(res);
}

/* do_xor --- perform an ^ operation */

static NODE *
do_xor(int nargs)
{
	NODE *s1;
	uintmax_t res, uval;
	AWKLDBL val;
	int i;

	if (nargs < 2)
		fatal(_("xor: called with less than two arguments"));

	res = 0;	/* silence compiler warning */
	for (i = 1; nargs > 0; nargs--, i++) {
		s1 = POP_SCALAR();
		if (do_lint && (s1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("xor: argument %d is non-numeric"), i);

		(void) force_number(s1);
		val = LDBL_VAL(s1);
		if (do_lint && val < LDC(0.0))
			lintwarn(_("xor: argument %d negative value %g will give strange results"),
					i, (double) val);

		uval = (uintmax_t) val;
		if (i == 1)
			res = uval;
		else
			res ^= uval;
		DEREF(s1);
	}

	return make_integer(res);
}

/* do_compl --- perform a ~ operation */

static NODE *
do_compl(int nargs)
{
	NODE *tmp;
	AWKLDBL d;
	uintmax_t uval;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("compl: received non-numeric argument"));
	(void) force_number(tmp);
	d = LDBL_VAL(tmp);
	DEREF(tmp);

	if (do_lint) {
		if (d < LDC(0.0))
			lintwarn(_("compl(%f): negative value will give strange results"), (double) d);
		if (double_to_int(d) != d)
			lintwarn(_("compl(%f): fractional value will be truncated"), (double) d);
	}

	uval = (uintmax_t) d;
	uval = ~ uval;
	return make_integer(uval);
}

/* nondec2awkldbl --- convert octal or hex value to long double */

/*
 * Because of awk's concatenation rules and the way awk.y:yylex()
 * collects a number, this routine has to be willing to stop on the
 * first invalid character.
 */

static AWKLDBL
nondec2awkldbl(char *str, size_t len)
{
	AWKLDBL retval = 0.0;
	char save;
	short val;
	char *start = str;

	if (*str == '0' && (str[1] == 'x' || str[1] == 'X')) {
		/*
		 * User called strtonum("0x") or some such,
		 * so just quit early.
		 */
		if (len <= 2)
			return 0.0;

		for (str += 2, len -= 2; len > 0; len--, str++) {
			switch (*str) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				val = *str - '0';
				break;
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
				val = *str - 'a' + 10;
				break;
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				val = *str - 'A' + 10;
				break;
			default:
				goto done;
			}
			retval = (retval * LDC(16.0)) + (AWKLDBL) val;
		}
	} else if (*str == '0') {
		for (; len > 0; len--) {
			if (! isdigit((unsigned char) *str))
				goto done;
			else if (*str == '8' || *str == '9') {
				str = start;
				goto decimal;
			}
			retval = (retval * LDC(8.0)) + (AWKLDBL) (*str - '0');
			str++;
		}
	} else {
decimal:
		save = str[len];
		retval = gawk_strtold(str, NULL);
		str[len] = save;
	}
done:
	return retval;
}


/* do_rand --- do the rand function */

static bool firstrand = true;
/* Some systems require this array to be integer aligned. Sigh. */
#define SIZEOF_STATE 256
static uint32_t istate[SIZEOF_STATE/sizeof(uint32_t)];
static char *const state = (char *const) istate;

/* ARGSUSED */
static NODE *
do_rand(int nargs ATTRIBUTE_UNUSED)
{
	double tmprand;
#define RAND_DIVISOR ((double)GAWK_RANDOM_MAX+1.0)
	if (firstrand) {
		(void) initstate((unsigned) 1, state, SIZEOF_STATE);
		/* don't need to srandom(1), initstate() does it for us. */
		firstrand = false;
		setstate(state);
	}
	/*
	 * Per historical practice and POSIX, return value N is
	 *
	 * 	0 <= n < 1
	 */
 	/*
	 * Date: Wed, 28 Aug 2013 17:52:46 -0700
	 * From: Bob Jewett <jewett@bill.scs.agilent.com>
	 *
 	 * Call random() twice to fill in more bits in the value
 	 * of the double.  Also, there is a bug in random() such
 	 * that when the values of successive values are combined
 	 * like (rand1*rand2)^2, (rand3*rand4)^2,  ...  the
 	 * resulting time series is not white noise.  The
 	 * following also seems to fix that bug. 
 	 *
 	 * The add/subtract 0.5 keeps small bits from filling
 	 * below 2^-53 in the double, not that anyone should be
 	 * looking down there. 
 	 */
 
 	tmprand = 0.5 + ( (random()/RAND_DIVISOR + random()) / RAND_DIVISOR);
	return make_awkldbl( (AWKLDBL) (tmprand - 0.5) );
}

/* do_srand --- seed the random number generator */

static NODE *
do_srand(int nargs)
{
	NODE *tmp;
	static long save_seed = 1;
	long ret = save_seed;	/* SVR4 awk srand returns previous seed */
	AWKLDBL d;

	if (firstrand) {
		(void) initstate((unsigned) 1, state, SIZEOF_STATE);
		/* don't need to srandom(1), we're changing the seed below */
		firstrand = false;
		(void) setstate(state);
	}

	if (nargs == 0)
		srandom((unsigned int) (save_seed = (long) time((time_t *) 0)));
	else {
		tmp = POP_SCALAR();
		if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("srand: received non-numeric argument"));
		(void) force_number(tmp);
		d = LDBL_VAL(tmp);
		srandom((unsigned int) (save_seed = (long) d));
		DEREF(tmp);
	}
	return make_awkldbl(ret);
}

/* str2awkldbl --- create a number node from string */

static NODE *
str2awkldbl(char *str, char **endptr, int base, bool is_integer ATTRIBUTE_UNUSED)
{
	NODE *r;
	AWKLDBL d;

	if (base == 0) {
		/*
		 * special case -- only used to parse input in the debugger?
		 * FIXME: reject ieee specials we don't want and/or use the same
		 * rules when reading numbers from a source file and nuke this case.
		 * Also in double.c.
		 */

		errno = 0;
		d = gawk_strtold(str, endptr);
		if (errno != 0)
			d = LDC(0.0);
	} else {
		if (base == 8 || base == 16)
			d = nondec2awkldbl(str, strlen(str));
		else {
			/* atof for double */
			errno = 0;
			d = gawk_strtold(str, NULL);
			if (errno != 0)
				d = LDC(0.0);
			errno = 0;
		}
	}

	r = make_awkldbl(d);

	/* XXX: is this relevant ? */
	if (d <= INT32_MAX && d >= INT32_MIN && d == (int32_t) d)
		r->flags |= NUMINT;

	return r;
}

/* awkldbl_mod --- remainder from division of two numbers */

static NODE *
awkldbl_mod(const NODE *t1, const NODE *t2)
{
	AWKLDBL d2;

	d2 = LDBL_VAL(t2);
	if (d2 == LDC(0.0))
		fatal(_("division by zero attempted in `%%'"));
	return make_awkldbl(gawk_fmodl(LDBL_VAL(t1), d2));
}

/* awkldbl_pow --- power function */

static NODE *
awkldbl_pow(const NODE *t1, const NODE *t2)
{
	return make_awkldbl(calc_exp(LDBL_VAL(t1), LDBL_VAL(t2)));
}


/*
 * calc_exp_posint --- calculate x^n for positive integral n,
 *	using exponentiation by squaring without recursion.
 */

static AWKLDBL
calc_exp_posint(AWKLDBL x, long n)
{
	AWKLDBL mult = LDC(1.0);

	while (n > 1) {
		if ((n % 2) == 1)
			mult *= x;
		x *= x;
		n /= 2;
	}
	return mult * x;
}

/* calc_exp --- calculate x1^x2 */

static AWKLDBL
calc_exp(AWKLDBL x1, AWKLDBL x2)
{
	long lx = x2;

	if (((AWKLDBL) lx) == x2) {		/* integer exponent */
		if (lx == 0)
			return LDC(1.0);
		return (lx > 0) ? calc_exp_posint(x1, lx)
				: LDC(1.0) / calc_exp_posint(x1, -lx);
	}
	return gawk_powl(x1, x2);
}

/* cmp_awkldbls --- compare two doubles */

static int
cmp_awkldbls(const NODE *t1, const NODE *t2)
{
	/*
	 * This routine is also used to sort numeric array indices or values.
	 * For the purposes of sorting, NaN is considered greater than
	 * any other value, and all NaN values are considered equivalent and equal.
	 * This isn't in compliance with IEEE standard, but compliance w.r.t. NaN
	 * comparison at the awk level is a different issue, and needs to be dealt
	 * with in the interpreter for each opcode seperately.
	 */
	AWKLDBL d1 = LDBL_VAL(t1);
	AWKLDBL d2 = LDBL_VAL(t2);

	if (isnan(d1))
		return ! isnan(d2);
	if (isnan(d2))
		return -1;
	/* don't subtract, in case one or both are infinite */
	if (d1 == d2)
		return 0;
	if (d1 < d2)
		return -1;
	return 1;
}

/* force_awkldbl --- force a value to be numeric */

static NODE *
force_awkldbl(NODE *n)
{
	char *cp;
	char *cpend;
	char save;
	char *ptr;
	unsigned int newflags;

	if ((n->flags & NUMCUR) != 0)
		return n;

	/* all the conditionals are an attempt to avoid the expensive strtod */

	/* Note: only set NUMCUR if we actually convert some digits */

	get_long_double(n->qnumbr);
	LDBL_VAL(n) = LDC(0.0);

	if (n->stlen == 0)
		return n;

	cp = n->stptr;
	/*
	 * 2/2007:
	 * POSIX, by way of severe language lawyering, seems to
	 * allow things like "inf" and "nan" to mean something.
	 * So if do_posix, the user gets what he deserves.
	 * This also allows hexadecimal floating point. Ugh.
	 */
	if (! do_posix) {
		if (isalpha((unsigned char) *cp)) {
			return n;
		} else if (n->stlen == 4 && is_ieee_magic_val(n->stptr)) {
			if ((n->flags & MAYBE_NUM) != 0)
				n->flags &= ~MAYBE_NUM;
			n->flags |= NUMBER|NUMCUR;
			LDBL_VAL(n) = get_ieee_magic_val(n->stptr);
			return n;
		}
		/* else
			fall through */
	}
	/* else not POSIX, so
		fall through */

	cpend = cp + n->stlen;
	while (cp < cpend && isspace((unsigned char) *cp))
		cp++;

	if (   cp == cpend		/* only spaces, or */
	    || (! do_posix		/* not POSIXLY paranoid and */
	        && (isalpha((unsigned char) *cp)	/* letter, or */
					/* CANNOT do non-decimal and saw 0x */
		    || (! do_non_decimal_data && cp[0] == '0'
		        && (cp[1] == 'x' || cp[1] == 'X'))))) {
		return n;
	}

	if ((n->flags & MAYBE_NUM) != 0) {
		newflags = NUMBER;
		n->flags &= ~MAYBE_NUM;
	} else
		newflags = 0;

	if (cpend - cp == 1) {		/* only one character */
		if (isdigit((unsigned char) *cp)) {	/* it's a digit! */
			LDBL_VAL(n) = (AWKLDBL)(*cp - '0');
			n->flags |= newflags;
			n->flags |= NUMCUR;
			if (cp == n->stptr)		/* no leading spaces */
				n->flags |= NUMINT;
		}
		return n;
	}

	if (do_non_decimal_data) {	/* main.c assures false if do_posix */
		errno = 0;
		if (! do_traditional && get_numbase(cp, true) != 10) {
			LDBL_VAL(n) = nondec2awkldbl(cp, cpend - cp);
			n->flags |= NUMCUR;
			ptr = cpend;
			goto finish;
		}
	}

	errno = 0;
	save = *cpend;
	*cpend = '\0';
	LDBL_VAL(n) = gawk_strtold((const char *) cp, & ptr);

	/* POSIX says trailing space is OK for NUMBER */
	while (isspace((unsigned char) *ptr))
		ptr++;
	*cpend = save;
finish:
	if (errno == 0 && ptr == cpend) {
		n->flags |= newflags;
		n->flags |= NUMCUR;
	} else {
		errno = 0;
	}
	return n;
}


/*
 * The following lookup table is used as an optimization in force_string;
 * (more complicated) variations on this theme didn't seem to pay off, but 
 * systematic testing might be in order at some point.
 */
static const char *values[] = {
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
};
#define	NVAL	(sizeof(values)/sizeof(values[0]))

/* format_awkldbl_val --- format a numeric value based on format */

static NODE *
format_awkldbl_val(const char *format, int index, NODE *s)
{
	char buf[BUFSIZ];
	char *sp = buf;
	AWKLDBL ival, d;

	/*
	 * 2/2007: Simplify our lives here. Instead of worrying about
	 * whether or not the value will fit into a long just so we
	 * can use sprintf("%ld", val) on it, always format it ourselves.
	 * The only thing to worry about is that integral values always
	 * format as integers. %.0f does that very well.
	 *
	 * 6/2008: Would that things were so simple. Always using %.0f
	 * imposes a notable performance penalty for applications that
	 * do a lot of conversion of integers to strings. So, we reinstate
	 * the old code, but use %.0f for integral values that are outside
	 * the range of a long.  This seems a reasonable compromise.
	 *
	 * 12/2009: Use <= and >= in the comparisons with LONG_xxx instead of
	 * < and > so that things work correctly on systems with 64 bit integers.
	 */

	d = LDBL_VAL(s);

	if ((s->flags & STRCUR) != 0)
		efree(s->stptr);
	free_wstr(s);

	/* not an integral value, or out of range */
	if ((ival = double_to_int(d)) != d
			|| ival <= (AWKLDBL) LONG_MIN || ival >= (AWKLDBL) LONG_MAX
	) {
		struct format_spec spec;
		struct print_fmt_buf *outb;

		/*
		 * Once upon a time, we just blindly did this:
		 *	sprintf(sp, format, s->numbr);
		 *	s->stlen = strlen(sp);
		 *	s->stfmt = (char) index;
		 * but that's no good if, e.g., OFMT is %s. So we punt,
		 * and just always format the value ourselves.
		 */

		/* XXX: format_spec copied since can be altered in the formatting routine */

		if (ival == d) {
			/* integral value, but outside range of %ld, use %.0f */
			spec = *fmt_list[INT_0f_FMT_INDEX].spec;
			s->stfmt = -1;
		} else {
			assert(fmt_list[index].spec != NULL);	/* or we can use fmt_parse() */
			spec = *fmt_list[index].spec;
			s->stfmt = (char) index;	
		}

		outb = get_fmt_buf();
		format_awkldbl_printf(s, & spec, outb);
		(void) bytes2node(outb, s);
		free_fmt_buf(outb);

		s->stptr[s->stlen] = '\0';
	} else {
		/*
		 * integral value; force conversion to long only once.
		 */
		long num = (long) ival;

		if (num < NVAL && num >= 0) {
			sp = (char *) values[num];
			s->stlen = 1;
		} else {
			(void) sprintf(sp, "%ld", num);
			s->stlen = strlen(sp);
		}
		s->stfmt = -1;
		if ((s->flags & INTIND) != 0) {
			s->flags &= ~(INTIND|NUMBER);
			s->flags |= STRING;
		}

		emalloc(s->stptr, char *, s->stlen + 2, "format_awkldbl_val");
		memcpy(s->stptr, sp, s->stlen + 1);
	}

	s->flags |= STRCUR;
	return s;
}

/* is_ieee_magic_val --- return true for +inf, -inf, +nan, -nan */

static int
is_ieee_magic_val(const char *val)
{
	/*
	 * Avoid strncasecmp: it mishandles ASCII bytes in some locales.
	 * Assume the length is 4, as the caller checks this.
	 */
	return (   (val[0] == '+' || val[0] == '-')
		&& (   (   (val[1] == 'i' || val[1] == 'I')
			&& (val[2] == 'n' || val[2] == 'N')
			&& (val[3] == 'f' || val[3] == 'F'))
		    || (   (val[1] == 'n' || val[1] == 'N')
			&& (val[2] == 'a' || val[2] == 'A')
			&& (val[3] == 'n' || val[3] == 'N'))));
}

/* get_ieee_magic_val --- return magic value for string */

static AWKLDBL
get_ieee_magic_val(const char *val)
{
	static bool first = true;
	static AWKLDBL inf;
	static AWKLDBL nan;
	char *ptr;
	AWKLDBL v;

	v = gawk_strtold(val, & ptr);

	if (val == ptr) { /* Older strtod implementations don't support inf or nan. */
		if (first) {
			first = false;
			nan = gawk_sqrtl(-LDC(1.0));	/* FIXME -- this isn't right */
			inf = -gawk_logl(LDC(0.0));	/* Ditto */
		}
		v = ((val[1] == 'i' || val[1] == 'I') ? inf : nan);
		if (val[0] == '-')
			v = -v;
	}

	return v;
}

/* do_int --- convert double to int for awk */

static NODE *
do_int(int nargs)
{
	NODE *tmp;
	AWKLDBL d;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("int: received non-numeric argument"));
	(void) force_number(tmp);
	d = LDBL_VAL(tmp);
	DEREF(tmp);
	return make_awkldbl(double_to_int(d));
}

/* do_log --- the log function */

static NODE *
do_log(int nargs)
{
	NODE *tmp;
	AWKLDBL d, arg;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("log: received non-numeric argument"));
	(void) force_number(tmp);
	arg = LDBL_VAL(tmp);
	if (arg < LDC(0.0))
		warning(_("log: received negative argument %g"), (double) arg);
	d = gawk_logl(arg);
	DEREF(tmp);
	return make_awkldbl(d);
}

/* do_sqrt --- do the sqrt function */

static NODE *
do_sqrt(int nargs)
{
	NODE *tmp;
	AWKLDBL arg;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("sqrt: received non-numeric argument"));
	(void) force_number(tmp);
	arg = LDBL_VAL(tmp);
	DEREF(tmp);
	if (arg < LDC(0.0))
		warning(_("sqrt: called with negative argument %g"), (double) arg);
	return make_awkldbl(gawk_sqrtl(arg));
}

/* do_exp --- exponential function */

static NODE *
do_exp(int nargs)
{
	NODE *tmp;
	AWKLDBL d, res;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("exp: received non-numeric argument"));
	(void) force_number(tmp);
	d = LDBL_VAL(tmp);
	DEREF(tmp);
	errno = 0;
	res = gawk_expl(d);
	if (errno == ERANGE)
		warning(_("exp: argument %g is out of range"), (double) d);
	return make_awkldbl(res);
}

/* do_atan2 --- do the atan2 function */

static NODE *
do_atan2(int nargs)
{
	NODE *t1, *t2;
	AWKLDBL d1, d2;

	t2 = POP_SCALAR();
	t1 = POP_SCALAR();
	if (do_lint) {
		if ((t1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("atan2: received non-numeric first argument"));
		if ((t2->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("atan2: received non-numeric second argument"));
	}
	(void) force_number(t1);
	(void) force_number(t2);
	d1 = LDBL_VAL(t1);
	d2 = LDBL_VAL(t2);
	DEREF(t1);
	DEREF(t2);
	return make_awkldbl(gawk_atan2l(d1, d2));
}

/* do_sin --- do the sin function */

static NODE *
do_sin(int nargs)
{
	NODE *tmp;
	AWKLDBL d;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("sin: received non-numeric argument"));
	(void) force_number(tmp);
	d = gawk_sinl(LDBL_VAL(tmp));
	DEREF(tmp);
	return make_awkldbl(d);
}

/* do_cos --- do the cos function */

static NODE *
do_cos(int nargs)
{
	NODE *tmp;
	AWKLDBL d;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("cos: received non-numeric argument"));
	(void) force_number(tmp);
	d = gawk_cosl(LDBL_VAL(tmp));
	DEREF(tmp);
	return make_awkldbl(d);
}

/* do_strtonum --- the strtonum function */

static NODE *
do_strtonum(int nargs)
{
	NODE *tmp;
	AWKLDBL d;

	tmp = POP_SCALAR();
	if ((tmp->flags & (NUMBER|NUMCUR)) != 0) {
		(void) force_number(tmp);
		d = LDBL_VAL(tmp);
	} else if (get_numbase(tmp->stptr, use_lc_numeric) != 10)
		d = nondec2awkldbl(tmp->stptr, tmp->stlen);
	else {
		(void) force_number(tmp);
		d = LDBL_VAL(tmp);
	}

	DEREF(tmp);
	return make_awkldbl(d);
}

/* format_awkldbl_prinf --- format a number for (s)printf */

static int
format_awkldbl_printf(NODE *arg, struct format_spec *spec, struct print_fmt_buf *outb)
{
	AWKLDBL tmpval;
	uintmax_t uval;
	bool sgn;
	int i, ii, jj;
	char *chp, *cp;
	char cs1;
	int nc;

	static char stackbuf[64];	/* temporary buffer for integer formatting */ 
	static char *intbuf = stackbuf;
	size_t intbuf_size = 64;

#	define CP		cpbuf_start(outb)
#	define CEND		cpbuf_end(outb)
#	define CPBUF		cpbuf(outb)

	tmpval = LDBL_VAL(arg);
	spec->fill = space_string;
	spec->chbuf = lchbuf;

	cp = CP;
	cs1 = spec->fmtchar;
	switch (cs1) {
	case 'd':
	case 'i':
		if (isnan(tmpval) || isinf(tmpval))
			goto out_of_range;

		tmpval = double_to_int(tmpval);

		/*
		 * ``The result of converting a zero value with a
		 * precision of zero is no characters.''
		 */
		if (spec->have_prec && spec->prec == 0 && tmpval == LDC(0.0)) {
			pr_num_tail(cp, spec->prec, spec, outb);
			return 0;
		}

		if (tmpval < LDC(0.0)) {
			tmpval = -tmpval;
			sgn = true;
		} else {
			if (tmpval == - LDC(0.0))	/* avoid printing -0; XXX:  0.0 == -0.0, but does the job. */
				tmpval = LDC(0.0);
			sgn = false;
		}

		/*
		 * Use snprintf return value to tell if there
		 * is enough room in the buffer or not.
		 */
		while ((i = format_uint_1(intbuf, intbuf_size, tmpval)) >= intbuf_size) {
			if (intbuf == stackbuf)
				intbuf = NULL;
			intbuf_size = i + 1;
			erealloc(intbuf, char *, intbuf_size, "format_tree");
		}

		if (i < 1)
			goto out_of_range;

		chp = & intbuf[i-1];
		ii = jj = 0;
		do {
			tmpbuf_prepend(outb, *chp);
			chp--; i--;
#if defined(HAVE_LOCALE_H)
			if (spec->quote_flag && loc.grouping[ii] && ++jj == loc.grouping[ii]) {
				if (i)	/* only add if more digits coming */
					tmpbuf_prepend(outb, loc.thousands_sep[0]);	/* XXX - assumption it's one char */
				if (loc.grouping[ii+1] == 0)
					jj = 0;		/* keep using current val in loc.grouping[ii] */
				else if (loc.grouping[ii+1] == CHAR_MAX)
					spec->quote_flag= false;
				else {                 
					ii++;
					jj = 0;
				}
			}
#endif
		} while (i > 0);


		/* add more output digits to match the precision */
		if (spec->have_prec) {
			while (CEND - CP < spec->prec)
				tmpbuf_prepend(outb, '0');
		}

		if (sgn)
			tmpbuf_prepend(outb, '-');
		else if (spec->signchar)
			tmpbuf_prepend(outb, spec->signchar);
		/*
		 * When to fill with zeroes is of course not simple.
		 * First: No zero fill if left-justifying.
		 * Next: There seem to be two cases:
		 * 	A '0' without a precision, e.g. %06d
		 * 	A precision with no field width, e.g. %.10d
		 * Any other case, we don't want to fill with zeroes.
		 */
		if (! spec->lj
		    && ((spec->zero_flag && ! spec->have_prec)
			 || (spec->fw == 0 && spec->have_prec)))
			spec->fill = zero_string;
		if (spec->prec > spec->fw)
			spec->fw = spec->prec;
		spec->prec = CEND - CP;
		if (spec->fw > spec->prec && ! spec->lj && spec->fill != space_string
		    && (*CP == '-' || spec->signchar)) {
			bchunk_one(outb, CP);
			CP++;
			spec->prec--;
			spec->fw--;
		}
		cp = CP;

		pr_num_tail(CP, spec->prec, spec, outb);
		return 0;

	case 'X':
		spec->chbuf = Uchbuf;
		/* FALL THROUGH */
	case 'x':
		/* FALL THROUGH */
	case 'u':
		/* FALL THROUGH */
	case 'o':
		/*
		 * ``The result of converting a zero value with a
		 * precision of zero is no characters.''
		 *
		 * If I remember the ANSI C standard, though,
		 * it says that for octal conversions
		 * the precision is artificially increased
		 * to add an extra 0 if # is supplied.
		 * Indeed, in C,
		 * 	printf("%#.0o\n", 0);
		 * prints a single 0.
		 */
	
		if (! spec->alt && spec->have_prec && spec->prec == 0 && tmpval == LDC(0.0)) {
			pr_num_tail(cp, spec->prec, spec, outb);
			return 0;
		}

		if (tmpval < LDC(0.0)) {
			uval = (uintmax_t) (intmax_t) tmpval;
			if ((AWKLDBL)(intmax_t) uval != double_to_int(tmpval))
				goto out_of_range;
		} else {
			uval = (uintmax_t) tmpval;
			if ((AWKLDBL) uval != double_to_int(tmpval))
				goto out_of_range;
		}

		format_nondecimal(uval, spec, outb);
		return 0;

out_of_range:
		/* out of range - emergency use of %g format */
		if (do_lint)
			lintwarn(_("[s]printf: value %g is out of range for `%%%c' format"),
					(double) tmpval, cs1);
		cs1 = 'g';
		goto fmt1;

	case 'F':
#if ! defined(PRINTF_HAS_F_FORMAT) || PRINTF_HAS_F_FORMAT != 1
		cs1 = 'f';
		/* FALL THROUGH */
#endif
	case 'g':
	case 'G':
	case 'e':
	case 'f':
	case 'E':
#ifdef NUMDEBUG
	case 'a':	/* hexadecimal */
	case 'b':	/* MPFR binary format */
#endif
fmt1:
		if (! spec->have_prec)
			spec->prec = DEFAULT_G_PRECISION;

		chksize(outb, spec->fw + spec->prec + 11);	/* 11 == slop */
		cp = CPBUF;	/* XXX:  using the temporary prepend-buffer and
		                 * we know it has enough room (>=11).
                                 */
		*cp++ = '%';
		if (spec->lj)
			*cp++ = '-';
		if (spec->signchar)
			*cp++ = spec->signchar;
		if (spec->alt)
			*cp++ = '#';
		if (spec->zero_flag)
			*cp++ = '0';
		if (spec->quote_flag)
			*cp++ = '\'';

#if defined(LC_NUMERIC)
		if (spec->quote_flag && ! use_lc_numeric)
			setlocale(LC_NUMERIC, "");
#endif

		sprintf(cp, "*.*L%c", cs1);
		while ((nc = format_float_1(buf_end(outb), buf_space(outb), CPBUF,
				(int) spec->fw, (int) spec->prec, tmpval)) >= buf_space(outb))
			chksize(outb, nc + 1);

#if defined(LC_NUMERIC)
		if (spec->quote_flag && ! use_lc_numeric)
			setlocale(LC_NUMERIC, "C");
#endif

		buf_adjust(outb, nc); /* adjust data and free space in output buffer */
		return 0;

	default:
		cant_happen();	
	}

	return -1;
#undef CP
#undef CEND
#undef CPBUF
}


/*--------------- replacement routines for losing systems -----------*/


/* init_pow2d_table --- populate powers of 2 table */

static void
init_pow2d_table()
{
	unsigned i;
	emalloc(pow2d_table, AWKLDBL *, 129 * sizeof (AWKLDBL), "init_pow2d_table");
	pow2d_table[0] = LDC(1.0);
	for (i = 1; i <= 128; i++)
		pow2d_table[i] = pow2d_table[i - 1] * LDC(2.0);
}

/* pow2ld__p --- return 2^n for some integer n >= 0 */

static inline AWKLDBL
pow2ld__p(int n)
{
	AWKLDBL dval = LDC(1.0);
        for (; n > 128; n -= 128)
		dval *= pow2d_table[128];
        return dval * pow2d_table[n];
}

#if defined(GAWK_FMT_INT)

/*
 * gawk_floorl_finite_p --- provide floor() for long double. The double value
 *	must be finite and >= 0. If the 2nd array argument isn't NULL, also split the
 *	integer value into 16 (32) bit chunks.
 */

static AWKLDBL
gawk_floorl_finite_p(AWKLDBL x, gawk_uint_t *chunk)
{
	int high, low, mid;
	AWKLDBL intval = LDC(0.0);

#if	GAWK_LDBL_INT_BITS == 128
	if (x >= pow2ld(113))
#else
	if (x >= pow2ld(64))
#endif
		return -LDC(1.0);

	if (chunk != NULL)
		memset(chunk, '\0', 4 * sizeof (gawk_uint_t));

	/* binary search */
	high = GAWK_LDBL_INT_BITS - 1;
	while (x >= LDC(2.0)) {
		low = 0;
		while (low <= high) {
			mid = (low + high) / 2;
			if (x < pow2ld(mid))
				high = mid - 1;
			else
				low = mid + 1;
		}

		if (x < pow2ld(low)) {
			x -= pow2ld(low - 1);
			intval += pow2ld(low - 1);

			if (chunk != NULL) {
				/*
				 * seperate the 64 (128) bits into 16 (32) bit chunks.
				 *	| D[3]  |  D[2] |  D[1] | D[0]  |
				 *	|<------- x (64/128 bits) ----->|
				 */

#if	GAWK_LDBL_INT_BITS == 128
				if (low <= 32) chunk[0] += (gawk_uint_t) pow2ld(low - 1);
				else if (low <= 64) chunk[1] += (gawk_uint_t) pow2ld(low - 33);
				else if (low <= 96) chunk[2] += (gawk_uint_t) pow2ld(low - 65);
				else chunk[3] += (gawk_uint_t) pow2ld(low - 97);
#else
				if (low <= 16) chunk[0] += (gawk_uint_t) pow2ld(low - 1);
				else if (low <= 32) chunk[1] += (gawk_uint_t) pow2ld(low - 17);
				else if (low <= 48) chunk[2] += (gawk_uint_t) pow2ld(low - 33);
				else chunk[3] += (gawk_uint_t) pow2ld(low - 49);
#endif
			}
		}

		high = low;
	}

	if (x >= LDC(1.0)) {
		if (chunk != NULL)
			chunk[0]++;
		x -= LDC(1.0);
		intval += LDC(1.0);
	}
	return intval;	/* >= 0.0 */
}

/*
 * format_uint_finite_p --- format a AWKLDBL as an unsigned integer. The AWKLDBL value
 *	must be finite and in the range 0 <= x < 2^LDBL_FRAC_BITS.
 */

static int
format_uint_finite_p(char *str, size_t size, AWKLDBL x)
{
	/*
	 * For an explanation of the algorithm, and the derivation for the
	 * magic numbers below, please see:
	 *
	 *  Binary to Decimal Conversion in Limited Precision
	 *  Part of the Arithmetic Tutorial Collection
	 *  by Douglas W. Jones
	 *  THE UNIVERSITY OF IOWA Department of Computer Science
	 *  URL: homepage.cs.uiowa.edu/~jones/bcd/decimal.html
	 */

#if	GAWK_LDBL_INT_BITS == 128
	static gawk_uint_t coeff[] = { 
		1, 4967296, 9551616,
#ifdef	GAWK_INT_IS_LONG_LONG
		3585223950336ULL,
#else
		3585223950336UL,
#endif
		429, 4407370, 3400832,
		184467, 5142643,
		79228162
	};
	static const char format[] = "%7.7u%7.7u%7.7u%7.7u%7.7u";
	gawk_uint_t decimal_base = 10000000;
#else
	static gawk_uint_t coeff[] = {
		1, 5536, 7296, 656,
		6, 9496, 7671,
		42, 4749,
		281
	};
	static const char format[] = "%4.4u%4.4u%4.4u%4.4u%4.4u";
	gawk_uint_t decimal_base = 10000;
#endif
	char *p;
	int len;
	gawk_uint_t d0, d1, d2, d3, d4, q;
	gawk_uint_t chunk[4];

	assert(x >= LDC(0.0));

	assert(size > 35);	/* maximum size ever needed excluding terminating null */

	if (gawk_floorl_finite_p(x, chunk) < LDC(0.0))	/* outside range */
		return -1;

        d0 = chunk[0] + coeff[1] * chunk[1] + coeff[2] * chunk[2] + coeff[3] * chunk[3];
        q = d0 / decimal_base;
        d0 = d0 % decimal_base;

        d1 = q + coeff[4] * chunk[1] + coeff[5] * chunk[2] + coeff[6] * chunk[3];
        q = d1 / decimal_base;
        d1 = d1 % decimal_base;

        d2 = q + coeff[7] * chunk[2] + coeff[8] * chunk[3];
        q = d2 / decimal_base;
        d2 = d2 % decimal_base;

        d3 = q + coeff[9] * chunk[3];
        q = d3 / decimal_base;
        d3 = d3 % decimal_base;

        d4 = q;

	(void) sprintf(str, format, (unsigned) d4, (unsigned) d3,
				(unsigned) d2, (unsigned) d1, (unsigned) d0);

	/* strip leading 0s */
	for (p = str; *(p + 1); p++)	/* don't strip the last 0, the value can be "0" */
		if (*p != '0')
			break;
	len = strlen(p);
	if (p != str)
		memmove(str, p, len + 1);
	return len;
}

#endif
