#ifndef MEASURE_H
#define MEASURE_H


/******************************************************************************
*		INCLUDES
******************************************************************************/


#include "mps.h"
#include <stdint.h>
#include <linux/perf_event.h>


/******************************************************************************
*		DEFINITIONS
******************************************************************************/


struct _Conf;

/**
* struct for perf_event type information, including the event type and a short description
*/
typedef struct {
	uint32_t   type;
	uint64_t   config;
	char      *desc; // description of the event type and config
} PerfEventType;

/**
* struct for group of perf_event types
*
* Linux perf_event interface measure counters in groups, where every group is measured as a unit
* (i.e. the counters in the same group would be counted on the exact same CPU insturctions).
*
* While putting event types in the same groups helps comparing them (again, they were counted on the exact
* same CPU instructions), sometimes the kernel can't put all of them as a unit, making the counter measurement
* unreliable (from my tries, putting software cycles counters with hardware counters, made the software cycles
* counters to return always 0)
*
* For that reason, we divide the perf_event types into seperate groups to measure them simultaneously
* This groups are saved in the static variable perf_events
*/
typedef struct {
	PerfEventType  *events;
	size_t          n;
} PerfEventTypeGroup;

// The events to meaure and their short description
static PerfEventType perf_group_1[] = {
	{PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS, "page faults"},
	{PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_CLOCK, "software cpu clock"},
	{PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK, "software task clock"}
};

static PerfEventType perf_group_2[] = {
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, "number of instructions"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS, "number of branch instructions"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES, "number of cycles"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_BUS_CYCLES, "bus cycles"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES, "total cycles"}
};

static PerfEventTypeGroup perf_events[] = {
	{perf_group_1, 3},
	{perf_group_2, 5}
};

#define N_PERF_GROUPS (sizeof(perf_events) / sizeof(PerfEventTypeGroup))

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

/**
* struct for perf_event group statistics
*
* This struct should match a PerfEventTypeGroup, and the number of elements in perf_stats
* should be its number of events i.e. perf_stats[i] is the statistics for the PerfEventTypeGroup.events[i]
*/
typedef struct {
	uint64_t  *perf_stats;
} PerfEventGroupStats;

/**
* Statistics for an mps (multi-pattern search) instance
*/
typedef struct instance_stats {
	// perf_groups_stats should match perf_events static variable
	PerfEventGroupStats perf_groups_stats[N_PERF_GROUPS];
	SuccessRate suc_rate;
	size_t total_mem;
	clock_t total_cycles;
} InstanceStats;


/******************************************************************************
*		API FUNCTIONS
******************************************************************************/


void measure_instances_stats(struct _Conf* conf);
void write_stats_to_file(struct _Conf* conf);

#endif // MEASURE_H