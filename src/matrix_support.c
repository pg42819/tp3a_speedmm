#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include "csvhelper.h"
//#include "matrix_config.h"
#include "matrix_support.h"
#include "papi_support.h"
#include <omp.h>

#define OPT_SILENT 299
#define OPT_IDENTITY 300
#define OPT_TEST_EQUAL_COLS 302
#define OPT_TEST_REVERSE_ROWS 303
#define OPT_GIGA 304

#define L3_CACHE_MIB 30
// flag set by --quiet or -q

/**
 * Convert the omp schedule kind to an int for easy graphing and
 * handle the OMP 4.5 introduction of monotonic for static by returning zero.
 *
 * @param chunk_size pointer to an int to hold the chunk size
 * @return an integer representing the OpenMP schedule kind:
 *      0 = monotonic (OMP 4.5+)
 *      1 = static
 *      2 = dynamic (default)
 *      3 = guided
 *      4 = auto
 */
#ifdef MATRIX_OMP
int omp_schedule_kind(int *chunk_size)
{
    int chunk_s = -1; // create our own in case chunk_size is a null pointer
    enum omp_sched_t kind = omp_sched_static;
    omp_get_schedule(&kind, &chunk_s);

//    printf("kind   : %d\n", kind);
//    printf("omp_sched_static    : %d\n", omp_sched_static);
//    printf("omp_sched_dynamic   : %d\n", omp_sched_dynamic);
//    printf("omp_sched_guided    : %d\n", omp_sched_guided);
//    printf("omp_sched_auto      : %d\n", omp_sched_auto);
//    printf("omp_sched_monotonic : %d\n", omp_sched_monotonic);
    // allow for chunk_size null if we don't care about it otherwise assign it
    if (chunk_size != NULL) {
        *chunk_size = chunk_s;
    }

    if ((int)kind < -1) {
        // on mac os the OMP_SCHEDULE variable value "static" results in -2147483647 (-MAX_INT)
        // which is probably meant to match the omp_sched_monotonic enum but misses by 1
        // But we switch it for 1 anyway to simulate static on the Linux SEARCH server which is
        // where this program is finally run anyway (the Mac OS run is just for dev/debug)
        // Note that monotonic was introduced in OpenMP 4.5
        return 1;
    }
    return (int)kind;
}

void omp_debug(char *msg) {
    int chunks = -1;
    int kind = omp_schedule_kind(&chunks);
    printf("%s: OMP schedule kind %d with chunk size %d on thread %d of %d\n",
           msg, kind, chunks, omp_get_thread_num(), omp_get_num_threads());
}
#endif
// simple doubleing point matrices
int matrix_size(int n) {
    return n * n * sizeof(float);
}

/**
 * Print headers for output CSV files
 * @param out file pointer for output
 * @param headers array of strings
 * @param dimensions number of strings in the array
 */
void print_headers(FILE *out, char **headers, int dimensions) {
    if (headers == NULL) return;

    fprintf(out, "%s", headers[0]);
    for (int i = 1; i < dimensions; ++i) {
        fprintf(out, ",%s", headers[i]);
    }
    // add a 3rd header called "Cluster" to match the Knime output for easier comparison
    fprintf(out, ",Cluster\n");
}

/**
 * Print the headers for the metrics table to a file pointer.
 * Used for the first run to use a metrics file to produce the header row
 *
 * @param out file pointer
 */
void print_metrics_headers(FILE *out, size_t num_events, int event_codes[num_events])
{
    char flops_prefix= config->giga ? 'G' : '_';
    fprintf(out, "label,size,total_micro_seconds,FLOPs,%cFLOPs_per_second,order_name,block_size,"
                 "max_threads,omp_schedule,omp_chunk_size,"
                 "test_results,", flops_prefix);
    print_papi_headers(out, num_events, event_codes);
    fprintf(out, "\n");
}

/**
 * Print the results of the run with timing numbers in a single row to go in a csv file
 * @param out output file pointer
 * @param metrics metrics object
 */
void print_metrics(FILE *out, struct metrics *metrics,
                   size_t num_events, long long event_values[num_events])
{
    char *test_results = "untested";
    switch (metrics->test_result) {
        case 1:
            test_results = "passed";
            break;
        case -1:
            test_results = "FAILED!";
            break;
    }

    char *order_name = loop_order_name(metrics->loop_order);

    fprintf(out,
            "%s,%d,%lld,%ld,%f,%s,%d,%d,%d,%d,%s," ,
            metrics->label,
            metrics->size,
            metrics->total_micro_seconds,
            metrics->flops,
            metrics->flops_per_second,
            order_name,
            metrics->block_size,
            metrics->omp_max_threads, metrics->omp_schedule_kind, metrics->omp_chunk_size,
            test_results);
    print_papi_events(out, num_events, event_values);
    fprintf(out, "\n");
}

/**
 * Read matrix from the CSV file into the given matrix
 *
 * @param csv_file file pointer to the input file
 * @param matrix pre-allocated array of points into which to read the file
 * @param max_points max number of points to read
 * @param headers if not null, pre-allocated string array to hold the headers
 * @param dimensions number of headers
 *
 * @return number of actual rows read from the file
 */
int read_csv(FILE* csv_file, double matrix[][N])
{
    char *line;
    int i = 0;
    while ((line = csvgetline(csv_file)) != NULL) {
        int num_fields = csvnfield(); // fields on the line
        if (num_fields != N) {
            ERROR("%d values found on line. The file must contain square matrix of size %zu: %s", num_fields, N, line);
            return 0;
        }
        if (num_fields < 2) {
            printf("Warning: found non-empty trailing line. Will stop reading rows now: %s", line);
            break;
        }

        if (i > N) {
            printf("Warning: more that %zu rows in file. Ignoring after the first %zu: %s", N, N, line);
        }
        else {
            for (size_t j = 0; j < N; ++j) {
                char *cell = csvfield(j);
                matrix[i][j] = strtof(cell, NULL);
            }
            i++;
        }
    }
    fclose(csv_file);
    return i;
}

/**
 * Fill the given square matrix with a random value
 *
 * @param matrix pre-allocated two dimensional array of doubles for the matrix
 */
void fill_matrix_random(double matrix[][N])
{
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            matrix[i][j] = ((double) rand()) / ((float) RAND_MAX);
        }
    }
}

/**
 * Fill the given square matrix with the provided double value
 *
 * @param matrix pre-allocated two dimensional array of doubles for the matrix
 * @param value value to put in every cell of the matrix
 */
void fill_matrix_constant(double matrix[][N], double value)
{
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            matrix[i][j] = value;
        }
    }
}

/**
 * Fill the given square matrix as an identity matrix such that A . I = A
 *
 * Useful for testing the multiplication and writing unchanged matrices originals
 *
 * @param matrix pre-allocated two dimensional array of doubles for the matrix
 */
void fill_matrix_identity(double matrix[][N])
{
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            double value = i == j ? 1.0 : 0; // diagonal = 1.0, others = 0.0`
            matrix[i][j] = value;
        }
    }
}

/**
 * Write the matrix of to a file pointer (may be stdout)
 *
 * @param matrix two dimensional array of doubles for the matrix
 * @param out file pointer for output
 */
void write_matrix(FILE *out, char *label, char sep, double matrix[][N])
{
    if (label != NULL) {
        fprintf(out, "\nMatrix %s (%dx%d)\n", label, (int)N, (int)N);
    }
    for (size_t i = 0; i < N; ++i) {
        if (i > 0) {
            fprintf(out, "\n"); // break line between rows
        }
        for (size_t j = 0; j < N; ++j) {
            if (sep > 0) {
                // format for csv - no whitespace
                if (j > 0) {
                    fprintf(out, "%c%.3f", sep, matrix[i][j]);
                }
                else {
                    fprintf(out, "%.3f", matrix[i][j]); // first cell on row
                }
            }
            else {
                // format for neat alignment
                fprintf(out, "%.3f  ", matrix[i][j]);
            }
        }
    }
    fprintf(out, "\n");
}

/**
 * Print the matrix of to stdout
 *
 * @param matrix two dimensional array of doubles for the matrix
 */
void print_matrix(char *label, double matrix[][N])
{
    write_matrix(stdout, label, -1, matrix);
    printf("\n");
}

void debug_matrix(char *label, double matrix[][N])
{
    if (config->verbose) {
        write_matrix(stdout, label, -1, matrix);
        printf("\n");
    }
}


/**
 *
 * Write a Comma-Separated-Values file holding a matrix  to the specified file path.
 *
 * IF the file exists it is silently overwritten.
 *
 * @param csv_file_name absolute path to the file to be written
 * @param matrix two dimensional array of doubles for the matrix
 * @param out file pointer for output
 */
void write_csv_file(char *csv_file_name, double matrix[][N])
{
    FILE *csv_file = fopen(csv_file_name, "w");
    if (!csv_file) {
        fprintf(stderr, "Error: cannot write to the output file at %s\n", csv_file_name);
        exit(1);
    }
    write_matrix(csv_file, NULL, ',', matrix);
}

int read_csv_file(char *csv_file_name, double matrix[][N])
{
    FILE *csv_file = fopen(csv_file_name, "r");
    if (!csv_file) {
        fprintf(stderr, "Error: cannot read the input file at %s\n", csv_file_name);
        exit(1);
    }
    return read_csv(csv_file, matrix);
}

/**
 * Create a big enough array to clear all the way to the L3 cache
 */
#pragma optimize("", off)
void clear_caches()
{
    const int size = L3_CACHE_MIB * 1024 * 1024;
    char *c = (char *)malloc(size);
    for (unsigned i = 0; i < size; i++)
        c = 0;
}

void write_metrics_file(char *metrics_file_name, struct metrics *metrics,
                        size_t num_events, int event_codes[num_events],
                        long long event_values[num_events], int failed_codes[num_events])
{
    char *mode = "a"; // default to append to the metrics file
    bool first_time = false;
    if (access(metrics_file_name, F_OK ) == -1 ) {
        // first time - lets change the mode to "w" and append
        fprintf(stdout, "Creating metrics file and adding headers: %s\n", metrics_file_name);
        first_time = true;
        mode = "w";
    }
    FILE *metrics_file = fopen(metrics_file_name, mode);
    if (metrics_file == NULL) {
        ERROR("Could not create or find a file to store metrics at %s. Does the dir exist?", metrics_file_name);
        exit(1);
    }

    if (first_time) {
        print_metrics_headers(metrics_file, num_events, event_codes);
    }
    print_metrics(metrics_file, metrics, num_events, event_values);
}

/**
 * Compares the columns to validate that they are all the same.
 *
 * This will be true for a matrix C , if C = A . J
 * where J is a matrix-of-ones (every value is 1.0)
 *
 * The method returns -1 after the first failure.
 *
 * @return 1 or -1 if the columns all match
 */
int test_equal_cols(struct config *config, double matrix[][N])
{
    int result = 1;
    for (size_t i = 0; i < N; ++i) {
        // start at col 1 not 0 so we can compare with j-1
        for (size_t j = 1; j < N; ++j) {
            double diff = matrix[i][j] - matrix[i][j-1];
            if (diff > FAILURE_THRESHOLD) {
                if (!config->silent) {
                    fprintf(stderr, "Test failure: result[%zu][%zu] %f does not match result[%zu][%lu]: %f (diff: %f)\n",
                            i, j, matrix[i][j], i, j-1, matrix[i][j-1], diff);
                }
                result = -1;
                break; // give up comparing at first failure in this row - but do the other rows
            }
        }
    }
    if (result > 0) {
        INFO("Successful test of matching columns");
    }
    else {
        INFO("Failed! Columns do not match");
        if (config->verbose) {
            print_matrix("Non-matching cols:", matrix);
        }
    }
    return result;
}

/**
 * Compares the rows to validate that they are all the same.
 *
 * This will be true for a matrix C , if C = J . A
 * where J is matrix-of-ones (every value is 1.0)
 *
 * The method returns -1 after the first failure.
 *
 * @return 1 or -1 if the rows all match
 */
int test_equal_rows(struct config *config, double matrix[][N])
{
    int result = 1;
    // start at row 1 not 0 so we can compare with i-1
    for (size_t i = 1; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            double diff = matrix[i][j] - matrix[i-1][j];
            if (diff > FAILURE_THRESHOLD) {
                if (!config->silent) {
                    fprintf(stderr, "Test failure: result[%zu][%zu] %f does not match result[%lu][%zu]: %f (diff: %f)\n",
                            i, j, matrix[i][j], i-1, j, matrix[i-1][j], diff);
                }
                result = -1;
                break; // give up comparing at first failure in this row - but do the other rows
            }
        }
    }
    if (result > 0) {
        INFO("Successful test of matching rows");
    }
    else {
        INFO("Failed! Rows do not match");
        if (config->verbose) {
            print_matrix("Non-matching rows:", matrix);
        }
    }
    return result;
}

/**
 * Compares the matrix against test file.
 *
 * If every cell in the matrix has an equal value at the same coordinates in the
 * test matrix from the test file, then 1 is returned, indicating a success
 * otherwise -1 is returned indicating a failure.
 *
 * Note that the test file may have more rows than the dataset - trailing rows are ignored
 * in this case - but if it has fewer rows, this is considered a test failure.
 *
 * The method returns -1 after the first failure.
 *
 * @return 1 or -1 if the files match
 */
int test_results(struct config *config, char *test_file_name, double matrix[][N])
{
    int result = 1;
    double (*test_matrix)[N] = malloc(N * N * sizeof(double));
    int test_matrix_size = read_csv_file(test_file_name, test_matrix);
    if (test_matrix_size < (int)N) {
        if (!config->silent) {
            fprintf(stderr, "Test failed. The test matrix has %d rows whereas the produced matrix has %zu",
                            test_matrix_size, N);
        }
        result = 1;
    }
    else {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                double diff = test_matrix[i][j] - matrix[i][j];
                if (diff > FAILURE_THRESHOLD) {
                    if (!config->silent) {
                        fprintf(stderr, "Test failure: result[%zu][%zu] %f does not match expected: %f (diff: %f)\n",
                                i, j, matrix[i][j], test_matrix[i][j], diff);
                    }
                    result = -1;
                    break; // give up comparing at first failure in this row - but do the other rows
                }
            }
        }
    }
    if (result < 0 && config->verbose) {
        print_matrix("Expected", test_matrix);
        print_matrix("Actual", matrix);
    }
    return result;
}

