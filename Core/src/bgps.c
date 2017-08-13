/**
* Breslauer-Galil Pattern Searcher
*
* The Real-Time Breslauer-Galil algorithm for searching pattern in a stream.
*
* Definitions:
*
* We use rabin-karp fingerprints in order to check when we have a match between two strings.
* A cumulative fingerprint is a fingerprint of all the stream until some position.
* We divide the pattern to log n stages, each stage k is a prefix of the pattern of length 2^k (except last which is n).
*
* Viable Occurance (a.k.a VO), is a position in the stream, where we didn't yet ruled out the option
* of it to be a match. (i.e. a match can occur where the first character matched is the VO).
* The k-block of a VO is the stream[VO.pos .. VO.pos + 2 ^ k].
* (where 2 ^ k is replaced with the pattern length (n) in case that k is the last stage).
* Each VO start from the first stage, and climb up the stage ladder to the last stage (stage logn).
* When we know that a position can't be a match, we say that the VO fall from the ladder
* Because of properties of periodicity, we know that all the VOs in a stage form a linear (arithmetic) progression.
* 
* For the first stage, we use Galil's Real-Time version of the KMP algorithm, in the following way:
*
*   1.  We find the period of stage (ceil(log log n) + 1), and computes where it ends (the last position
*       that still have the same period).
*
*   2.  We then find the last stage that contained in this continuation.
*       We know this stage (from now "first stage"), have period of length > log n.
*
*   3.  We divide the first stage to number of periods and remaining. (first stage = m * period + remaining)
*
*   4.  Then, we make two different instances of Galil's Real-Time version of KMP, one for the period
*       and one for the remaining. We give both of them every character arrives, and count the number
*       of continued instances of the period in the stream (i.e. number of times we saw the period, when each
*       instance started right after the earlier ended).
*
*   5.  If we found out that the number of periods matches are equal to the needed size,
*       and that the remaining is also a match, we say that that was a match in the first stage.
*
*
* For every other stage:
*   We go through all stages in a round-robin fashion (one stage per character, in decreasing order,
*   while saving cumulative fingerprints in a buffer). And in each stage k:
*
*   -   We check if we already saw the character that ended the (k+1)-block of the first VO in stage k.
*
*   -   If we did, we check if the fingerprint of the (k+1)-block match the expected fingerprint of stage k + 1.
*       (and anyway removing this VO from stage k, since we can determine if it went to next stage or fell)
*       For that, we precompute the fingerprints of all the stages.
*
*   -   Because of the way we chose the first stage, we know that between every two VOs in the same stage,
*       there is distance difference of at least log n characters, so it is OK to check only the first one.
*
*   -   For the last stage (and 1-before-last stage if the length difference between them is smaller then log n),
*       we check on every character if they match, so we won't remember too late we had a match.
*       (since other stages have a delay in the matches since we go through the stages in round-robin).
*
* IMPORTANT: The stages round-robin MUST be in decreading order, because otherwise, the next can happen:
*   Lets say x is the first VO in stage i, and y is the second VO in the same stage.
*   Lets also say that z is a VO currently moving from stage i-1 to stage i.
*   It is possible that x should not be a VO right now (but it still is, because we didn't got to it in the round-robin),
*   and that z is not linear progression with x and y (even though x,y and z are matches).
*   So if we go to z before x, we will think we have a fingerprint collision, even though we don't.
*/


/******************************************************************************************************
*		INCLUDES
******************************************************************************************************/


#include "bgps.h"
#include "util.h"
#include <stdio.h>
#include <time.h>


/******************************************************************************************************
*		INNER FUNCTIONS
******************************************************************************************************/


/**
* Calculate log2 of given number.
*
* @param x        The number to calculate log2 on
* @param ceil     Whether to ceil the result (otherwise its floor)
*
* @return         The log2 of x (ceiled/floored according to ceil parameter)
*/
static int bg_log2(size_t x, int ceil)
{
    static const unsigned long long t[6] = {
        0xFFFFFFFF00000000ull,
        0x00000000FFFF0000ull,
        0x000000000000FF00ull,
        0x00000000000000F0ull,
        0x000000000000000Cull,
        0x0000000000000002ull
    };
    // y should start from 1 only if we are ceiling and x is not power of 2
    int y = (!ceil || ((x & (x - 1)) == 0)) ? 0 : 1;
    int j = 32;
    int i;
    for (i = 0; i < 6; i++) {
        int k = (((x & t[i]) == 0) ? 0 : j);
        y += k;
        x >>= k;
        j >>= 1;
    }

    return y;
}

/**
* Get the length of a stage by the real stage number
*/
static inline size_t real_stage_to_len(BGStruct* bg, size_t stage_num) {
	return stage_num == bg->logn ? bg->n : (1 << stage_num);
}

/**
* Get the real stage number of the longest stage that fit in the given length
*/
static inline size_t len_to_real_stage(BGStruct* bg, size_t len) {
	return len == bg->n ? bg->logn : bg_log2(len, 0);
}

/**
* Get the length of a stage by the stage number (real stage is first_stage + the stage number)
*/
static inline size_t stage_to_len(BGStruct* bg, size_t stage_num) {
	return real_stage_to_len(bg, stage_num + bg->first_stage);
}

/**
* Get the stage number of the longest stage that fit in the given length
*/
static inline size_t len_to_stage(BGStruct* bg, size_t len) {
	return len_to_real_stage(bg, len) - bg->first_stage;
}

/**
* Find until where the period of pattern[0..n-1] continue in pattern[0..all-1].
* 
* @param pattern  The pattern itself
* @param all      The length of the all pattern
* @param n        The length of the sub-pattern which we have period of
* @param period   The period length of pattern[0..n-1] (assumes that the period is correct)
*
* @return         The first position in which the period of pattern[0..n-1], is finished in pattern[0..all-1]
*                 or return the pattern length (all) if the period continue in all the pattern
*/
static size_t _bgps_find_period_continue(char* pattern, size_t all, size_t n, size_t period) {
	for ( ; n < all; n++) {
		if (pattern[n] != pattern[n % period]) {
			return n;
		}
	}
	return n; // if got here, the all patern have the same period (n == all)
}

/**
* Initializes the kmp in the bg struct.
*
* Following members of bg MUST to be already initialized:
*   n
*   loglogn
*
* Initializes:
*   first_stage
*   kmp_period
*   kmp_remaining
*   n_kmp_period
*   current_n_kmp_period
*   last_kmp_period_match_pos
*
* @param bg       The bg struct we work on
* @param pattern  The pattern of the bg struct
*/
static void _bgps_init_kmp(BGStruct* bg, char* pattern) {
	size_t stage_loglogn_period = kmp_get_period(pattern, 1 << bg->loglogn);
	size_t period_stopped_pos = _bgps_find_period_continue(pattern, bg->n, 1 << bg->loglogn, stage_loglogn_period);
	// The last position in which the period continue is period_stopped_pos - 1
	// So, the length of the block [0..period_stopped_pos-1] is period_stopped_pos
	bg->first_stage = len_to_real_stage(bg, period_stopped_pos);
	bg->kmp_period = kmp_new(pattern, stage_loglogn_period);
	bg->n_kmp_period = real_stage_to_len(bg, bg->first_stage) / stage_loglogn_period;
	size_t remaining = real_stage_to_len(bg, bg->first_stage) % stage_loglogn_period;
	if (remaining) {
		bg->kmp_remaining = kmp_new(pattern, remaining);
	} else {
		bg->kmp_remaining = NULL;
	}
	bg->current_n_kmp_period = 0;
	bg->last_kmp_period_match_pos = 0;
}

/**
* Initializes the fps of all stages in the bg struct
*
* Following members of bg MUST be already initialized:
*   logn
*   first_stage
*   r
*   p
*   flags
*
* Initialize:
*   first_stage_r ( = r^(2^first_stage - 1) )
*   fps
*   determine value of BG_NEED_BEFORE_LAST_STAGE_FLAG
*
* @param bg       The bg struct
* @param pattern  The pattern of the bg struct
*/
static void _bgps_init_fps(BGStruct* bg, char* pattern) {
	// There is N_STAGES + 1 fingerprints, because the all pattern's fingerprint must be saved, but it is not a stage.
	bg->fps = (fingerprint_t*) malloc ((N_STAGES(bg) + 1) * sizeof(fingerprint_t));
	FieldVal rn;
	size_t first_stage = bg->first_stage, logn = bg->logn;
	bg->fps[0] = calc_fp(pattern, stage_to_len(bg, 0), &rn, &bg->r, bg->p);
	field_div(&bg->first_stage_r, &rn, &bg->r, bg->p); // now, first_stage_r == r^(2^first_stage - 1)
	int i = first_stage + 1;
	for (; i < logn; ++i) {
		bg->fps[i - first_stage] = calc_fp_with_prefix(
			pattern,
			1 << i,
			bg->fps[i - first_stage - 1],
			1 << (i - 1),
			&rn,
			&bg->r,
			bg->p
			);
	}
	if (first_stage != logn) {
		// Now i is logn
		bg->fps[i - first_stage] = calc_fp_with_prefix(
			pattern,
			bg->n,
			bg->fps[i - first_stage - 1],
			1 << (i - 1),
			&rn,
			&bg->r,
			bg->p
		);
		if (bg->n - (1 << (i - 1)) < logn) {
			bg->flags |= BG_NEED_BEFORE_LAST_STAGE_FLAG;
		}	
	}
}

/**
* Add new VO to the VOs (if possible)
*
* Also update the flags for having last stage(s) if necessary
*
* @param bg       The bg struct
* @param stage    The stage number (index in vos)
* @param pos      The position of the VO we want to add (first character of the VO)
* @param fp       The fingerprint of the all stream until pos NOT INCLUDE pos (i.e. fp(stream[0..pos-1]) )
* @param rn       r^pos
*
* @return       0 - failed
*               1 - succeed
*/
static int _bgps_add_vo(BGStruct* bg, size_t stage, pos_t pos, fingerprint_t fp, FieldVal* rn) {
	VOLinearProgression* vos = &bg->vos[stage];
	field_t p = bg->p;
	if (vos->n == 0) {
		vos->first.pos = pos;
		vos->first.fp = fp;
		field_copy(&vos->first.r, rn);
		vos->n = 1;
		if (stage == bg->logn) {
			bg->flags |= BG_HAVE_LAST_STAGE_FLAG;
		} else if (bg->flags & BG_NEED_BEFORE_LAST_STAGE_FLAG && stage == bg->logn - 1) {
			bg->flags |= BG_HAVE_BEFORE_LAST_STAGE_FLAG;
		}
	} else if (vos->n == 1) {
		vos->step.pos = pos - vos->first.pos;
		vos->step.fp = calc_fp_suffix(fp, vos->first.fp, &vos->first.r, p);
		field_div(&vos->step.r, rn, &vos->first.r, p);
		vos->n = 2;
	} else {
		if (vos->first.pos + (vos->n + 1) * vos->step.pos != pos) {
			// the new position is not in linear proression with the others
			return 0;
		}
		vos->n++;
	}
	return 1;
}

/**
* Remove the first VO from the VOs linear progression.
*
* Also update the flags for having last stage(s) if necessary
*
* @param bg     The bg struct
* @param stage  The stage number (index in vos)
*/
static void _bgps_remove_first_vo(BGStruct* bg, size_t stage) {
	VOLinearProgression* vos = &bg->vos[stage];
	field_t p = bg->p;
	if (vos->n == 0) {
		return;
	} else if (vos->n == 1) {
		vos->n = 0;
		if (stage == bg->logn) {
			bg->flags &= ~BG_HAVE_LAST_STAGE_FLAG;
		} else if (stage == bg->logn - 1) {
			bg->flags &= ~BG_HAVE_BEFORE_LAST_STAGE_FLAG;
		}
	} else {
		vos->first.pos += vos->step.pos;
		vos->first.fp = calc_fp_from_prefix_suffix(vos->first.fp, vos->step.fp, &vos->step.r, p);
		field_mul(&vos->first.r, &vos->first.r, &vos->step.r, p);
		vos->n--;
	}
}

/**
* Check whether we can upgrade the first VO in a stage to the next stage.
*
* If there been at least 2^(stage_num+1) characters between the first vo and the current character,
* it means that we need to remove this vo from the stage, and check whether it is match the next stage.
* The VO match the next stage if:
*   - The fingerprint of the VO block at stage stage_num+1 ( fp( stream[VO.pos .. VO.pos+2^(stage_num+1)] ) )
*     match the fingerprint of stage stage_num+1 in the pattern.
*   - The VO is linear progression with the other VOs in stage stage_num+1.
*
* @param bg          The bg struct
* @param stage_num   The stage number as indexed in vos & fps (real stage is first_stage + parameter)
*
* @return            1 if there was an upgrade, 0 otherwise
*/
static int _bgps_vo_stage_upgrade(BGStruct* bg, size_t stage_num) {
	int ret = 0;
	VOLinearProgression* vos = &bg->vos[stage_num];
	if (vos->n == 0) {
		return 0;
	}
	// end_pos is the position of the last character of the pattern of next stage starting at the first VO.
	pos_t end_pos = vos->first.pos + stage_to_len(bg, stage_num + 1);
	if (bg->current_pos < end_pos || bg->current_pos >= end_pos + bg->logn) {
		return 0; // We not yet need to upgrade the first VO
	}
	// check if fingerprint match the pattern:
	fingerprint_t check_fp = calc_fp_suffix(bg->last_fps[end_pos % bg->logn], vos->first.fp, &vos->first.r, bg->p);
	if (check_fp == bg->fps[stage_num + 1]) {
		if (stage_num == N_STAGES(bg) - 1) {
			// Last stage dont have next stage
			ret = 1;
		} else if (!_bgps_add_vo(bg, stage_num + 1, vos->first.pos, vos->first.fp, &vos->first.r)) {
			// There is a fingerprint collision, just wipe out the vos in that stage
			bg->vos[stage_num + 1].n = 0;
			fprintf(stderr, "fingerprint collision, at position %llu, wiping out the stage\n", vos->first.pos);
		} else {
			ret = 1;
		}
	}
	// remove the first VO from VOs:
	_bgps_remove_first_vo(bg, stage_num);
	return ret;
}

/**
* Check the last (and maybe before last) stage(s), because they need to be checked every char.
*
* @param bg   The bg struct
*
* @return     Whether the last stage match.
*/
static int _bgps_check_last_stages(BGStruct* bg) {
	if (bg->flags & BG_HAVE_BEFORE_LAST_STAGE_FLAG) {
		_bgps_vo_stage_upgrade(bg, N_STAGES(bg) - 2);
	}
	if (bg->flags & BG_HAVE_LAST_STAGE_FLAG) {
		return _bgps_vo_stage_upgrade(bg, N_STAGES(bg) - 1);
	}
	return 0;
}

/**
* Check whether we have a match of the first stage.
*
* @param bg       The bg struct
* @param c        The char just read
*
* @return         Whether the first stage has a match
*/
static int _bgps_check_first_stage(BGStruct* bg, char c) {
	// In any case, we need to give c to kmp_period (and kmp_remaining if exist)	
	int kmp_period_match = kmp_read_char(bg->kmp_period, c);
	size_t period_len = kmp_get_pattern_len(bg->kmp_period);
	int kmp_remaining_match = bg->kmp_remaining ? kmp_read_char(bg->kmp_remaining, c) : 1;
	size_t remaining_len = bg->kmp_remaining ? kmp_get_pattern_len(bg->kmp_remaining) : 0;

	if (kmp_period_match) { // if there was a match in kmp_period
		if (bg->last_kmp_period_match_pos + period_len == bg->current_pos) {
			bg->current_n_kmp_period++;
		} else {
			bg->current_n_kmp_period = 1;
		}
		bg->last_kmp_period_match_pos = bg->current_pos;
	} else {
		if (bg->last_kmp_period_match_pos + period_len <= bg->current_pos) {
			// We passed the position in which the next match should occur
			bg->current_n_kmp_period = 0;
		}
	}	
	/**
	* There is a match iff:
	* 1. There is a remaining match
	* 2. The number of periods matched until now is at least the needed number
	* 3. The position of the last period match + the remaining length is the current position
	*/
	if (kmp_remaining_match
	    && bg->current_n_kmp_period >= bg->n_kmp_period
	    && bg->last_kmp_period_match_pos + remaining_len == bg->current_pos) {
		return 1;
	}
	return 0;
}

/**
* Add the current position as the end of a new VO to the first stage
*
* @param bg       The bg struct
*/
static void _bg_add_to_first_stage(BGStruct* bg) {
	FieldVal vo_r;
	pos_t vo_pos = bg->current_pos - stage_to_len(bg, 0) + 1;
	field_div(&vo_r, &bg->current_r, &bg->first_stage_r, bg->p);
	fingerprint_t vo_fp = calc_fp_prefix(bg->current_fp, bg->fps[0], &vo_r, bg->p);
	if (!_bgps_add_vo(bg, 0, vo_pos, vo_fp, &vo_r)) {
		// fingerprint collision, just ignore the new vo (possible option is to wipe out first stage)
		fprintf(stderr, "fingerprint collision, at position %llu\n", vo_pos);
	}
}


/******************************************************************************************************
*		API FUNCTIONS
******************************************************************************************************/


/**
* Create new BGStruct accoring to a specific pattern
*
* @param pattern  The pattern to search
* @param n        The pattern length
*
* @return         Dynamically allocated BGStruct to use on stream
*/
BGStruct* bg_new(char* pattern, size_t n, field_t p) {
	BGStruct* bg = (BGStruct*) malloc (sizeof(BGStruct));
	memset(bg, 0, sizeof(BGStruct));
	bg->n = n;
	if (n <= BG_SHORT_PATTERN_LENGTH) {
		// in case of short pattern, we use the kmp real-time version for the all pattern.
		bg->flags |= BG_SHORT_PATTERN_FLAG;
		bg->kmp_period = kmp_new(pattern, n);
		return bg;
	}
	bg->logn = bg_log2(n, 1);
	bg->loglogn = bg_log2(bg->logn, 1) + 1;
	_bgps_init_kmp(bg, pattern);
	bg->p = p;
	srand(time(NULL));
	field_t r;
	do {
		r = rand() % p;
	} while (r <= 1);
	bg->r.val = r;
	bg->r.inv = calculate_inverse(r, p);

	_bgps_init_fps(bg, pattern);
	bg->last_fps = (fingerprint_t*) malloc (bg->logn * sizeof(fingerprint_t));
	bg->vos = (VOLinearProgression*) malloc (N_STAGES(bg) * sizeof(VOLinearProgression));	
	memset(bg->vos, 0, N_STAGES(bg) * sizeof(VOLinearProgression));
	bg->current_fp = 0;
	bg->current_r.val = 1;
	bg->current_r.inv = 1;
	bg->current_pos = 0;
	bg->current_stage = 0;
	return bg;
}

/**
* Return the total memory used for this struct
*
* @param bg       The bg struct
*
* @return         The total memory used for this struct (in bytes)
*/
size_t bg_get_total_mem(BGStruct* bg) {
	if (bg == NULL) return 0;
	if (bg->flags & BG_SHORT_PATTERN_FLAG) {
		return sizeof(BGStruct) + kmp_get_total_mem(bg->kmp_period);
	}
	return sizeof(BGStruct) +                              // for the struct itself
	       (N_STAGES(bg) + 1) * sizeof(fingerprint_t) +    // for fps member
	       N_STAGES(bg) * sizeof(VOLinearProgression) +    // for vos member
	       bg->logn * sizeof(fingerprint_t) +              // for last_fps member
	       kmp_get_total_mem(bg->kmp_period) +             // for kmp_period member
	       kmp_get_total_mem(bg->kmp_remaining);           // for kmp_remaining member
}

/**
* Reset the bg to the initial state
*
* @param bg    The bg struct to reset
*/
void bg_reset(BGStruct* bg) {
	if (bg->flags & BG_SHORT_PATTERN_FLAG) {
		kmp_reset(bg->kmp_period);
		return;
	}
	bg->current_r.val = 1;
	bg->current_r.inv = 1;
	bg->current_pos = 0;
	bg->current_fp = 0;
	bg->current_stage = 0;
	bg->last_kmp_period_match_pos = 0;
	bg->current_n_kmp_period = 0;
	memset(bg->vos, 0, N_STAGES(bg) * sizeof(VOLinearProgression));
	kmp_reset(bg->kmp_period);
	kmp_reset(bg->kmp_remaining);
	bg->flags &= ~BG_HAVE_LAST_STAGE_FLAG & ~BG_HAVE_BEFORE_LAST_STAGE_FLAG;
}

/**
* Read char and return whether we have a match.
*
* @param bg     The BGStruct of the pattern we want to search
* @param c      The new character from the stream
*
* @return       1 if found pattern, 0 if not
*/
int bg_read_char(BGStruct* bg, char c) {
	if (bg->flags & BG_SHORT_PATTERN_FLAG) {
		return kmp_read_char(bg->kmp_period, c);
	} else if (N_STAGES(bg) == 0) {
		return _bgps_check_first_stage(bg, c);
	}
	int ret = 0;
	bg->current_fp = calc_fp_from_prefix_suffix(bg->current_fp,
	                                            (fingerprint_t)c,
	                                            &bg->current_r,
	                                            bg->p);
	bg->last_fps[bg->current_pos % bg->logn] = bg->current_fp;
	if (_bgps_check_first_stage(bg, c)) {
		_bg_add_to_first_stage(bg);
	}
	if (_bgps_check_last_stages(bg)) {
		ret = 1;
	}
	if (N_STAGES(bg) > 1) {
		// If there is only one stage, then it is already handled by last stage checking
		// Worst that can happen, the before last stage is checked twice (which is fine)
		_bgps_vo_stage_upgrade(bg, bg->current_stage);
		MOD_DEC(bg->current_stage, N_STAGES(bg) - 1);
	}
	field_mul(&bg->current_r, &bg->current_r, &bg->r, bg->p);
	bg->current_pos++;
	return ret;
}

/**
* Free memory of BGStruct
*
* @param bg     The BGStruct to free
*/
void bg_free(BGStruct* bg) {
	if (!bg) return;
	if (bg->fps) free(bg->fps);
	if (bg->vos) free(bg->vos);
	if (bg->kmp_period) free(bg->kmp_period);
	if (bg->kmp_remaining) free(bg->kmp_remaining);
	if (bg->last_fps) free(bg->last_fps);
	free(bg);
}

/*
================================= F O R     T E S T I N G ================================
*/

/*
void print_VOS(VOLinearProgression* vos, int stage) {
	printf("VOs in stage %d: from %llu to %llu (%d steps of %llu)\n", stage, vos->first.pos,
		vos->first.pos + (vos->n - 1) * vos->step.pos, vos->n, vos->step.pos);
	printf("	first fingerprint: %llu, step fingerprint: %llu\n", vos->first.fp, vos->step.fp);
	printf("	r^first position = %llu, r^step = %llu\n", vos->first.r.val, vos->step.r.val);
}

void print_BG(BGStruct* bg) {
	printf("BG struct information:\n\n");
	if (bg->flags & BG_SHORT_PATTERN_FLAG) {
		printf("BG is in short pattern mode\n");
	} else {
		printf("p = %llu, r = %llu, n = %d, first stage = %d\n", bg->p, bg->r.val, bg->n, bg->first_stage);
		printf("logn = %d, loglogn = %d, number of VO-stages = %d\n", bg->logn, bg->loglogn, N_STAGES(bg));
		printf("kmp period length = %d, kmp remaining length = %d, number of periods = %d\n",
			kmp_get_pattern_len(bg->kmp_period), kmp_get_pattern_len(bg->kmp_remaining), bg->n_kmp_period);
	}
}

void print_BG_after_char(BGStruct* bg) {
	if (bg->flags & BG_SHORT_PATTERN_FLAG) {
		printf("BG is in short pattern mode\n");
	} else {
		printf("current position = %llu, r ^ current position = %llu, current fingerprint = %llu\n",
			bg->current_pos, bg->current_r.val, bg->current_fp);
		printf("current stage = %d, current flag = %d\n", bg->current_stage, bg->flags);
		printf("VOS:\n");
		int i;
		for (i = 0; i < N_STAGES(bg); ++i) {
			print_VOS(&bg->vos[i], i);
		}
	}
}
*/

/*
int main() {
	int i;
	printf("\n\n");
    field_t p = 101ul;
    //               0         1   
    //               01234567890123
    char* pattern = "ABCDABDABC";
    BGStruct* bg = bg_new(pattern, strlen(pattern), p);
    //            0         1         2         3         4
    //            012345678901234567890123456789012345678901234
    char* text = "ABCDABCDABDABCDABDABCDABBABCDABDABCDABDBADFSG";

    // matches: 13, 20, 34.

    printf("\n\npattern is: %s\n\n", pattern);
    printf("fps:\n");
    for (i = 0; i < N_STAGES(bg); ++i) {
    	printf("fp %llu in stage no. %d\n", bg->fps[i], i);
    }
    print_BG(bg);
    for (i = 0; i < strlen(text); ++i) {
    	//printf("\n\non character number %d, char = %c:\n\n\n", i, text[i]);
    	if (bg_read_char(bg, text[i])) {
    		printf("=======================================================================\n");
    		printf("======================== found match on index %d ======================\n", i);
    		printf("=======================================================================\n");
    	}
    	//print_BG_after_char(bg);
    }
}
*/