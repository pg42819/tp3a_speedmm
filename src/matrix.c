#include <stdio.h>
#include <stdlib.h>
//#include <omp.h>
#include "matrix.h"
#include "matrix_support.c"

struct config *config;

/**
 * Perform a dot-multiplication on two square matrices of a given size in the speciied order
 *
 * IMPORTANT: The result matrix must be zeroed for this function to succeed
 *
 * @param order the loop order: one of ijk, kij, or kji
 * @param matrix1 left-side of the dot-multiplication
 * @param matrix2 right-side of the dot-multiplication
 * @param result preallocated zeroed matrix into which to store the results
 */
extern long dot_multiply_matrices(enum loop_order order, double matrix1[][N], double matrix2[][N], double result[][N]);

long measurable_work(double matrix_a[][N], double matrix_b[][N], double result[][N])
{
    long counted_flops = 0;
    DEBUG("Multiplying matrices");
    counted_flops= dot_multiply_matrices(config->loop_order, matrix_a, matrix_b, result);
    DEBUG("Matrix multiplication involved %ld FLOPs", counted_flops);

    switch (counted_flops) {
        case MATRIX_NOT_YET_IMPLEMENTED:
            ERROR("Matrix operation not yet implemented");
            exit(1);
        case MATRIX_FAILED:
            ERROR("Matrix operation failed");
            exit(1);
    }

    return counted_flops;
}

void measure(struct metrics *metrics, long long start_time, long long stop_time, long flops)
{
    metrics->total_micro_seconds = stop_time - start_time;
    DEBUG("Stop - Start   = %lld microseconds", metrics->total_micro_seconds);
    metrics->flops = flops;
}

/**
 * Loop over the sets of papi event names in papi_arg and run the matrix multiplication, collectiong overall metrics
 */
int run_timer_loops(unsigned timer_loop_count, struct metrics *metrics,
        double matrix_a[][N], double matrix_b[][N], double dot_product[][N])
{
    DEBUG("Running %u timer loops over matrix calculations", timer_loop_count);
    for (unsigned work_loop_index = 0; work_loop_index < timer_loop_count; work_loop_index++) {
        fill_matrix_constant(dot_product, 0.0f);
        // start papi counters
        long long start_time = get_papi_time();
        DEBUG("Started PAPI at: %lld microseconds", start_time);

        // do the work of multiplying
        long counted_flops = measurable_work(matrix_a, matrix_b, dot_product);

        // stop the counters
        long long stop_time = get_papi_time();
        DEBUG("Stopped PAPI at: %lld microseconds", stop_time);

        measure(metrics, start_time, stop_time, counted_flops);
    }
    return 0;
}

/**
 * Loop over the sets of papi event names in papi_arg and run the matrix multiplication, collectiong overall metrics
 */
int run_papi_loops(char* papi_arg, struct metrics *metrics, int *event_codes, long long *papi_results, int *failed_codes,
                   double matrix_a[][N], double matrix_b[][N], double dot_product[][N])
{
    int total_event_count = 0;
    int event_set = 0;
    unsigned subset_start = 0;
    char *subsets[MAX_PAPI_CODES];
    unsigned subset_count = split_string(papi_arg, "!", subsets);
    for (unsigned work_loop_index = 0; work_loop_index < subset_count; work_loop_index++) {
        fill_matrix_constant(dot_product, 0.0f);
        clear_caches(); // clear caches betweeen each run by filling them
        bool measure_time = (work_loop_index == 0); // measure time on the first run only
        char *subset_arg = subsets[work_loop_index];
        DEBUG("\nLoop #%u : PAPI subset: %s", work_loop_index, subset_arg);
        subset_start = total_event_count;
        char *sub_events[MAX_PAPI_CODES];
        unsigned sub_events_count = split_string(subset_arg, ":", sub_events);
        for (unsigned event_j = 0; event_j < sub_events_count; event_j++) {
            event_codes[total_event_count] = papi_code(sub_events[event_j]);
            DEBUG("event %u in subsset %u is %s with code: %#010x", event_j, work_loop_index,
                  sub_events[event_j], event_codes[total_event_count]);
            total_event_count++;
        }
        DEBUG("About to papi init events start:%u size:%u for subset:%s", subset_start, sub_events_count,
              subset_arg);
        event_set = init_papi_events(subset_start, sub_events_count, event_codes,
                                     failed_codes); // papi event set handle

        // start papi counters
        long long start_time = start_papi(event_set);
        DEBUG("Started PAPI at: %lld microseconds", start_time);

        // do the work of multiplying
        long counted_flops = measurable_work(matrix_a, matrix_b, dot_product);

        // stop the counters
        long long stop_time = stop_papi(event_set, subset_start, papi_results);
        DEBUG("Stopped PAPI at: %lld microseconds", stop_time);

        if (measure_time) {
            measure(metrics, start_time, stop_time, counted_flops);
        }
    }
    return total_event_count;
}

int main(int argc, char* argv []) {
    struct config local_config = parse_cli(argc, argv);
    // simplify config.size to n
    config = &local_config;

    // names for use in output messages
    char *a_desc = "A";
    char *b_desc = "B";

    // declare and allocated the 3 matrices
    DEBUG("Allocating 3 %zu x %zu double arrays for matrices A, B and results", N, N);
    double (* matrix_a)[N] __attribute__ ((aligned (32)))= malloc(N * N * sizeof(double));
    double (* matrix_b)[N] __attribute__ ((aligned (32))) = malloc(N * N * sizeof(double));
    double (* dot_product)[N] __attribute__ ((aligned (32))) = malloc(N * N * sizeof(double));

    DEBUG("Filling result matrix with zeros");
    // fill result with zero to eliminate that from matrix measurements
    fill_matrix_constant(dot_product, 0.0f);

    if (config->in_file) {
        char *csv_file_name = valid_file('f', config->in_file);
        INFO("Reading matrix A from %s", config->in_file);
        a_desc = "from file";
        read_csv_file(csv_file_name, matrix_a);
        INFO("Finished reading matrix A from %s", config->in_file);
    }
    else {
        INFO("Generating random data for matrix A");
        a_desc = "random";
        fill_matrix_random(matrix_a);
        INFO("Finished generating random data for matrix A");
    }

    if (config->identity) {
        INFO("Using identity matrix for matrix B (so A . B = A . I = A)");
        b_desc = "identity";
        fill_matrix_identity(matrix_b);
    }
    else {
        INFO("Using 1.0-filled matrix data for matrix B");
        b_desc = "all 1.0";
        fill_matrix_constant(matrix_b, 1.0f);
    }

    struct metrics metrics = new_metrics(config);
    metrics.size = N;
    metrics.omp_max_threads = 0; //omp_get_max_threads();
    // get kind: dynamic, static, auto.. and the chunk size
    metrics.omp_schedule_kind = 0;//omp_schedule_kind(&metnrics.omp_chunk_size);

    init_papi();

    int event_codes[MAX_PAPI_CODES];
    int failed_codes[MAX_PAPI_CODES];
    // prepare an array to store the results
    long long papi_results[MAX_PAPI_CODES];
    unsigned total_event_count = 0;

    if (config->papi_arg) {
        total_event_count = run_papi_loops(config->papi_arg, &metrics, event_codes, papi_results, failed_codes,
                                           matrix_a, matrix_b, dot_product);
    }
    else {
        // timer loops = config->timer_loops
        run_timer_loops(1, &metrics, matrix_a, matrix_b, dot_product);
    }

    update_metrics(&metrics);

// output file is not always written: sometimes we only run for metrics and compare with test data
    if (config->out_file) {
        INFO("Writing output to %s", config->out_file);
        write_csv_file(config->out_file, dot_product);
    }

    if (config->verbose) {
        char a_label[200];
        char b_label[200];
        sprintf(a_label, "A (%s)", a_desc);
        sprintf(b_label, "B (%s)", b_desc);
    }

    if (config->test_file) {
        char* test_file_name = valid_file('t', config->test_file);
        INFO("Comparing results against test file: %s", config->test_file);
        metrics.test_result = test_results(config, test_file_name, dot_product);
    }

    if (config->metrics_file) {
        // metrics file may or may not already exist
        INFO("Reporting metrics to: %s", config->metrics_file);
        write_metrics_file(config->metrics_file, &metrics,
                           total_event_count, event_codes, papi_results, failed_codes);
    }

    if (!config->silent) {
        print_metrics_headers(stdout, total_event_count, event_codes);
        print_metrics(stdout, &metrics, total_event_count, papi_results);
        describe_papi_events(total_event_count, event_codes, papi_results, failed_codes);
        printf("\nTime to multiply : %0lld microseconds (%.2f s)\n", metrics.total_micro_seconds, metrics.total_seconds);
        printf("FLOPs counted    : %ld\n", metrics.flops);
        printf("FLOPs/second     : %0f\n", metrics.flops_per_second);
        printf("\n");
        printf("(hide these messages with --silent)\n");
        printf("\n");
    }

    INFO("Matrix run completed");
    return 0;
}


