/* math -- instructions for Mach */

#include "flincl.h"
#include "math.h"

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

/* get_long -- get an integer value from FVal,
 *  assumes that p is not NULL
 */
        

void Mach::oper_truncate()
{
    FUnion p = read_tos();
    long pval;
    if (p.node) {
        if (p.node->has_tag(TAG_LONG)) {
            // do nothing
        } else if (p.node->has_tag(TAG_DOUBLE)) {
            pval = (long) p.fdouble->get_double();
            pop();
            push(FLong::make(pval, this));
        } else not_a_number();
    } else not_a_number();
}


void Mach::oper_float()
{
    FUnion p = read_tos();
    int64 pval;
    if (p.node) {
        if (p.flong->test_long(&pval)) {
            replace(FDouble::make((double) pval, this));
        } else if (p.node->has_tag(TAG_DOUBLE)) {
            // do nothing
        } else not_a_number();
    } else not_a_number();
}


void Mach::oper_round()
{
    FUnion p = read_tos();
    int64 pval;
    if (p.node) {
        if (p.node->has_tag(TAG_LONG)) {
            // do nothing
        } else if (p.node->has_tag(TAG_DOUBLE)) {
            pval = (long) (p.fdouble->get_double() + 0.5);
            replace(FLong::make(pval, this));
        } else not_a_number();
    } else not_a_number();
}


void Mach::oper_inc()
{
    FUnion p = read_tos();
	FVal newp;
    int64 pval;
    if (p.node) {
        if (p.flong->test_long(&pval)) {
            newp = FLong::make(pval + 1, this);
        } else if (p.node->has_tag(TAG_DOUBLE)) {
            newp = FDouble::make(p.fdouble->get_double() + 1.0, this);
        } else {
            newp = NULL;
            not_a_number();
        }
    }
    replace(newp);
}


void Mach::oper_dec()
{
    FUnion p = read_tos();
	FVal newp = NULL;
    int64 pval;
    if (p.node) {
        if (p.flong->test_long(&pval)) {
            newp = FLong::make(pval - 1, this);
        } else if (p.node->has_tag(TAG_DOUBLE)) {
            newp = FDouble::make(p.fdouble->get_double() - 1.0, this);
        } else {
            not_a_number();
        }
    }
    replace(newp);
}

#define abs64(x) ((x) < 0 ? -(x) : x)

void Mach::oper_abs()
{
    FUnion p = read_tos();
	FVal newp = NULL;
    int64 pval;
    if (p.node) {
        if (p.flong->test_long(&pval)) {
            newp = FLong::make(abs64(pval), this);
        } else if (p.node->has_tag(TAG_DOUBLE)) {
			double d = p.fdouble->get_double();
			if (d < 0) d = -d;
            newp = FDouble::make(d, this);
        } else {
            not_a_number();
        }
    }
    replace(newp);
}


void Mach::oper_sin()
{
    FDouble_ptr p = read_tos().fdouble;
	double d;
    if (p->test_double(&d)) {
        replace(FDouble::make(sin(d), this));
    } else not_a_float();
}


void Mach::oper_cos()
{
    FDouble_ptr p = read_tos().fdouble;
	double d;
    if (p->test_double(&d)) {
        replace(FDouble::make(cos(d), this));
    } else not_a_float();
}


void Mach::oper_tan()
{
    FDouble_ptr p = read_tos().fdouble;
	double d;
    if (p->test_double(&d)) {
        replace(FDouble::make(tan(d), this));
    } else not_a_float();
}


void Mach::oper_exp()
{
    FDouble_ptr p = read_tos().fdouble;
	double d;
    if (p->test_double(&d)) {
        replace(FDouble::make(exp(d), this));
    } else not_a_float();
}


void Mach::oper_sqrt()
{
    FDouble_ptr p = read_tos().fdouble;
	double d;
    if (p->test_double(&d)) {
        replace(FDouble::make(sqrt(d), this));
    } else not_a_float();
}


void Mach::oper_lognot()
{
    FLong_ptr p = read_tos().flong;
	int64 i;
    if (p->test_long(&i)) {
        replace(FLong::make(~i, this));
    } else not_an_integer();
}


void Mach::oper_minus()
{
    FUnion p = read_tos();
	FVal newp;
    int64 i;
	double d;
    if (p.node) {
        if (p.flong->test_long(&i)) {
            newp = FLong::make(-i, this);
        } else if (p.fdouble->test_double(&d)) {
            newp = FDouble::make(-d, this);
        } else {
            newp = NULL;
            not_a_number();
        }
    }
    replace(newp);
}


void Mach::oper_bool2int()
{
	Symbol * p = read_tos().symbol;
	if (p == NULL) replace(FLong::make(0, this));
	else if (p == s_true) replace(FLong::make(1, this));
	else not_a_boolean();
}


void Mach::oper_int2bool()
{
    FLong_ptr p = read_tos().flong;
	int64 i;
    if (p->test_long(&i)) {
        replace(i ? s_true : NULL);
    } else not_an_integer();
}


/* BINARY OPERATORS */


void Mach::oper_add()
{
    FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 p1val, p2val;
    FVal res = NULL;
    if (p1.node && p2.node) {
        if (p1.flong->test_long(&p1val)) {
            if (p2.flong->test_long(&p2val)) {
                res = FLong::make(p1val + p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::make(p1val + p2.fdouble->get_double(), this);
            } else not_a_number();
        } else if (p1.node->has_tag(TAG_DOUBLE)) {
            if (p2.flong->test_long(&p2val)) {
                res = FDouble::make(p1.fdouble->get_double() + p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::make(p1.fdouble->get_double() + p2.fdouble->get_double(), this);
            } else not_a_number();
        } else if (p1.node->has_tag(TAG_ARRAY) && p2.node->has_tag(TAG_ARRAY)) {
            res = p1.array->concatenate(p2.array, this);
        } else if (p1.node->has_tag(TAG_STRING) && p2.node->has_tag(TAG_STRING)) {
            res = p1.fstring->concatenate(p2.fstring, this);
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Mach::oper_sub()
{
    FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 p1val, p2val;
    FVal res = NULL;
    if (p1.node && p2.node) {
        if (p1.flong->test_long(&p1val)) {
            if (p2.flong->test_long(&p2val)) {
                res = FLong::make(p1val - p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::make(p1val - p2.fdouble->get_double(), this);
            } else not_a_number();
        } else if (p1.node->has_tag(TAG_DOUBLE)) {
            if (p2.flong->test_long(&p2val)) {
                res = FDouble::make(p1.fdouble->get_double() - p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::make(p1.fdouble->get_double() - 
                                          p2.fdouble->get_double(), this);
            } else not_a_number();
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Mach::oper_mul()
{
    FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 p1val, p2val;
    FVal res = NULL;
    if (p1.node && p2.node) {
        if (p1.flong->test_long(&p1val)) {
            if (p2.flong->test_long(&p2val)) {
                res = FLong::make(p1val * p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::make(p1val * p2.fdouble->get_double(), this);
            } else not_a_number();
        } else if (p1.node->has_tag(TAG_DOUBLE)) {
            if (p2.flong->test_long(&p2val)) {
                res = FDouble::make(p1.fdouble->get_double() * p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::make(p1.fdouble->get_double() * 
                                          p2.fdouble->get_double(), this);
            } else not_a_number();
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Mach::oper_power()
{
    FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 l;
    double d1, d2;
    FVal res = NULL;
    if (p1.node && p2.node) {
        if (p1.flong->test_long(&l)) {
            d1 = (double) l;
        } else if (p1.node->has_tag(TAG_DOUBLE)) {
            d1 = p1.fdouble->get_double();
        } else {
            not_a_number();
            return;
        }
        if (p2.flong->test_long(&l)) {
            d2 = (double) l;
        } else if (p2.node->has_tag(TAG_DOUBLE)) {
            d2 = p2.fdouble->get_double();
        } else {
            not_a_number();
            return;
        }
        pop();
        replace(FDouble::make(pow(d1, d2), this));
    } else not_a_number();
}


void Mach::oper_div()
{
    FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 p1val, p2val;
    FVal res = NULL;
    if (p1.node && p2.node) {
        if (p1.flong->test_long(&p1val)) {
            if (p2.flong->test_long(&p2val)) {
                res = FLong::make(p1val / p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::make(p1val / p2.fdouble->get_double(), this);
            } else not_a_number();
        } else if (p1.node->has_tag(TAG_DOUBLE)) {
            if (p2.flong->test_long(&p2val)) {
                res = FDouble::make(p1.fdouble->get_double() / p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::make(p1.fdouble->get_double() / 
                                          p2.fdouble->get_double(), this);
            } else not_a_number();
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Mach::oper_rem()
{
    FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 p1val, p2val;
    FVal res = NULL;
    if (p1.flong->test_long(&p1val) && p2.flong->test_long(&p2val)) {
        res = FLong::make(p1val % p2val, this);
		pop();
		replace(res);
		return;
	}
	not_a_number();
    pop();
}


void Mach::oper_min()
{
	FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 i1, i2;
	double d1, d2;
    FVal res = NULL;
    if (p1.flong->test_long(&i1)) {
        if (p2.flong->test_long(&i2)) {
            res = FLong::make(min(i1, i2), this);
        } else if (p2.fdouble->test_double(&d2)) {
            res = FDouble::make(min(i1, d2), this);
        } else not_a_number();
    } else if (p1.fdouble->test_double(&d1)) {
        if (p2.flong->test_long(&i2)) {
            res = FDouble::make(min(d1, i2), this);
        } else if (p2.fdouble->test_double(&d2)) {
            res = FDouble::make(min(d1, d2), this);
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Mach::oper_max()
{
	FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 i1, i2;
	double d1, d2;
    FVal res = NULL;
    if (p1.flong->test_long(&i1)) {
        if (p2.flong->test_long(&i2)) {
            res = FLong::make(max(i1, i2), this);
        } else if (p2.fdouble->test_double(&d2)) {
            res = FDouble::make(max(i1, d2), this);
        } else not_a_number();
    } else if (p1.fdouble->test_double(&d1)) {
        if (p2.flong->test_long(&i2)) {
            res = FDouble::make(max(d1, i2), this);
        } else if (p2.fdouble->test_double(&d2)) {
            res = FDouble::make(max(d1, d2), this);
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}

void Mach::oper_logand()
{
	FLong_ptr p1 = read_next_tos().flong;
	FLong_ptr p2 = read_tos().flong;
	int64 i1, i2;
	FVal res = NULL;
	if (p1->test_long(&i1) && p2->test_long(&i2)) {
		res = FLong::make((long) (i1 & i2), this);
	}
	pop();
	replace(res);
}


void Mach::oper_rshift()
{
	FLong_ptr p1 = read_next_tos().flong;
	FLong_ptr p2 = read_tos().flong;
	int64 i1, i2;
	FVal res = NULL;
	if (p1->test_long(&i1) && p2->test_long(&i2)) {
		res = FLong::make(i1 >> i2, this);
	} else p1->get_long_error(this);
	pop();
	replace(res);
}


void Mach::oper_lshift()
{
	FLong_ptr p1 = read_next_tos().flong;
	FLong_ptr p2 = read_tos().flong;
	int64 i1, i2;
	FVal res = NULL;
	if (p1->test_long(&i1) && p2->test_long(&i2)) {
		res = FLong::make(i1 << i2, this);
	} else p1->get_long_error(this);
	pop();
	replace(res);
}




void Mach::oper_logior()
{
	FLong_ptr p1 = read_next_tos().flong;
	FLong_ptr p2 = read_tos().flong;
	int64 i1, i2;
	FVal res = NULL;
	if (p1->test_long(&i1) && p2->test_long(&i2)) {
		res = FLong::make((long) (i1 | i2), this);
	}
	pop();
	replace(res);
}


void Mach::oper_logxor()
{
	FLong_ptr p1 = read_next_tos().flong;
	FLong_ptr p2 = read_tos().flong;
	int64 i1, i2;
	FVal res = NULL;
	if (p1->test_long(&i1) && p2->test_long(&i2)) {
		res = FLong::make(i1 ^ i2, this);
	}
	pop();
	replace(res);
}


void Mach::oper_less()
{
	FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 i1, i2;
	double d1, d2;
    FVal res = NULL;
    if (p1.flong->test_long(&i1)) {
        if (p2.flong->test_long(&i2)) {
            if (i1 < i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (i1 < d2) res = s_true; 
        } else not_a_number();
    } else if (p1.fdouble->test_double(&d1)) {
        if (p2.flong->test_long(&i2)) {
            if (d1 < i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (d1 < d2) res = s_true; 
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Mach::oper_lesseql()
{
	FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 i1, i2;
	double d1, d2;
    FVal res = NULL;
    if (p1.flong->test_long(&i1)) {
        if (p2.flong->test_long(&i2)) {
            if (i1 <= i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (i1 <= d2) res = s_true; 
        } else not_a_number();
    } else if (p1.fdouble->test_double(&d1)) {
        if (p2.flong->test_long(&i2)) {
            if (d1 <= i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (d1 <= d2) res = s_true; 
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Mach::oper_noteql()
{
	FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 i1, i2;
	double d1, d2;
    FVal res = NULL;
    if (p1.flong->test_long(&i1)) {
        if (p2.flong->test_long(&i2)) {
            if (i1 != i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (i1 != d2) res = s_true; 
        } else not_a_number();
    } else if (p1.fdouble->test_double(&d1)) {
        if (p2.flong->test_long(&i2)) {
            if (d1 != i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (d1 != d2) res = s_true; 
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Mach::oper_gtreql()
{
	FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 i1, i2;
	double d1, d2;
    FVal res = NULL;
    if (p1.flong->test_long(&i1)) {
        if (p2.flong->test_long(&i2)) {
            if (i1 >= i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (i1 >= d2) res = s_true; 
        } else not_a_number();
    } else if (p1.fdouble->test_double(&d1)) {
        if (p2.flong->test_long(&i2)) {
            if (d1 >= i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (d1 >= d2) res = s_true; 
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Mach::oper_gtr()
{
	FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 i1, i2;
	double d1, d2;
    FVal res = NULL;
    if (p1.flong->test_long(&i1)) {
        if (p2.flong->test_long(&i2)) {
            if (i1 > i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (i1 > d2) res = s_true; 
        } else not_a_number();
    } else if (p1.fdouble->test_double(&d1)) {
        if (p2.flong->test_long(&i2)) {
            if (d1 > i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (d1 > d2) res = s_true; 
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}



/* NO OPERANDS */
void Mach::oper_random() { illegal_instruction(); }


