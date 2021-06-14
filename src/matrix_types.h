#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Global square matrix size
#define N 4096
// How many rows to report progress on in verbose mode
#define PROGRESS_GRANULARITY 100

#define MAX_SIZE 15
#define DEFAULT_BLOCK_SIZE 0
#define FAILURE_THRESHOLD 0.001

// Error codes to use instead of flops count
#define MATRIX_FAILED -1
#define MATRIX_NOT_YET_IMPLEMENTED -2

// math min, max
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


// order of loops in multiplication
enum loop_order { ijk, ikj, jki };

struct config {
    char *in_file;
    char *out_file;
    char *test_file;
    char *papi_arg; /// comma separated papi event names
    char *metrics_file;
    char *label;
    enum loop_order loop_order;
    int size;
    int block_size;
    bool identity;
    bool silent;
    bool verbose;
    bool debug;
    bool quiet;
    bool giga; // giga flops/second instead of flops/second
    bool papi_ignore;
    bool test_equal_cols;
    bool test_reverse_rows;
};

struct metrics {
    char *label; // label for metrics row from -l command line arg
    long flops; // doubleing point operations
    double total_seconds;         // total time in seconds for the run
    long long total_micro_seconds; // total time in seconds for the run
    double flops_per_second;
    int test_result;     // 0 = not tested, 1 = passed, -1 = failed comparison with expected data
    int size;  // size of square matrices
    int omp_max_threads; // OMP max threads, usually set by OMP_NUM_THREADS env var or an function call
    // Next 2 are OMP schedule kind (static, dynamic, auto) and chunk size, set by OMP_SCHEDULE var.
    // See: https://gcc.gnu.org/onlinedocs/libgomp/omp_005fget_005fschedule.html#omp_005fget_005fschedule
    int omp_schedule_kind;
    int omp_chunk_size;
    enum loop_order loop_order;
    int block_size;
};

#define DEBUG__INT(fmt, ...) if (config->debug) printf("DEBUG " fmt "%s", __VA_ARGS__);
#define DEBUG(...) DEBUG__INT(__VA_ARGS__, "\n")
//#define DEBUG(...) (void)0

#define INFO__INT(fmt, ...) if (config != NULL && !config->quiet) printf(fmt "%s", __VA_ARGS__)
#define INFO(...) INFO__INT(__VA_ARGS__, "\n")
//#define INFO(...) (void)0
#define VERBOSE__INT(fmt, ...) if (config != NULL && config->verbose) printf(fmt "%s", __VA_ARGS__)
#define VERBOSE(...) VERBOSE__INT(__VA_ARGS__, "\n")
//#define VERBOSE(...) (void)0

#define ERROR__INT(fmt, ...) fprintf(stderr, "ERROR: " fmt "%s", __VA_ARGS__)
#define ERROR(...) ERROR__INT(__VA_ARGS__, "\n")
//#define ERROR(...) (void)0
