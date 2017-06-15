#include "Fingerprint.h"

/**
* Calculate the fingerprint of a sequence.
*
* @param seq	The char sequence
* @param len	The length of the char sequence
* @param rn     Place to put the filed-value of r^n
* @param r      The value of r for this sequence
* @param p      The size of the field
*
* @return		The fingerprint of the sequence
*/
fingerprint_t calc_fp(char* seq, size_t len, FieldVal* rn, FieldVal* r, fingerprint_t p) {
    fingerprint_t ret = 0, current_rn = 1, current_inv_rn = 1;
    fingerprint_t r_val = r->val, r_inv = r->inv; // So we won't access memory all the time
    size_t i;
    for (i = len; i > 0; --i) {
        ret += (*(seq++) * current_rn);
        ret %= p;
        current_rn = (current_rn * r_val) % p;
        current_inv_rn = (current_inv_rn * r_inv) % p;
    }
    rn->val = current_rn;
    rn->inv = current_inv_rn;
    return ret;
}

/**
* Calculate the fingerprint of a sequence when already have fingerprint of prefix (and r^length of prefix).
*
* @param seq		    The char sequence
* @param len		    The sequence length
* @param prefix_fp	The fingerprint of the prefix we have
* @param prefix_len	The length of the prefix we have
* @param rn           r^prefix_len, changed to be r^len during run (must not be NULL)
* @param r            The value of r for this sequence (rn must have been calculated with the same r)
* @param p            The size of the field
*
* @return		The fingerprint of the sequence
*/
fingerprint_t calc_fp_with_prefix(char* seq, size_t len, fingerprint_t prefix_fp, size_t prefix_len,
        FieldVal* rn, FieldVal* r, fingerprint_t p) {
    fingerprint_t current_rn = rn->val, current_inv_rn = rn->inv;
    fingerprint_t r_val = r->val, r_inv = r->inv; // So we won't access memory all the time
    size_t i;
    seq += prefix_len;
    for (i = len - prefix_len; i > 0; --i) {
        prefix_fp += *(seq++) * current_rn;
        prefix_fp %= p;
        current_rn = (current_rn * r_val) % p;
        current_inv_rn = (current_inv_rn * r_inv) % p;
    }
    rn->val = current_rn;
    rn->inv = current_inv_rn;
    return prefix_fp;
}


