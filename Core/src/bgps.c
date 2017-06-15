/**
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
* 	1.	We find the period of stage (ceil(log log n) + 1), and computes where it ends (the last position
*		that still have the same period).
*
*	2.	We then find the last stage that contained in this continuation.
*		We know this stage (from now "first stage"), have period of length > log n.
*
*	3.	We divide the first stage to number of periods and remaining. (first stage = m * period + remaining)
*
*	4.	Then, we make two different instances of Galil's Real-Time version of KMP, one for the period
*		and one for the remaining. We give both of them every character arrives, and count the number
*		of continued instances of the period in the stream (i.e. number of times we saw the period, when each
*		instance started right after the earlier ended).
*
*	5.	If we found out that the number of periods matches are equal to the needed size,
*		and that the remaining is also a match, we say that that was a match in the first stage.
*
*
* For every other stage:
*	We go through all stages in a round-robin fashion (one stage per character, in decreasing order,
*	while saving cumulative fingerprints in a buffer). And in each stage k:
*
*	-	We check if we already saw the character that ended the (k+1)-block of the first VO in stage k.
*
*	-	If we did, we check if the fingerprint of the (k+1)-block match the expected fingerprint of stage k + 1.
*		(and anyway removing this VO from stage k, since we can determine if it went to next stage or fell)
* 		For that, we precompute the fingerprints of all the stages.
*
*	-	Because of the way we chose the first stage, we know that between every two VOs in the same stage,
*		there is distance difference of at least log n characters, so it is OK to check only the first one.
*
*	-	For the last stage (and 1-before-last stage if the length difference between them is smaller then log n),
*		we check on every character if they match, so we won't remember too late we had a match.
*		(since other stages have a delay in the matches since we go through the stages in round-robin).
*
* IMPORTANT: The stages round-robin MUST be in decreading order, because otherwise, the next can happen:
*	Lets say x is the first VO in stage i, and y is the second VO in the same stage.
*	Lets also say that z is a VO currently moving from stage i-1 to stage i.
*	It is possible that x should not be a VO right now (but it still is, because we didn't got to it in the round-robin),
*	and that z is not linear progression with x and y (even though x,y and z are matches).
*	So if we go to z before x, we will think we have a fingerprint collision, even though we don't.
*/
#include "bgps.h"
#include <stdio.h>
#include <time.h>


void print_BG_after_char(BGStruct* bg);

/**
* Calculate log2 of given number.
*
* @param x        The number to calculate log2 on
* @param ceil     Whether to ceil the result (otherwise its floor)
*
* @return         The log2 of x (ceiled/floored according to ceil parameter)
*/
int bg_log2(field_t x, int ceil)
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
* Find until where the period of pattern[0..n-1] continue in pattern[0..all-1].
* 
* @param pattern  The pattern itself
* @param all      The length of the all pattern
* @param n        The length of the sub-pattern which we have period of
* @param period   The period length of pattern[0..n-1] (assumes that the period is correct)
*
* @return         The last position in which the period of pattern[0..n-1], is continued in pattern[0..all-1]
*/
size_t _bgps_find_period_continue(char* pattern, size_t all, size_t n, size_t period) {
    for ( ; n < all; n++) {
        if (pattern[n] != pattern[n % period]) {
            return n - 1;
        }
    }
    return n - 1; // if got here, the all patern have the same period (n == all)
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
void _bgps_init_kmp(BGStruct* bg, char* pattern) {
    int stage_loglogn_period = kmp_get_period(pattern, 1 << bg->loglogn);
    int last_period_continued = _bgps_find_period_continue(pattern, bg->n, 1 << bg->loglogn, stage_loglogn_period);
    //printf("stage loglogn period is %d, and the last period continue is %d\n", stage_loglogn_period, last_period_continued);
    bg->first_stage = bg_log2(last_period_continued, 0);
    bg->kmp_period = kmp_new(pattern, stage_loglogn_period);
    bg->n_kmp_period = (1 << bg->first_stage) / stage_loglogn_period;
    int remaining = (1 << bg->first_stage) % stage_loglogn_period;
    if (remaining != 0) {
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
void _bgps_init_fps(BGStruct* bg, char* pattern) {
    bg->fps = (fingerprint_t*) malloc ((N_STAGES(bg) + 1) * sizeof(fingerprint_t));    
    // There is N_STAGES + 1 fingerprints, because the all pattern's fingerprint must be saved, but it is not a stage.
    FieldVal rn;
    size_t first_stage = bg->first_stage;
    bg->fps[0] = calc_fp(pattern, 1 << first_stage, &rn, &bg->r, bg->p);
    field_div(&bg->first_stage_r, &rn, &bg->r, bg->p); // now, first_stage_rn == r^(2^first_stage - 1)
    int i = first_stage + 1;
    for (; i < bg->logn; ++i) {
        bg->fps[i - first_stage] = 
            calc_fp_with_prefix(pattern, 1 << i, bg->fps[i - first_stage - 1], 1 << (i - 1), &rn, &bg->r, bg->p);
    }
    // Now i is logn, but we prefer to write i because it in cache
    bg->fps[i - bg->first_stage] =
        calc_fp_with_prefix(pattern, bg->n, bg->fps[i - bg->first_stage - 1], 1 << (i - 1), &rn, &bg->r, bg->p);
    if (bg->n - (1 << (i - 1)) < bg->logn) {
        bg->flags |= BG_NEED_BEFORE_LAST_STAGE_FLAG;
    }
}

/**
* Create new BGStruct accoring to a specific pattern
*
* @param pattern  The pattern to search
* @param n        The pattern length
*
* @return		Dynamic allocated BGStruct to use on stream
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
    
    //
    // TODO move to somewhere else, and find way to get rid of p
    bg->p = p;
    srand(time(NULL));
    field_t r;
    do {
        r = rand() % p;
    } while (r <= 1);
    bg->r.val = r;
    bg->r.inv = calculate_inverse(r, p);
    //
    //

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
* Add new VO to the VOs (if possible)
*
* @param vos      The VOs linear progression
* @param pos      The position of the VO we want to add (first character of the VO)
* @param fp       The fingerprint of the all stream until pos NOT INCLUDE pos (i.e. fp(stream[0..pos-1]) )
* @param rn       r^pos
* @param p        The size of the field
*
* @return       0 - failed
*				1 - succeed
*				2 - the VOs was empty (after the call, number of VOs is 1)
*/
int _bgps_add_vo(VOLinearProgression* vos, pos_t pos, fingerprint_t fp, FieldVal* rn, field_t p) {
	//printf("add vo pos = %llu\n", pos);
    if (vos->n == 0) {
    	vos->first.pos = pos;
    	vos->first.fp = fp;
    	field_copy(&vos->first.r, rn);
    	vos->n = 1;
    	return 2;
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
* @param vos	The VOs linear progression
* @param p		The size of the field
*
* @return		0 - there are still VOs left
*				1 - the VOs is now empty
*/
int _bgps_remove_first_vo(VOLinearProgression* vos, field_t p) {
	//printf("remove vo pos = %llu, n = %d\n", vos->first.pos, vos->n);
	if (vos->n == 0) {
		return 1;
	} else if (vos->n == 1) {
		vos->n = 0;
		return 1;
	} else {
		vos->first.pos += vos->step.pos;
		vos->first.fp = calc_fp_from_prefix_suffix(vos->first.fp, vos->step.fp, &vos->step.r, p);
		field_mul(&vos->first.r, &vos->first.r, &vos->step.r, p);
		vos->n--;
		return 0;
	}
}

/**
* Check whether we can upgrade the first VO in a stage to the next stage.
*
* If there been at least 2^(stage_num+1) characters between the first vo and the current character,
* it means that we need to remove this vo from the stage, and check whether it is match the next stage.
* The VO match the next stage if:
*	- The fingerprint of the VO block at stage stage_num+1 ( fp( stream[VO.pos .. VO.pos+2^(stage_num+1)] ) )
*	  	match the fingerprint of stage stage_num+1 in the pattern.
*	- The VO is linear progression with the other VOs in stage stage_num+1.
*
* Change flags BG_HAVE_LAST_STAGE_FLAG, BG_HAVE_BEFORE_LAST_STAGE_FLAG if needed.
*
* @param bg			The bg struct
* @param stage_num	The stage number as indexed in vos & fps (real stage is first_stage + parameter)
*
* @return		If we found that the first vo should be upgraded to the next stage,
*			    but it don't fit to the next stage's linear progression, return 0.
*			    Otherwise return 1.
*/
int _bgps_vo_stage_upgrade(BGStruct* bg, size_t stage_num) {
	//printf("upgrade first vo of stage %d\n", stage_num);
	VOLinearProgression* vos = &bg->vos[stage_num];
	VOLinearProgression* next_vos = &bg->vos[stage_num + 1];
	if (vos->n == 0) {
		return 1;
	}
	size_t real_next_stage = bg->first_stage + stage_num + 1;
	// end_pos is the position of the last character of the pattern of next stage starting at the first VO.
	pos_t end_pos = vos->first.pos + (real_next_stage == bg->logn ? bg->n : 1 << real_next_stage);
	//printf("end_pos = %llu, current_pos = %llu\n", end_pos, bg->current_pos);
	if (bg->current_pos < end_pos || bg->current_pos >= end_pos + bg->logn) {
		return 1; // We not yet need to upgrade the first VO
	}
	// check if fingerprint match the pattern:
	fingerprint_t check_fp = calc_fp_suffix(bg->last_fps[end_pos % bg->logn], vos->first.fp, &vos->first.r, bg->p);
	if (check_fp == bg->fps[stage_num]) {
		int resp = _bgps_add_vo(next_vos, vos->first.pos, vos->first.fp, &vos->first.r, bg->p);
		if (!resp) {
			LOG("Fingerprint Collision, possibly match on %llu\n", vos->first.pos + bg->n);
			_bgps_remove_first_vo(vos, bg->p);
			return 0;
		} else if (resp == 2) {
			// check if we need to turn on flags:
			//printf("checking for flags change: stage number is %d\n", stage_num);
			if (stage_num == N_STAGES(bg) - 2 && bg->flags & BG_NEED_BEFORE_LAST_STAGE_FLAG) {
				bg->flags |= BG_NEED_BEFORE_LAST_STAGE_FLAG;
			} else if (stage_num == N_STAGES(bg) - 1) {
				bg->flags |= BG_HAVE_LAST_STAGE_FLAG;
			}
		}
	}
	// remove the first VO from VOs:
	if (_bgps_remove_first_vo(vos, bg->p)) {
		// check if we need to turn off flags:
		if (stage_num == N_STAGES(bg) - 2) {
			bg->flags &= ~BG_HAVE_BEFORE_LAST_STAGE_FLAG;
		} else if (stage_num == N_STAGES(bg) - 1) {
			bg->flags &= ~BG_HAVE_LAST_STAGE_FLAG;
		}
	}
	return 1;
}

/**
* Check the last (and maybe before last) stage(s), because they need to be checked every char.
*
* @param bg   The bg struct
*
* @return     Whether the last stage match.
*/
int _bgps_check_last_stages(BGStruct* bg) {
	//printf("check last stage(s)\n");
	int ret = 0;
	VOLinearProgression* vos;
	if (bg->flags & BG_HAVE_LAST_STAGE_FLAG) {
		//printf("checking last stage\n");
		vos = &bg->vos[N_STAGES(bg) - 1];
		if (vos->first.pos + bg->n - 1 == bg->current_pos) {
			//printf("last stage is on position, ");
			fingerprint_t check_fp = calc_fp_suffix(bg->current_fp, vos->first.fp, &vos->first.r, bg->p);
			//printf("last stage fp on stream is %llu, last stage pattern fp is %llu\n", check_fp, bg->fps[N_STAGES(bg)]);
			//printf("VO:first fp is %llu, first r is %llu, current fp is %llu\n", vos->first.fp, vos->first.r.val, bg->current_fp);
			if (check_fp == bg->fps[N_STAGES(bg)]) {
				//printf("have a match in position %llu\n", bg->current_pos);
				ret = 1;
			}
			if (_bgps_remove_first_vo(vos, bg->p)) {
				bg->flags &= ~BG_HAVE_LAST_STAGE_FLAG;
			}
		}
	}
	if (bg->flags & BG_HAVE_BEFORE_LAST_STAGE_FLAG) {
		_bgps_vo_stage_upgrade(bg, N_STAGES(bg) - 2);
	}
	return ret;
}

/**
* Check whether we have a match of the first stage.
*
* @param bg       The bg struct
* @param c        The char just read
*
* @return         Whether the first stage has a match
*/
int _bgps_check_first_stage(BGStruct* bg, char c) {
	//printf("check first stage\n");
	// In any case, we need to give c to kmp_period (and kmp_remaining if exist)
	int kmp_period_match = kmp_read_char(bg->kmp_period, c);
	size_t period_len = kmp_get_pattern_len(bg->kmp_period);

	if (kmp_period_match) { // if there was a match in kmp_period
		if (bg->last_kmp_period_match_pos + period_len == bg->current_pos) {
			// It fit the last kmp_period match
			bg->current_n_kmp_period++;
		} else {
			/*
			* If the current match is before period_len after the last one
			* (first character of this match is before the last character of the last match):
			* Since kmp_period is not periodic (otherwise, there was a shorter period),
			* if we have an ovelapping matches, then there is no way that we will have another
			* match, period_len character after the last one (because it would imply shorter period).
			* So we know we can ignore the last match, and continue with this one.
			*
			* If the current match is after period_len after the last one:
			* It means that period_len after the last match, there was no match,
			* so we can, again, ignore the last match and continue with this one.
			*/
			bg->current_n_kmp_period = 1;
		}
		bg->last_kmp_period_match_pos = bg->current_pos;
		//printf("kmp -- period match, now number of matches = %d\n", bg->current_n_kmp_period);
	}
	// Now, bg->current_n_kmp_period is update to this char
	int kmp_remaining_match;
	if (bg->kmp_remaining) {
		kmp_remaining_match = kmp_read_char(bg->kmp_remaining, c);
		//if (kmp_remaining_match) printf("kmp -- remaining match\n");
	} else {
		kmp_remaining_match = 1; // If there is no remaining, we treat it as 'found'
	}

	if (kmp_remaining_match && bg->current_n_kmp_period == bg->n_kmp_period) {
		// If there is a full match of the first stage
		// (we have all periods and remaining OR we have all periods and no remaining)
		//printf("full kmp match!\n");
		bg->current_n_kmp_period--;
		return 1;
	}
	return 0;
}

/**
* Read char and return whether we have a match.
*
* @param bg	The BGStruct of the pattern we want to search
* @param c	The new character from the stream
*
* @return		1 if found pattern, 0 if not
*/
int bg_read_char(BGStruct* bg, char c) {
	if (bg->flags & BG_SHORT_PATTERN_FLAG) {
		return kmp_read_char(bg->kmp_period, c);
	}
    int ret = 0;
    bg->current_fp = calc_fp_from_prefix_suffix(bg->current_fp,(fingerprint_t)c, &bg->current_r, bg->p);
    bg->last_fps[bg->current_pos % bg->logn] = bg->current_fp;
	if (_bgps_check_first_stage(bg, c)) {
		pos_t vo_pos = bg->current_pos - (1 << bg->first_stage) + 1; // current position - the size of the first stage + 1
		FieldVal vo_r;
		field_div(&vo_r, &bg->current_r, &bg->first_stage_r, bg->p);
		fingerprint_t vo_fp = calc_fp_prefix(bg->current_fp, bg->fps[0], &vo_r, bg->p);
		int resp = _bgps_add_vo(&bg->vos[0], vo_pos, vo_fp, &vo_r, bg->p);
		//printf("response of add vo (first stage) is %d\n", resp);
		if (!resp) {
			LOG("Fingerprint Collision, possibly match from %llu to %llu\n", bg->vos[0].first.pos + bg->n, vo_pos + bg->n);
		}
		else if (resp == 2) {
			// if this stage is the last one / before last one
			if (N_STAGES(bg) == 1) {
				bg->flags |= BG_HAVE_LAST_STAGE_FLAG;
			}
			else if (N_STAGES(bg) == 2 && bg->flags & BG_NEED_BEFORE_LAST_STAGE_FLAG) {
				bg->flags |= BG_HAVE_BEFORE_LAST_STAGE_FLAG;
			}
		}
	}
	if (_bgps_check_last_stages(bg)) {
		ret = 1;
	}
    _bgps_vo_stage_upgrade(bg, bg->current_stage);
    MOD_DEC(bg->current_stage, N_STAGES(bg));
    field_mul(&bg->current_r, &bg->current_r, &bg->r, bg->p);
    bg->current_pos++;
    return ret;
}

/**
* Free memory of BGStruct
*
* @param bg	the BGStruct to free
*/
void bg_free(BGStruct* bg) {
	if (!bg) return;
    if (bg->fps) free(bg->fps);
    if (bg->vos) free(bg->vos);
    if (bg->kmp_period) free(bg->kmp_period);
    if (bg->kmp_remaining) free(bg->kmp_remaining);
    if (bg->fps) free(bg->fps);
    free(bg);
}

/*
================================= F O R     T E S T I N G ================================
*/

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

//============================== F O R      T E S T I N G ======================
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