// @@@LICENSE
//
//      Copyright (c) 2009-2013 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// LICENSE@@@

#include "number_validator.h"
#include "number.h"
#include "validation_state.h"
#include "validation_event.h"
#include "parser_context.h"
#include <glib.h>
#include <string.h>
#include <stdlib.h>


static bool _check_conditions(NumberValidator *v, Number const *n,
                              ValidationState *s, void *ctxt)
{
	if (v->integer && !number_is_integer(n))
	{
		validation_state_notify_error(s, VEC_NOT_INTEGER_NUMBER, ctxt);
		return false;
	}

	if (v->expected_set && 0 != number_compare(n, &v->expected_value))
	{
		validation_state_notify_error(s, VEC_UNEXPECTED_VALUE, ctxt);
		return false;
	}

	if (v->min_set)
	{
		int cmp = number_compare(n, &v->min);
		if (cmp == -1 || (cmp == 0 && v->min_exclusive))
		{
			validation_state_notify_error(s, VEC_NUMBER_TOO_SMALL, ctxt);
			return false;
		}
	}

	if (v->max_set)
	{
		int cmp = number_compare(n, &v->max);
		if (cmp == 1 || (cmp == 0 && v->max_exclusive))
		{
			validation_state_notify_error(s, VEC_NUMBER_TOO_BIG, ctxt);
			return false;
		}
	}

	return true;
}

static bool _check(Validator *v, ValidationEvent const *e, ValidationState *s, void *ctxt)
{
	if (e->type != EV_NUM)
	{
		validation_state_notify_error(s, VEC_NOT_NUMBER, ctxt);
		validation_state_pop_validator(s);
		return false;
	}

	Number n;
	number_init(&n);
	if (number_set_n(&n, e->value.string.ptr, e->value.string.len))
	{
		number_clear(&n);
		// TODO: Number format error
		validation_state_notify_error(s, VEC_NOT_NUMBER, ctxt);
		validation_state_pop_validator(s);
		return false;
	}

	bool res = _check_conditions((NumberValidator *) v, &n, s, ctxt);

	number_clear(&n);
	validation_state_pop_validator(s);
	return res;
}

static void _release(Validator *validator)
{
	NumberValidator *v = (NumberValidator *) validator;
	number_validator_release(v);
}

static void _set_maximum(Validator *v, Number *num)
{
	NumberValidator *n = (NumberValidator *) v;
	n->max_set = true;
	number_init(&n->max);
	number_copy(&n->max, num);
}

static void _set_maximum_exclusive(Validator *v, bool exclusive)
{
	NumberValidator *n = (NumberValidator *) v;
	number_validator_add_max_exclusive_constraint(n, exclusive);
}

static void _set_minimum(Validator *v, Number *num)
{
	NumberValidator *n = (NumberValidator *) v;
	n->min_set = true;
	number_init(&n->min);
	number_copy(&n->min, num);
}

static void _set_minimum_exclusive(Validator *v, bool exclusive)
{
	NumberValidator *n = (NumberValidator *) v;
	number_validator_add_min_exclusive_constraint(n, exclusive);
}

static ValidatorVtable number_vtable =
{
	.check = _check,
	.release = _release,
	.set_number_maximum = _set_maximum,
	.set_number_maximum_exclusive = _set_maximum_exclusive,
	.set_number_minimum = _set_minimum,
	.set_number_minimum_exclusive = _set_minimum_exclusive,
};

NumberValidator* number_validator_new(void)
{
	NumberValidator *self = g_new0(NumberValidator, 1);
	if (!self)
		return NULL;
	validator_init(&self->base, &number_vtable);
	return self;
}

NumberValidator* integer_validator_new(void)
{
	NumberValidator *v = number_validator_new();
	v->integer = true;
	return v;
}

void number_validator_release(NumberValidator *v)
{
	if (v->expected_set)
		number_clear(&v->expected_value);
	if (v->min_set)
		number_clear(&v->min);
	if (v->max_set)
		number_clear(&v->max);
	g_free(v);
}

bool number_validator_add_min_constraint(NumberValidator *n, const char* val)
{
	Number num;
	number_init(&num);
	if (number_set(&num, val))
	{
		number_clear(&num);
		return false;
	}
	_set_minimum(&n->base, &num);
	number_clear(&num);
	return true;
}

bool number_validator_add_min_exclusive_constraint(NumberValidator *n, bool exclusive)
{
	n->min_exclusive = exclusive;
	return true;
}

bool number_validator_add_max_constraint(NumberValidator *n, const char* val)
{
	Number num;
	number_init(&num);
	if (number_set(&num, val))
	{
		number_clear(&num);
		return false;
	}
	_set_maximum(&n->base, &num);
	number_clear(&num);
	return true;
}

bool number_validator_add_max_exclusive_constraint(NumberValidator *n, bool exclusive)
{
	n->max_exclusive = exclusive;
	return true;
}

bool number_validator_add_expected_value(NumberValidator *n, StringSpan *span)
{
	if (n->expected_set)
		number_clear(&n->expected_value), n->expected_set = false;

	number_init(&n->expected_value);
	if (number_set_n(&n->expected_value, span->str, span->str_len))
	{
		number_clear(&n->expected_value);
		return false;
	}
	n->expected_set = true;
	return true;
}
