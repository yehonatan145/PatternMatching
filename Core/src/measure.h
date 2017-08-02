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

typedef struct instance_stats {
	uint64_t perf_stats[N_PERF_EVENTS];
	double success_rate;
	// TODO add stats
} InstanceStats;

void measure_instances_stats(struct _Conf* conf);
void write_stats_to_file(struct _Conf* conf);

#endif // MEASURE_H