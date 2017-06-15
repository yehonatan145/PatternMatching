#ifndef FIELD_H
#define FIELD_H

/******************************************************************************************************
*		INCLUDES:
******************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/******************************************************************************************************
*		DEFINITIONS:
******************************************************************************************************/

#define MOD_INC(x,p) ((x) = ((x) + 1) % (p))
#define MOD_DEC(x,p) ((x) = ((x) ? (x) - 1 : (p) - 1))

// NOTE: field size must be up to sqrt(sizeof(field_t)) because we need to be able to multiply

typedef unsigned long long field_t;

/*
struct for saving value in field and it's inverse
*/
typedef struct {
	field_t val;
	field_t inv;
} FieldVal;

/******************************************************************************************************
*		FUNCTION TO EXTERN:
******************************************************************************************************/

/* Calculate the inverse of given field value and the field size */
field_t calculate_inverse(field_t a, field_t p);

/******************************************************************************************************
*		INLINES:
******************************************************************************************************/

static inline void field_copy(FieldVal* dst, FieldVal* src) {
    dst->val = src->val;
    dst->inv = src->inv;
}

/*
Divide in the field.

@param dst          The value to put the result numerator/denomerater in
@param numerator    The numerator of the division
@param denomerator  The denomerator of the division
@param p			The size of the field
*/
static inline void field_div(FieldVal* dst, FieldVal* numerator, FieldVal* denomerator, field_t p) {
    // since if dst == denomerator, when we change dst->val we also change denomerator->val
    // to solve this, we save den_val before changing dst->val
    field_t den_val = denomerator->val;
    dst->val = (numerator->val * denomerator->inv) % p;
    dst->inv = (den_val * numerator->inv) % p;
}

/*
Divide in the field.

@param dst          The value to put the result numerator/denomerater in
@param numerator    The numerator of the division
@param denomerator  The denomerator of the division
@param p			The size of the field
*/
static inline void field_mul(FieldVal* dst, FieldVal* val1, FieldVal* val2, field_t p) {
    dst->val = (val1->val * val2->val) % p;
    dst->inv = (val1->inv * val2->inv) % p;
}

#endif /* FIELD_H */
