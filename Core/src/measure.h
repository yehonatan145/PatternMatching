#ifndef MEASURE_H
#define MEASURE_H

#include "mps.h"
#include <stdint.h>
#include <linux/perf_event.h>

struct _Conf;

typedef struct {
	uint32_t   type;
	uint64_t   config;
	char      *desc; // description of the event type and config
} PerfEventType;

// The events to meaure and their short description
static PerfEventType perf_events[] = {
                {PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS, "page faults"},
                {PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, "number of instructions"}
                };

#define N_PERF_EVENTS (sizeof(perf_events) / sizeof(PerfEventType))

/**
* Struct for saving the success rete of the algorithm
*
* All the members are the number of characters that had their name
* (e.g. false_pos is the number of characters that were false positive)
*/
typedef struct {
	size_t success;     // the algo return the longest pattern that match
	size_t false_pos;   // the algo return pattern that isn't realy matching
	size_t false_neg;   // the algo return that there isn't any match, even though there is
	size_t partial_suc; // the algo return a pattern that is a match, but not the longest one that match
} SuccessRate;

typedef struct instance_stats {
	uint64_t perf_stats[N_PERF_EVENTS];
	SuccessRate suc_rate;
	size_t total_mem;
} InstanceStats;

void measure_instances_stats(struct _Conf* conf);
void write_stats_to_file(struct _Conf* conf);

#endif // MEASURE_H