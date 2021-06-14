#include <stdio.h>
#include "local_papi.h"
//#include "papivi_local.h"
//#include "../test/papiStdEventDefs_local.h"

#define DUMMY_EVENT_CODE PAPI_L1_DCM

#define DUMMY_EVENT_SET_HANDLE 44

// Stub methods to allow compilation on mac - dont' actually produce
// anything useful
int PAPI_start_counters(int events[], int len) {
    return PAPI_OK;
}

int PAPI_num_counters() {
    return 99;
}

/**< return the process microseconds since some arbitrary starting point */
long long PAPI_get_virt_usec(void) {
    return 1000;
}

int PAPI_library_init(int version) {
    return version;
}

int PAPI_create_eventset(int *EventSet) {
    *EventSet = DUMMY_EVENT_SET_HANDLE;
    return PAPI_OK;
}

/**< add array of PAPI preset or native hardware events to an event set */
int PAPI_add_events(int EventSet, int *Events, int number) {
    printf("Add Events: EventSet [%d]  Num events [%d]\n", EventSet, number);
    for (int i = 0; i < number;  i++) {
        printf("           event[%d] = %d\n", i, Events[i]);
    }
    return PAPI_OK;
}

/**< add single PAPI preset or native hardware event to an event set */
int  PAPI_add_event(int EventSet, int Event) {
    return PAPI_OK;
}
/**< return a pointer to the error name corresponding to a specified error code */
char *PAPI_strerror(int errorCode) {
    char msg[200];
    sprintf(msg, "error: %d", errorCode);
}

/**< start counting hardware events in an event set */
int  PAPI_start(int EventSet) {
    return PAPI_OK;
}


/**< stop counting hardware events in an event set and return current events */
int  PAPI_stop(int EventSet, long long * values) {
    return PAPI_OK;
}

/**< translate an integer PAPI event code into an ASCII PAPI preset or native name */
int  PAPI_event_code_to_name(int EventCode, char *out) {
    if (EventCode == DUMMY_EVENT_CODE) {
        sprintf(out, "PAPI_GOOD");
    }
    else {
        sprintf(out, "PAPI_BAD");
    }
    return PAPI_OK;
}

int PAPI_get_event_info(int EventCode, PAPI_event_info_t *info) {
    // TODO noop for now
    return PAPI_OK;
}

/**< query if a PAPI event exists */
int  PAPI_query_event(int EventCode) {
    return PAPI_OK;
}

/**< translate an ASCII PAPI preset or native name into an integer PAPI event code */
int PAPI_event_name_to_code(char *in, int *out) {
    *out = DUMMY_EVENT_CODE;
    return PAPI_OK;
}

/**< change the option settings of the PAPI library or a specific event set */
int PAPI_set_opt(int option, PAPI_option_t * ptr) {
    return PAPI_OK;
}
