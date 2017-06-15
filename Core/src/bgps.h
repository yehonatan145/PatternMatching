/**
* Breslauer-Galil algorithm for searching pattern in a stream.
*
*/
#ifndef BGPS_H
#define BGPS_H

/******************************************************************************************************
*       INCLUDES:
******************************************************************************************************/

#include "Fingerprint.h"
#include "kmprt.h"

/******************************************************************************************************
*		DEFINITIONS:
******************************************************************************************************/

typedef unsigned long long pos_t;
#define LOG printf

/*
Because we check each stage every log(n) characters, we might not report in time on the last one.
In order to solve this, whenever we have a VO in the last stage (and in 1-before-last in case that the length
difference between the last and 1-before-last stages is smaller than log(n)), we need to check them every char.
*/
#define BG_HAVE_LAST_STAGE_FLAG				0x1 // States that we have VO in last stage
#define BG_HAVE_BEFORE_LAST_STAGE_FLAG		0x2 // States that we have VO in 1-before-last stage
#define BG_NEED_BEFORE_LAST_STAGE_FLAG		0x4 // States whether we need to check 1-before-last stage
                                            	// (the langth difference between the last stages is smaller than log(n))

#define BG_SHORT_PATTERN_FLAG				0x8 // when the pattern is too short, we just use kmp real-time version instead
#define BG_SHORT_PATTERN_LENGTH 8



/*
struct for saving information about a position in the stream.
This could be used to save info from the start of the stream, or from another position.
*/
typedef struct {
	FieldVal r;
	pos_t pos;
	fingerprint_t fp; // The fingerprint until pos NOT INCLUDE pos ITSELF
} PosInfo;

/*
struct for saving linear progression of VOS in the same stage
*/
typedef struct {
    PosInfo first; // The information about the first char of the first VO from the start of the stream
    PosInfo step; // The information about the step from the first VO to the second VO
    /*
    * first.pos is the position of the first character of the first VO
    * first.fp is the fingerprint of stream[0..first.pos-1] (NOT INCLUDE first.pos ITSELF)
    * first.r is r^first.pos
    *
    * step.pos is the distance between the two first VOs (i.e. second.pos - first.pos)
    * step.fp is the fingerprint of the stream between the two VOs (i.e. stream[first.pos..second.pos-1])
    * step.r is r^step.pos
    */
    int n; // The number of VOs
} VOLinearProgression;

/*
struct for holding all the information about the pattern that we need to know
*/
typedef struct {
	

	FieldVal r;
	FieldVal current_r;
	FieldVal first_stage_r; // r^(length(first_stage) - 1)  (= r^(2^first_stage - 1)) )
		
	pos_t current_pos;
	pos_t last_kmp_period_match_pos; // The END position of the last match of kmp_period

	fingerprint_t current_fp; // The current fingerprint of all the text until now

	field_t p; // Size of field

	fingerprint_t 		*fps; // The array of fingerprints of every stage (from first_stage to stage logn-1)
	VOLinearProgression	*vos; // VOS array (last stage is all pattern)

	KMPRealTime			*kmp_period; // kmp struct for the period of first stage (or for all pattern on case of short pattern)
	KMPRealTime			*kmp_remaining; // kmp struct for the remaining of first stage (after the periods)
	fingerprint_t		*last_fps; // Saves last logn figerprints

	int n;
	int flags;
	int logn;
	int loglogn; // ceil(log(log(n))) + 1
	int first_stage; // The first stage
	int current_stage; // This is the index in vos, the real stage is current_stage + first_stage
	int n_kmp_period; // The number of periods that there are in first stage
	int current_n_kmp_period;
} BGStruct;

// Calculate number of stage given BGStruct
// N_STAGES is the number of VO-stages, the number of fps stored is N_STAGES+1, 
// because we need to save the fingerprint of the all pattern
#define N_STAGES(x) ((x)->logn - (x)->first_stage)

/******************************************************************************************************
*		FUNCTIONS TO EXTERN:
******************************************************************************************************/

// TODO think of way to get rid of p
BGStruct* bg_new(char* pattern, int n, field_t p);
int bg_read_char(BGStruct* bg, char c);
void bg_free(BGStruct* bg);

#endif /* BGPS_H */
