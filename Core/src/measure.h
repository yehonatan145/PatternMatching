#ifndef MEASURE_H
#define MEASURE_H

#include "mps.h"

struct _Conf;

typedef struct s_PerfEventType {
	uint32_t   type;
	uint64_t   config;
	char      *desc; // description of the event type and config
} PerfEventType;


// IMPORTANT: the number here should match the size of the array perf_events
// that defined in measure.c
#define N_PERF_EVENTS 2

extern PerfEventType perf_events[N_PERF_EVENTS];

typedef struct s_InstanceStats {
	uint64_t perf_stats[N_PERF_EVENTS];
	double success_rate;
	// TODO add stats
} InstanceStats;

void measure_instances_stats(struct _Conf* conf);
void write_stats_to_file(struct _Conf* conf);

#endif // MEASURE_H