#ifndef MATRIX_CONFIG
#define MATRIX_CONFIG
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include "matrix_types.h"

#define OPT_SILENT 299
#define OPT_IDENTITY 300
#define OPT_TEST_EQUAL_COLS 302
#define OPT_TEST_REVERSE_ROWS 303
#define OPT_GIGA 304

#define L3_CACHE_MIB 30

extern struct config *config;

/**
 * Initialize a new config to hold the run configuration set from the command line
 */
struct config new_config()
{
    struct config new_config;
    new_config.in_file = NULL;
    new_config.out_file = NULL;
    new_config.test_file = NULL;
    new_config.metrics_file = NULL;
    new_config.test_equal_cols = false;
    new_config.test_reverse_rows = false;
    new_config.identity = false;
    new_config.label = "no-label";
    new_config.size = N;
    new_config.block_size = 0;
    new_config.loop_order = ijk;
    new_config.silent = false;
    new_config.verbose = false;
    new_config.debug = false;
    new_config.quiet = false;
    new_config.papi_arg = NULL;
    new_config.papi_ignore = false;
    return new_config;
}

/**
 * Initialize a new metrics to hold the run performance metrics and settings
 */
struct metrics new_metrics(struct config *config)
{
    struct metrics new_metrics;
    new_metrics.label = config->label;
    new_metrics.loop_order = config->loop_order;
    new_metrics.block_size = config->block_size;
    new_metrics.flops = 0;
    new_metrics.total_seconds = 0;
    new_metrics.total_micro_seconds = 0;
    new_metrics.flops_per_second = 0;
    new_metrics.test_result = 0; // zero = no test performed
    new_metrics.omp_max_threads = -1;
    new_metrics.omp_schedule_kind = -1; // if we see -1 then the kind was not fetched
    return new_metrics;
}

/**
 * Update the metrics properties calculated from others
 */
void update_metrics(struct metrics *metrics)
{
    if (metrics->total_micro_seconds == 0) {
        DEBUG("Zero microseconds passed, so setting FLOPS/sec = -1 for infinity");
        metrics->flops_per_second = -1; // for infinity
    }
    else {
        metrics->total_seconds = ((double)metrics->total_micro_seconds / 1000000.0f);
        if (config->giga) {
            DEBUG("GFLOPs/sec calculated:  %ld / 1000 * %lld = %f", metrics->flops, metrics->total_micro_seconds,
                  metrics->flops_per_second);
            metrics->flops_per_second = ((double) metrics->flops / (1000.0f * (double) metrics->total_micro_seconds));
        } else {
            DEBUG("FLOPs/sec calculated:  %ld * 1 million / %lld = %f", metrics->flops, metrics->total_micro_seconds,
                  metrics->flops_per_second);
            metrics->flops_per_second = ((double) metrics->flops * 1000000.0f / (double) metrics->total_micro_seconds);
        }
    }

}

void usage()
{
    fprintf(stderr, "Usage: matrix_1 [options]\n");
    fprintf(stderr, "Options include:\n");
    fprintf(stderr, "    -s SIZE for a SIZExSIZE matrix: IGNORED IN THIS VERSION - FIXED AS CONSTANT: %d)\n", N);
    fprintf(stderr, "    -b BLOCK_SIZE for blocking MUST be an integral factor of the size (default: 0)\n");
    fprintf(stderr, "    --ijk | --ikj | --jki for the loop interchange order (default ijk)\n");
    fprintf(stderr, "    -f INFILE.CSV to read matrix A from a file (default is random generated matrix)\n");
    fprintf(stderr, "    -o OUTFILE.CSV to write the result matrix to a file (default is none)\n");
    fprintf(stderr, "    -t TEST.CSV compare result with TEST.CSV (only useful when -f, not random)\n");
    fprintf(stderr, "    -m METRICS.CSV append metrics to this CSV file (creates it if it does not exist)\n");
    fprintf(stderr, "    --test-equals-cols validate that the columns are all the same in the result\n");
    fprintf(stderr, "    --test-reverse-rows also perform B. A and validate that the rows are all the same in the result\n");
    fprintf(stderr, "    --identity use an identity matrix (I) intead of a ones-matrix for creating test dta\n");
    fprintf(stderr, "    -q --quiet fewer output messages\n");
    fprintf(stderr, "    --silent no output messages only the result for metrics\n");
    fprintf(stderr, "    --verbose lots of output messages including full matrices for debugging\n");
    fprintf(stderr, "    --debug debug messages (includes verbose)\n");
    fprintf(stderr, "    --papi-ignore do not fail if papi produces errors\n");
    fprintf(stderr, "    --papi, -p PAPI_CTR_1,PAPI_CTR2, measure specified PAPI counters, comma separated list\n");
    fprintf(stderr, "    -h print this help and exit\n");
    fprintf(stderr, "\n");
    exit(1);
}

char* loop_order_name(enum loop_order loop_order) {
    switch (loop_order) {
        case jki: return "jki";
        case ikj: return "ikj";
        default: return "ijk";
    }
}

char* valid_file(char opt, char *filename)
{
    if (access(filename, F_OK ) == -1 ) {
        fprintf(stderr, "Error: The option '%c' expects the name of an existing file (cannot find %s)\n", opt, filename);
        usage();
    }
    return filename;
}

int valid_count(char opt, char *arg)
{
    int value = atoi(arg);
    if (value <= 0) {
        fprintf(stderr, "Error: The option '%c' expects a counting number (got %s)\n", opt, arg);
        usage();
    }
    return value;
}

void validate_config(struct config config)
{
    const char* loop_order_names[] = {"ijk", "ikj", "jki"};
    if (!config.quiet) {
        printf("Config:\n");
        printf("Input file        : %-10s\n", config.in_file);
        printf("Output file       : %-10s\n", config.out_file);
        printf("Test file         : %-10s\n", config.test_file);
        printf("Metrics file      : %-10s\n", config.metrics_file);
        printf("Matrix size       : %-10d\n", config.size);
        printf("Loop order        : %s\n", loop_order_names[config.loop_order]);
        printf("Identity (vs ones): %d\n", config.identity);
        printf("Block size        : %d\n", config.block_size);
        printf("Test equal cols   : %d\n", config.test_equal_cols);
        printf("Test reverse rows : %d\n", config.test_reverse_rows);
        printf("Flags: \n");
        printf("debug       : %d\n", config.debug);
        printf("quiet       : %d\n", config.quiet);
        printf("silent      : %d\n", config.silent);
        printf("verbose     : %d\n", config.verbose);
        printf("papi_ignore : %d\n", config.papi_ignore);
        printf("\n");
    }
}

/**
 * Parse command line args and construct a config object
 */
struct config parse_cli(int argc, char *argv[])
{
    int opt;
    struct config config = new_config();

    struct option long_options[] = {
            {"ijk", no_argument, (int *)&config.loop_order, ijk},
            {"ikj", no_argument, (int *)&config.loop_order, ikj},
            {"jki", no_argument, (int *)&config.loop_order, jki},
            {"input", required_argument, NULL, 'f'},
            {"output", required_argument, NULL, 'o'},
            {"test", required_argument, NULL, 't'},
            {"size", required_argument, NULL, 's'},
            {"silent", no_argument, NULL, OPT_SILENT },
            {"papi-ignore", no_argument, NULL, 'i' },
            {"papi", required_argument, NULL, 'p'},
            {"verbose", no_argument, NULL, 'v' },
            {"debug", no_argument, NULL, 'd' },
            {"identity", no_argument, NULL, OPT_IDENTITY },
            {"giga", no_argument, NULL, OPT_GIGA },
            {"test-equal-cols", no_argument, NULL, OPT_TEST_EQUAL_COLS },
            {"test-reverse-rows", no_argument, NULL, OPT_TEST_REVERSE_ROWS },
            {"quiet", no_argument, NULL, 'q'},
            {NULL, 0, NULL, 0}
    };
    int option_index = 0;

    while((opt = getopt_long(argc, argv, "o:f:s:b:l:t:m:qh", long_options, &option_index)) != -1)
    {
//        fprintf(stderr, "FOUND OPT: [%c]\n", opt);
        switch(opt) {
            case 0:
                /* If this option set a flag, do nothing else: the flag is set */
                if (long_options[option_index].flag != 0)
                    break;
                // unexpected for now but maybe useful later
                printf("Unexpected option %s\n", long_options[option_index].name);
                usage();
            case 'h':
                usage();
                break;
            case 'd':
                config.debug = true;
                break;
            case 'i':
                config.papi_ignore = true;
                break;
            case OPT_SILENT:
                config.silent = true;
                break;
            case OPT_GIGA:
                config.giga = true;
                break;
            case OPT_IDENTITY:
                config.identity = true;
                break;
            case OPT_TEST_EQUAL_COLS:
                config.test_equal_cols = true;
                break;
            case OPT_TEST_REVERSE_ROWS:
                config.test_reverse_rows = true;
                break;
            case 'v':
                config.verbose = true;
                break;
            case 'q':
                config.quiet = true;
                break;
            case 'f':
                config.in_file = valid_file('f', optarg);
                break;
            case 'o':
                config.out_file = optarg;
                break;
            case 't':
                config.test_file = optarg;
                break;
            case 'p':
                config.papi_arg = optarg;
                break;
            case 'm':
                config.metrics_file = optarg;
                break;
            case 'l':
                config.label = optarg;
                break;
            case 's':
                config.size = valid_count('s', optarg);
                break;
            case 'b':
                config.block_size = valid_count('b', optarg);
                break;
            case ':':
                fprintf(stderr, "ERROR: Option %c needs a value\n", optopt);
                usage();
                break;
            case '?':
                fprintf(stderr, "ERROR: Unknown option: %c\n", optopt);
                usage();
                break;
            default:
                fprintf(stderr, "ERROR: Should never get here. opt=[%c]", opt);
        }
    }

    if (config.silent) {
        config.quiet = true; // silent implies quiet
    }
    if (config.debug) {
        config.verbose = true; // debug includes verbose messages
        config.silent = false; // just in case it was accidentally set on command line
        config.quiet = false;
    }

    validate_config(config);
    return config;
}

void progress_start(int total)
{
    if (!config->quiet) {
        printf("\nStarting matrix multiplication with %d rows, reporting every %d rows\n", total, PROGRESS_GRANULARITY);
    }
}

extern void progress(int rows, int total)
{
    if (!config->quiet) {
        if ((rows % PROGRESS_GRANULARITY) == 0) {
            int percent = rows * 100 / total;
            printf("%d%% calculated  %d / %d rows\n", percent, rows, total);
        }
    }
}

extern void progress_end(int rows)
{
    if (!config->quiet) {
        printf("Completed calculation of %d rows\n\n", rows);
    }
}

#endif