#include "measure.h"
#include "conf.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <asm/types.h>
#include <sys/syscall.h>
#include <linux/hw_breakpoint.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>

// IMPORTANT: the size of this array should match the macro N_PERF_EVENTS defined in measure.h
/*PerfEventType perf_events[] = {
                {PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS, "page faults"},
                {PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, "number of instructions"}
                };*/

/**
* struct used for returning values from performance files
*/
typedef struct {
	uint64_t nr;
	struct {
		uint64_t value;
		uint64_t id;
	} values[];
} ReadFormat;

// BUF_SIZE should be enough to contain ReadFormat when having N_PERF_EVENTS values
#define BUF_SIZE (sizeof(uint64_t) + (sizeof(uint64_t) * 2) * N_PERF_EVENTS + 1)

/**
* Wrapper function for the perf_event_open syscall
*/
long perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags) {
	int ret;
	ret = syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
	return ret;
}

/**
* Fill the perf_event_attr struct according to the type and config given
*
* @param pea     The pref_event_attr struct to change the attributes in
* @param type    The overall event type
* @param config  Specifies the event, in conjunction with type 
*/
void fill_perf_event_attr(struct perf_event_attr* pea, uint32_t type, uint64_t config) {
	memset(pea, 0, sizeof(struct perf_event_attr));
	pea->type = type;
	pea->size = sizeof(struct perf_event_attr);
	pea->config = config;
	pea->disabled = 1;
	pea->exclude_kernel = 1;
	pea->exclude_hv = 1;
	pea->read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
}

/**
* Find the index of an id in the values returned from perf event file reading
*
* @param rf     The data returned from perf event file reading
* @param id     The id to look after
*
* @return       The index of the value with that id, or rf->nr if none
*/
uint64_t find_index_of_id(ReadFormat* rf, uint64_t id) {
	uint64_t i, nr = rf->nr;

	for (i = 0; i < nr; ++i) {
		if (rf->values[i].id == id) {
			return i;
		}
	}
	return nr;
}

/**
* Run the mps instance while measuring all aspects of the instance to be measured.
*
* Put the resulted measurements in given parameter
*
* @param inst      The mps instance to run
* @param stats     Where to put the measurements that were measured
*/
void measure_single_instance_stats(MpsInstance* inst, InstanceStats* stats) {
	struct perf_event_attr pea;
	int fds[N_PERF_EVENTS];
	uint64_t ids[N_PERF_EVENTS];
	size_t i;
	uint64_t index;
	char buf[BUF_SIZE];
	ReadFormat *read_stats = (ReadFormat*)buf;

	// initialize the perf events
	fill_perf_event_attr(&pea, perf_events[0].type, perf_events[0].config);
	fds[0] = perf_event_open(&pea, 0, -1, -1, 0);
	ioctl(fds[0], PERF_EVENT_IOC_ID, &ids[0]);
	for (i = 1; i < N_PERF_EVENTS; ++i) {
		fill_perf_event_attr(&pea, perf_events[i].type, perf_events[i].config);
		fds[i] = perf_event_open(&pea, 0, -1, fds[0], 0);
		ioctl(fds[i], PERF_EVENT_IOC_ID, &ids[i]);
	}

	// start the measuring
	ioctl(fds[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
	ioctl(fds[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);

	// TODO run algorithm and test results

	// stop measuring, and read results
	ioctl(fds[0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
	read(fds[0], buf, BUF_SIZE);
	for (i = 0; i < N_PERF_EVENTS; ++i) {
		index = find_index_of_id(read_stats, ids[i]);
		if (index != read_stats->nr) {
			stats->perf_stats[i] = read_stats->values[index].value;
		}
	}

	// TODO measure success rate, and enter it to given "stats" parameter
	printf("the total memory for instance algo = %d is %lu\n", inst->algo, mps_table[inst->algo].total_mem(inst->obj));
}

/**
* Run all the mps instances on the streams and measure their statistics
* 
* Put the statistic in the mps_instances_stats member of the configuration struct
*
* @param conf    The configuration with the mps instances (and where to put the statistics)
*/
void measure_instances_stats(Conf* conf) {
	conf->mps_instances_stats = (InstanceStats*)malloc(conf->n_mps_instances * sizeof(InstanceStats));
	size_t i , n_mps_instances = conf->n_mps_instances;
	for (i = 0; i < n_mps_instances; ++i) {
		measure_single_instance_stats(&conf->mps_instances[i], &conf->mps_instances_stats[i]);
	}
}

/**
* Write the statistics to the output file specified in the configuration struct
*
* @param conf    The configuration struct with the statistic and the output file name
*/
void write_stats_to_file(Conf* conf) {
	// TODO implement
}