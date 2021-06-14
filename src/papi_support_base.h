#ifndef PAPI_SUPPORT_BASE
#define PAPI_SUPPORT_BASE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>

#ifdef __APPLE__
// macos does not have PAPI, so compile with headers and stubs
#include "local_papi.h"
#include "local_papi_stubs.h"
#else
#include "papi.h"
#endif

#include "matrix_config.h"


#define MAX_PAPI_CODES 100

// List of available codes from papi_avail -a  on an mei 662 SeARCH node
// COMMENT OUT WHATEVER IS NOT NEEDED
static int PAPI_DEFAULT_EVENT_CODES[] = {
        PAPI_L1_DCM, // Level 1 data cache misses
//	PAPI_L1_ICM, // Level 1 instruction cache misses
        PAPI_L2_DCM, // Level 2 data cache misses
//	PAPI_L2_ICM, // Level 2 instruction cache misses (instruction + data)
//	PAPI_L1_TCM, // Level 1 cache misses
//	PAPI_L2_TCM, // Level 2 cache misses
        PAPI_L3_TCM, // Level 3 cache misses
//	PAPI_TLB_DM, // Data translation lookaside buffer misses
//	PAPI_TLB_IM, // Instruction translation lookaside buffer misses

// failed to add but provided values - will notuse for now
// PAPI_L1_LDM, // Level 1 load misses

// failed	PAPI_L1_STM, // Level 1 store misses

// failed to add but provided values - will notuse for now
// failed	PAPI_L2_STM, // Level 2 store misses

//	PAPI_STL_ICY, // Cycles with no instruction issue
//	PAPI_BR_UCN, // Unconditional branch instructions
//	PAPI_BR_CN, // Conditional branch instructions
//	PAPI_BR_TKN, // Conditional branch instructions taken
//	PAPI_BR_NTK, // Conditional branch instructions not taken
//	PAPI_BR_MSP, // Conditional branch instructions mispredicted
//	PAPI_BR_PRC, // Conditional branch instructions correctly predicted
//	PAPI_TOT_INS, // Instructions completed
// failed    PAPI_FP_INS, // Floating point instructions
// failed 	PAPI_LD_INS, // Load instructions
// failed 	PAPI_SR_INS, // Store instructions
// PAPI_BR_INS, // Branch instructions
// failed	PAPI_TOT_CYC, // Total cycles
// failed	PAPI_L2_DCH, // Level 2 data cache hits

// did not fail to add, but always gave zero values (erroneously)
//	PAPI_L2_DCA, // Level 2 data cache accesses
// did not fail to add, but always gave zero values (erroneously)
//	PAPI_L3_DCA, // Level 3 data cache accesses

// failed to add and gave zero values
//    PAPI_L2_DCR, // Level 2 data cache reads
// failed to add and gave zero values
//    PAPI_L3_DCR, // Level 3 data cache reads
// failed to add and gave zero values
//    PAPI_L2_DCW, // Level 2 data cache writes
// failed to add and gave zero values
//    PAPI_L3_DCW, // Level 3 data cache writes

//	PAPI_L2_ICH, // Level 2 instruction cache hits
//	PAPI_L2_ICA, // Level 2 instruction cache accesses
//	PAPI_L3_ICA, // Level 3 instruction cache accesses
//	PAPI_L2_ICR, // Level 2 instruction cache reads
//	PAPI_L3_ICR, // Level 3 instruction cache reads
//	PAPI_L2_TCA, // Level 2 total cache accesses
//	PAPI_L3_TCA, // Level 3 total cache accesses
//	PAPI_L2_TCR, // Level 2 total cache reads
//	PAPI_L3_TCR, // Level 3 total cache reads
//	PAPI_L2_TCW, // Level 2 total cache writes
//	PAPI_L3_TCW, // Level 3 total cache writes
//	PAPI_FDV_INS, // Floating point divide instructions
// failed	PAPI_FP_OPS, // Floating point operations
// failed	PAPI_SP_OPS, // Floating point operations; optimized to count scaled single precision vector operations
//	PAPI_DP_OPS, // Floating point operations; optimized to count scaled double precision vector operations
// failed	PAPI_VEC_SP, // Single precision vector/SIMD instructions
//	PAPI_VEC_DP, // Double precision vector/SIMD instructions
// failed	PAPI_REF_CYC, // Reference clock cycles
};
static unsigned PAPI_NUM_DEFAULT_EVENT_CODES =
        sizeof(PAPI_DEFAULT_EVENT_CODES) / sizeof(PAPI_DEFAULT_EVENT_CODES[0]);

extern struct config *config;

void handle_papi(int papi_retval, char* error_msg, char* success_msg)
{
    if (papi_retval == PAPI_OK) {
        if (success_msg != NULL && config != NULL && !config->quiet) {
            printf("%s\n", success_msg);
        }
    }
    else {
        fprintf(stderr, "PAPI ERROR: %s. PAPI Error Code: %d : %s\n", error_msg, papi_retval, PAPI_strerror(papi_retval));
        if (config->papi_ignore) {
            // don't fail
            fprintf(stderr, "Continuing without PAPI working\n");
        } else {
            exit(1);
        }
    }
}

void handle_papi_errors(int papi_retval, char *error_msg)
{
    handle_papi(papi_retval, error_msg, NULL);
}

char *papi_name(int event_code) {
    char *code_name = (char *)malloc(sizeof(char) * PAPI_MAX_STR_LEN);
    int retval = PAPI_event_code_to_name(event_code, code_name);
    if (retval != PAPI_OK) {
        DEBUG("Failed to find name for code %d", event_code);
        return "BAD_NAME";
    }
    return code_name;
}

int papi_code(char *event_name) {
    int event_code = 0;
    int retval = PAPI_event_name_to_code(event_name, &event_code);
    if (retval != PAPI_OK) {
        char msg[200];
        sprintf(msg, "Failed to find PAPI code for [%s]", event_name);
        handle_papi_errors(retval, msg);
    }
    else {
        if (config->debug) {
            printf("DEBUG Found PAPI code [%#010x] for [%s]\n", event_code, event_name);
        }
    }
    return event_code;
}


/**
 * Initializes PAPI
 */
void init_papi()
{
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval != PAPI_VER_CURRENT) {
        handle_papi_errors(retval, "library initialization failed with");
    } else {
        INFO("PAPI library initialization succeeded with version: %d", retval);
    }
}

long long get_papi_time()
{
    return PAPI_get_virt_usec();
}

#endif
