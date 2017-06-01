#include "field.h"

/*
Calculate the inverse of a in field with size p
p should be prime (or at least gcd(a,p)=1).

@param a	The value to find inverse to
@param p	The size of the field

@return		The inverse of {@param a}
*/
field_t calculate_inverse(field_t a, field_t p) {
	/*
	* At all time the invariant is t*a == r (mod p) and tt*a == rr (mod p)
	* where r and rr is decreasing as in Euclid Algorithm (new_r = old_rr, new_rr = old_r mod old_rr)
	* At each stage we decrease r and rr until r == 1 (in which case, rr will be 0)
	* 
	* now, for each stage, if:
	*   t * a = r
	*   tt * a = rr
	*   rr < r
	* then we define:
	*   q := r / rr (integer division)
	*   ttt := t - q * tt
	*   rrr := r - q * rr
	* and then:
	*   ttt * a = (t - q * tt) * a = t * a - q * tt * a = r - q * rr = rrr
	* We got, ttt * a == rrr, with rrr < rr
	* And so we change:
	*   r <-- rr
	*   t <-- tt
	*   rr <-- rrr
	*   tt <-- ttt
	*
	* So now, we keep the invariant all the time.
	* When rr becomes 0, it means that r is now gcd(a,p) (which should be 1, since p should be prime)
	* So now, we get that for r = 1, t * a = 1 (mod p)
	*/
	field_t r = p, t = 0, rr = a, tt = 1;
	field_t temp, q;
	while (rr != 0) {
		q = r / rr;
		// (r, rr) = (rr, r - q * rr)
		temp = rr;
		rr = r - q * rr;
		r = temp;
		// (t, tt) = (tt, t - q * tt)
		temp = tt;
		/*
		* Every tt here (with the same modulo p) should do the trick, but we want 0 < tt < p.
		* Because we work with unsigned, we very careful not to go negative for a moment.
		*/
		tt = t > q * tt ? t - q * tt : t + p - (q * tt % p);
		t = temp;
	}
	// ASSERT(r, 1) // r need to be 1 right now (if, indeed, gcd(a,p) == 1)
	return t;
}
