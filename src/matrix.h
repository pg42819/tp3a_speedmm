#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <omp.h>
#include <string.h>
#include "matrix_types.h"


// global config shared among modules
extern struct config *config;

extern struct config new_config();
extern struct metrics new_metrics();
extern void usage();

extern char* valid_file(char opt, char *filename);
extern int valid_count(char opt, char *arg);
extern void validate_config(struct config config);

extern void progress_start(int total);
extern void progress(int rows,int total);
extern void progress_end(int rows);

extern struct config parse_cli(int argc, char *argv[]);

// help with debugging OMP
#ifdef MATRIX_OMP
extern int omp_schedule_kind(int *chunk_size);
extern void omp_debug(char *msg);
#endif