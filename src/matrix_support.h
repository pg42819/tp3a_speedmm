#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <omp.h>
#include <string.h>
#include "matrix_types.h"

extern void fill_matrix_constant(double matrix[][N], double value);
extern void fill_matrix_identity(double matrix[][N]);
extern void fill_matrix_random(double matrix[][N]);

extern void print_matrix(char *label, double matrix[][N]);
extern void print_metrics_headers(FILE *out, size_t num_events, int event_codes[num_events]);
extern void print_metrics(FILE *out, struct metrics *metrics,
                          size_t num_events, long long event_results[num_events]);

extern int read_csv_file(char *csv_file_name, double matrix[][N]);
extern int read_csv(FILE *csv_file, double matrix[][N]);
extern int test_results(struct config *config, char *test_file_name, double matrix[][N]);

extern void write_csv_file(char *csv_file_name, double matrix[][N]);
extern void write_matrix(FILE *out, char *label, char sep, double matrix[][N]);
extern void write_metrics_file(char *metrics_file_name, struct metrics *metrics,
                               size_t num_events, int event_codes[num_events],
                               long long event_values[num_events], int failed_codes[num_events]);

