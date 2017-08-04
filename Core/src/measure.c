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

// PERF_BUF_SIZE should be enough to contain ReadFormat when having N_PERF_EVENTS values
#define PERF_BUF_SIZE (sizeof(uint64_t) + (sizeof(uint64_t) * 2) * N_PERF_EVENTS + 2)

// The size of the buffer for the stream files
#define STREAM_BUFFER_SIZE (100 * 1024)

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
* Initialize the perf events measurements
*
* @param fds       The fds array to initialize to the file-descriptors of the perf events
* @param ids       The IDs that should correspond to the fds
*/
void init_perf_events(int fds[N_PERF_EVENTS], uint64_t ids[N_PERF_EVENTS]) {
	struct perf_event_attr pea;
	size_t i;

	fill_perf_event_attr(&pea, perf_events[0].type, perf_events[0].config);
	fds[0] = perf_event_open(&pea, 0, -1, -1, 0);
	ioctl(fds[0], PERF_EVENT_IOC_ID, &ids[0]);
	for (i = 1; i < N_PERF_EVENTS; ++i) {
		fill_perf_event_attr(&pea, perf_events[i].type, perf_events[i].config);
		fds[i] = perf_event_open(&pea, 0, -1, fds[0], 0);
		ioctl(fds[i], PERF_EVENT_IOC_ID, &ids[i]);
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
void measure_success_rate(SuccessRate* suc_rate, pattern_id_t algo_results[], pattern_id_t real_results[], ssize_t n) {
	ssize_t i;
	pattern_id_t real, algo;
	for (i = 0; i < n; ++i) {
		real = real_results[i];
		algo = algo_results[i];
		printf("on character %d: real = %ld, algo = %ld\n", i, real, algo);
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
* Run the mps instance while measuring all aspects of the instance to be measured.
*
* Put the resulted measurements in given parameter
*
* @param inst      The mps instance to run
* @param stats     Where to put the measurements that were measured
*/
void measure_single_instance_stats(MpsInstance* inst, InstanceStats* stats, Conf* conf) {
	printf("start meaure instance, algo = %d\n", inst->algo);
	int fds[N_PERF_EVENTS];
	uint64_t ids[N_PERF_EVENTS];
	size_t i;
	uint64_t index;
	char perf_buf[PERF_BUF_SIZE];
	ReadFormat *read_stats = (ReadFormat*)perf_buf;
	char stream_buffer[STREAM_BUFFER_SIZE];
	pattern_id_t algo_results[STREAM_BUFFER_SIZE];
	pattern_id_t real_results[STREAM_BUFFER_SIZE];
	size_t n_stream_files = conf->n_stream_files;
	char** stream_files = conf->stream_files;


	memset(stats, 0, sizeof(InstanceStats));
	init_perf_events(fds, ids);

	// start the measuring
	printf("start the measuring\n");
	ioctl(fds[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
	for (i = 0; i < n_stream_files; ++i) {
		size_t j;
		ssize_t len_read;
		pattern_id_t (*read_char_func)(void*, char) = mps_table[inst->algo].read_char;
		void* obj = inst->obj;
		pattern_id_t (*reliable_read_char)(void*, char) = mps_table[conf->reliable_mps_instance.algo].read_char;
		void* reliable_obj = conf->reliable_mps_instance.obj;
		int fd;

		printf("before reseting algos\n");
		mps_table[conf->reliable_mps_instance.algo].reset(reliable_obj);
		printf("after reseting reliable\n");
		mps_table[inst->algo].reset(obj);
		printf("after reseting algos\n");
		fd = open(stream_files[i], O_RDONLY);
		printf("after opening file %s, fd = %d\n", stream_files[i], fd);
		do {
			len_read = read(fd, stream_buffer, STREAM_BUFFER_SIZE);

			// perform the alogithm on the stream buffer and measure it
			printf("before algo perf\n");
			ioctl(fds[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
			for (j = 0; j < len_read; ++j) {
				algo_results[j] = read_char_func(obj, stream_buffer[j]);
			}
			ioctl(fds[0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
			printf("after algo perf\n");

			// perform the raliable alogirthm to discover real results, and measure success rate
			for (j = 0; j < len_read; ++j) {
				real_results[j] = reliable_read_char(reliable_obj, stream_buffer[j]);
			}
			measure_success_rate(&stats->suc_rate, algo_results, real_results, len_read);
		} while (len_read == STREAM_BUFFER_SIZE);
		close(fd);
	}
	
	// read the results
	printf("reading measurements results\n");
	read(fds[0], perf_buf, PERF_BUF_SIZE);
	for (i = 0; i < N_PERF_EVENTS; ++i) {
		index = find_index_of_id(read_stats, ids[i]);
		if (index != read_stats->nr) {
			stats->perf_stats[i] = read_stats->values[index].value;
		}
	}
	stats->total_mem = mps_table[inst->algo].total_mem(inst->obj);
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
	// TODO implement
	for (int i = 0; i < conf->n_mps_instances; ++i) {
		InstanceStats* is = &conf->mps_instances_stats[i];
		printf("algo %d:\n", i);
		printf("  total memory: %lu\n", is->total_mem);
		printf("  suc = %lu; false_pos = %lu; false_neg = %lu; partial = %lu\n",
			is->suc_rate.success, is->suc_rate.false_pos, is->suc_rate.false_neg, is->suc_rate.partial_suc);
		printf("  perf events:\n");
		for (int j = 0; j < N_PERF_EVENTS; ++j) {
			printf("    %s : %lu\n", perf_events[j].desc, is->perf_stats[j]);
		}
	}
	
}