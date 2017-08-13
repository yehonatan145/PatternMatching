/**
* Measuring Multi-Pattern Matching Algorithms performance, and success rate.
*
* The performance measurements are defined in the "perf_events" static variable which defined
* in the header file of this file.
*
* We read the stream files in blocks of size STREAM_BUFFER_SIZE, and using the algorithm on every block
* (so the stream to read would be in memory and not in file, because reading from file change performance)
*/


/******************************************************************************
*		INCLUDES
******************************************************************************/


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


/******************************************************************************
*		DEFINITIONS
******************************************************************************/


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

/**
* The data needed for a perf event
* This should match a PerfEventType
*/
typedef struct {
	uint64_t id; 
	int      fd;
} PerfEventData;

/**
* The data for perf events group
* This should match a PerfEventTypeGroup (i.e. the PerfEventGroupData.events[i] should match
* PerfEventTypeGroup.events[i], and be with the same number of elements)
*/
typedef struct {
	PerfEventData  *events;
} PerfEventGroupData;

// PERF_BUF_SIZE should be enough to contain ReadFormat
#define PERF_BUF_SIZE (sizeof(uint64_t) + (sizeof(uint64_t) * 2) * sizeof(perf_events) + 1)

// The size of the buffer for the stream files
#define STREAM_BUFFER_SIZE (100 * 1024)


/******************************************************************************
*		INNER FUNCTIONS
******************************************************************************/


/**
* Wrapper function for the perf_event_open syscall
*/
static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags) {
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
static void fill_perf_event_attr(struct perf_event_attr* pea, uint32_t type, uint64_t config) {
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
* Initialize the perf events measurements
*
* @param data      The data for all the perf events (matches perf_events)
*/
static PerfEventGroupData* create_perf_events_data() {
	PerfEventGroupData* data;
	struct perf_event_attr pea;
	size_t i, j, n;
	PerfEventType* current_type;
	PerfEventData* current_data;

	// Allocate memory for the data
	data = (PerfEventGroupData*)malloc(N_PERF_GROUPS * sizeof(PerfEventGroupData));
	for (i = 0; i < N_PERF_GROUPS; ++i) {
		data[i].events = (PerfEventData*)malloc(perf_events[i].n * sizeof(PerfEventData));
	}

	// Fill the data, according to perf_events
	for (i = 0; i < N_PERF_GROUPS; ++i) {
		n = perf_events[i].n;
		current_type = &perf_events[i].events[0];
		current_data = &data[i].events[0];

		// Create the leader perf event of the i-th group
		fill_perf_event_attr(&pea, current_type->type, current_type->config);
		current_data->fd = perf_event_open(&pea, 0, -1, -1, 0);
		ioctl(current_data->fd, PERF_EVENT_IOC_ID, &current_data->id);

		// Create the rest of the i-th group
		for (j = 1; j < n; ++j) {
			current_type = &perf_events[i].events[j];
			current_data = &data[i].events[j];

			fill_perf_event_attr(&pea, current_type->type, current_type->config);
			current_data->fd = perf_event_open(&pea, 0, -1, data[i].events[0].fd, 0);
			ioctl(current_data->fd, PERF_EVENT_IOC_ID, &current_data->id);
		}
	}
	return data;
}

/**
* Do the ioctl request on every perf_event group in teh data
*
* @param data        The perf_event groups data to work on
* @param request     The ioctl request (e.g. PERF_EVENT_IOC_ENABLE)
*/
static inline void perf_event_data_ioctl(PerfEventGroupData data[N_PERF_GROUPS], unsigned long request) {
	for (size_t i = 0; i < N_PERF_GROUPS; ++i) {
		ioctl(data[i].events[0].fd, request, PERF_IOC_FLAG_GROUP);
	}
}

/**
* Measure the success rate, and add it to the success rate given
*
* @param suc_rate          The success rate already measured
* @param algo_results      The algorithm results
* @param real_results      The real results for that stream
* @param n                 The number of results
*/
static void measure_success_rate(SuccessRate* suc_rate, pattern_id_t algo_results[], pattern_id_t real_results[], ssize_t n) {
	ssize_t i;
	pattern_id_t real, algo;
	for (i = 0; i < n; ++i) {
		real = real_results[i];
		algo = algo_results[i];
		if (real == algo) {
			suc_rate->success++;
		} else if (is_pattern_suffix(algo, real)) {
			suc_rate->partial_suc++;
		} else if (algo == null_pattern_id) {
			suc_rate->false_neg++;
		} else {
			suc_rate->false_pos++;
		}
	}
}

/**
* Find the index of an id in the values returned from perf event file reading
*
* @param rf     The data returned from perf event file reading
* @param id     The id to look after
*
* @return       The index of the value with that id, or rf->nr if none
*/
static size_t find_index_of_id(ReadFormat* rf, uint64_t id) {
	uint64_t i, nr = rf->nr;

	for (i = 0; i < nr; ++i) {
		if (rf->values[i].id == id) {
			return i;
		}
	}
	return nr;
}

/**
* Read the perf_event counters and update the statistics according to it
*
* @param data         The perf_event groups data
* @param stats        The place to put the statistics in
*/
static void read_perf_events_results(PerfEventGroupData data[N_PERF_GROUPS], InstanceStats* stats) {
	char perf_buf[PERF_BUF_SIZE];
	ReadFormat *read_stats = (ReadFormat*)perf_buf;
	size_t i, j, n, index;
	for (i = 0; i < N_PERF_GROUPS; ++i) {
		read(data[i].events[0].fd, perf_buf, PERF_BUF_SIZE);
		n = perf_events[i].n;
		for (j = 0; j < n; ++j) {
			index = find_index_of_id(read_stats, data[i].events[j].id);
			if (index != read_stats->nr) {
				stats->perf_groups_stats[i].perf_stats[j] = read_stats->values[index].value;
			}
		}
	}
}

/**
* Run the mps instance while measuring all aspects of the instance to be measured.
*
* Put the resulted measurements in given parameter
*
* @param inst      The mps instance to run
* @param stats     Where to put the measurements that were measured
*/
static void measure_single_instance_stats(MpsInstance* inst, InstanceStats* stats, Conf* conf) {
	size_t i;
	char stream_buffer[STREAM_BUFFER_SIZE];
	pattern_id_t algo_results[STREAM_BUFFER_SIZE];
	pattern_id_t real_results[STREAM_BUFFER_SIZE];
	size_t n_stream_files = conf->n_stream_files;
	char** stream_files = conf->stream_files;


	memset(stats, 0, sizeof(InstanceStats));
	for (i = 0; i < N_PERF_GROUPS; ++i) {
		stats->perf_groups_stats[i].perf_stats = (uint64_t*)malloc(perf_events[i].n * sizeof(uint64_t));
	}
	PerfEventGroupData* data = create_perf_events_data();

	// start the measuring
	printf("Measuring algorithm %s\n\n", mps_table[inst->algo].name);
	perf_event_data_ioctl(data, PERF_EVENT_IOC_RESET);
	for (i = 0; i < n_stream_files; ++i) {
		size_t j;
		ssize_t len_read;
		// hold the mps functions and object in variables,
		// so we won't need to access extra memory during measurement
		pattern_id_t (*read_char_func)(void*, char) = mps_table[inst->algo].read_char;
		void* obj = inst->obj;
		pattern_id_t (*reliable_read_char)(void*, char) = mps_table[conf->reliable_mps_instance.algo].read_char;
		void* reliable_obj = conf->reliable_mps_instance.obj;
		int fd;

		// Reset the algorithms before start of stream
		mps_table[conf->reliable_mps_instance.algo].reset(reliable_obj);
		mps_table[inst->algo].reset(obj);
		fd = open(stream_files[i], O_RDONLY);
		if (fd == -1) {
			fprintf(stderr, "can't open stream file %s: %s\n", stream_files[i], strerror(errno));
			FatalExit();
		}
		do {
			// Read chunk of size STREAM_BUFFER_SIZE from the stream and measure performance on it
			len_read = read(fd, stream_buffer, STREAM_BUFFER_SIZE);
			if (len_read == -1) {
				fprintf(stderr, "can't read from stream file %s: %s\n", stream_files[i], strerror(errno));
				FatalExit();
			}
			
			perf_event_data_ioctl(data, PERF_EVENT_IOC_ENABLE);
			for (j = 0; j < len_read; ++j) {
				algo_results[j] = read_char_func(obj, stream_buffer[j]);
			}
			perf_event_data_ioctl(data, PERF_EVENT_IOC_DISABLE);

			// perform the raliable alogirthm to discover real results, and measure success rate
			for (j = 0; j < len_read; ++j) {
				real_results[j] = reliable_read_char(reliable_obj, stream_buffer[j]);
			}
			measure_success_rate(&stats->suc_rate, algo_results, real_results, len_read);
		} while (len_read == STREAM_BUFFER_SIZE);
		close(fd);
	}

	printf("Finished measuring algorithm %s\n\n", mps_table[inst->algo].name);
	
	read_perf_events_results(data, stats);
	stats->total_mem = mps_table[inst->algo].total_mem(inst->obj);
}

/******************************************************************************
*		API FUNCTIONS
******************************************************************************/

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
		measure_single_instance_stats(&conf->mps_instances[i],
		                              &conf->mps_instances_stats[i],
		                              conf);
	}
}

/**
* Write the statistics to the output file specified in the configuration struct
*
* @param conf    The configuration struct with the statistic and the output file name
*/
void write_stats_to_file(Conf* conf) {
	// TODO implement, meanwhile print the results
	for (int i = 0; i < conf->n_mps_instances; ++i) {
		InstanceStats* is = &conf->mps_instances_stats[i];
		printf("algo %d - %s:\n", i, mps_table[i].name);
		printf("  total memory: %lu\n", is->total_mem);
		printf("  suc = %lu; false_pos = %lu; false_neg = %lu; partial = %lu\n",
			is->suc_rate.success, is->suc_rate.false_pos, is->suc_rate.false_neg, is->suc_rate.partial_suc);
		printf("  perf events:\n");
		for (size_t j = 0; j < N_PERF_GROUPS; ++j) {
			size_t n = perf_events[j].n;
			for (size_t k = 0; k < n; ++k) {
				printf("    %s : %lu\n", perf_events[j].events[k].desc, is->perf_groups_stats[j].perf_stats[k]);
			}
		}
	}
	
}