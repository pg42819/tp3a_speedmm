
#ifndef PAPI_SUPPORT_LOCAL
#define PAPI_SUPPORT_LOCAL
#include "papi_support_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>

#define MAX_PAPI_CODES 100

extern struct config *config;

unsigned split_string(char *string, char *delim, char *words[MAX_PAPI_CODES]) {
    if (string == NULL || strlen(string) == 0) return 0;
    char *buffer, *aPtr;
    unsigned count = 0, i;
    buffer = strdup(string);

    do {
        aPtr = strsep(&buffer, delim);
        if (aPtr && count < MAX_PAPI_CODES) words[count++] = aPtr;
    } while(aPtr && count < MAX_PAPI_CODES);

//    for (i = 0; i < count; i++) {
//        printf("%s\n", words[i]);
//    }
    return count;
}

/**
 * Initializes PAPI, and return an EventSet handle for the event set from the event codes
 *
 * @param offset offset into the total event_code array
 * @param num_events number of event codes to add
 * @param event_codes array of PAPI event codes
 * @param failed_codes array to which we add return values of any papi failures per event
 * @return EventSet handle to the event set in the papi api
 */
int init_papi_events(unsigned offset, unsigned num_events, int *event_codes, int *failed_codes) {
    int bad_codes = 0;
    for (int i = 0; i < num_events;  i++) {
        int code = event_codes[offset + i];
        if (PAPI_query_event(code) != PAPI_OK) {
            char* code_name = papi_name(code);
            ERROR("The PAPI event code [%s] is not available on this platform", code_name);
            bad_codes++;
        }
    }
    if (bad_codes == 0) {
        INFO("All provided codes exist on the platform");
    }
    else {
        exit(1);
    }

    int event_set = PAPI_NULL; // init a handle for the event set reference
    int retval = PAPI_create_eventset(&event_set);
    handle_papi(retval, "event set creation failed", "PAPI event set creation succeeded");

    // debug papi
//    PAPI_option_t options;
//    memset(&options, 0x0, sizeof(options));
//    options.debug.level = 1;
//    retval = PAPI_set_opt(PAPI_DOMAIN, &options);
//    retval = PAPI_set_opt(PAPI_DEBUG, &options);
//    handle_papi(retval, "set options failed", "PAPI set options succeeded");

    VERBOSE("PAPI Adding %u codes to the EventSet %d\n", num_events, event_set);
    int add_failures = 0;
    for (unsigned i = 0; i < num_events;  i++) {
        unsigned pos = offset + i;
        int code = event_codes[pos];
        char *code_name = papi_name(code);
        DEBUG("Adding %uth %s event to EventSet:%d", pos,  code_name, event_set);
        retval = PAPI_add_event(event_set, code);
        failed_codes[pos] = retval; // store for later use
        if (retval != PAPI_OK) {
            add_failures++;
            INFO("WARNING: Skipping PAPI code %s! Failed to add the %dth PAPI event: [%s]. Error code: %d (%s)\n",
                    code_name, pos, code_name, retval, PAPI_strerror(retval));
        }
    }

    if (add_failures == 0) {
        INFO("PAPI add events succeeded");
    } else {
        INFO("PAPI add events partially FAILED: %d events could not be added and were skipped.", add_failures);
    }

    retval = PAPI_num_counters();
    if (retval < 2) {
        handle_papi_errors(retval, "No hardware counters here, or PAPI not supported");
    } else {
        INFO("PAPI proceeding with hardware counters available: %d\n", retval);
    }

    return event_set;
}

long long start_papi(int event_set) {
    long long start_marker = get_papi_time();
    int retval = PAPI_start(event_set);
    handle_papi_errors(retval, "failed to start counters");
    return start_marker;
}

long long stop_papi(int event_set, unsigned offset, long long *all_event_results)
{
    long long stop_marker = get_papi_time();
    long long *event_results = &all_event_results[offset];
    int retval = PAPI_stop(event_set, event_results);
    handle_papi_errors(retval, "failed to stop and read counters");

    return stop_marker;
}

void describe_papi_event(int event_code, long long event_value, int error) {
    PAPI_event_info_t info;
    PAPI_get_event_info(event_code, &info);
    char *code_name = papi_name(event_code);
    if (error != PAPI_OK) {
        // actually failed so warn in output
        printf("PAPI counter [ %12s ] %-35s = %lld [WARN: ADD FAILURE! %d=%s]\n",
               code_name, info.long_descr, event_value, error, PAPI_strerror(error));
    }
    else {
        printf("PAPI counter [ %12s ] %-35s = %lld\n", code_name, info.long_descr, event_value);
    }
}

// output events in long form to the terminal
void describe_papi_events(size_t num_events, int event_codes[num_events],
                          long long event_values[num_events], int failed_codes[num_events]) {
    printf("\n");
    for (int i = 0; i < num_events; i++) {
        describe_papi_event(event_codes[i], event_values[i], failed_codes[i]);
    }
}

// print the event names in a csv line
void print_papi_headers(FILE *out, size_t num_events, int event_codes[num_events]) {
    if (num_events == 0) {
        // just to have something after the comma
        fprintf(out, "no-papi");
    }
    else {
        for (size_t i = 0; i < num_events; i++) {
            char *code_name = papi_name(event_codes[i]);
            if (i > 0) {
                fprintf(out, ",%s", code_name);
            } else {
                fprintf(out, "%s", code_name);
            }
        }
    }
}

// print the events in a csv line
void print_papi_events(FILE *out, size_t num_events, long long event_values[num_events])
{
    if (num_events == 0) {
        // just to have something after the comma
        fprintf(out, "0");
    }
    for (size_t i = 0; i < num_events; i++) {
        if (i > 0) {
            fprintf(out, ",%lld", event_values[i]);
        } else {
            fprintf(out, "%lld", event_values[i]);
        }
    }
}

#endif
