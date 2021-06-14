#include "../src/papi_support_base.h"

#define MX 1024
#define NITER 20
#define MEGA 1000000
#define TOT_FLOPS (2*MX*MX*NITER)
#define NUM_EVENTS 3

/**
 * demonstrates a canonical performance problem—traversing memory with nonunit stride.
 * We measure this code's performance using the PAPI high-level interface.
 * This example uses PAPI presets.
 */
int main () {
    int iter, i, j;
    int *event_codes = PAPI_DEFAULT_EVENT_CODES;
    size_t num_events = PAPI_NUM_INTERESTING_EVENT_CODES;
//    int event_codes[3] = {PAPI_L1_DCM,PAPI_LD_INS, PAPI_FP_OPS};
    long long *results = malloc(sizeof(long long) * num_events);
    int event_set;

    event_set = init_papi_events(num_events, event_codes);

    double ad[MX][MX];
    for (i = 0; i < MX; i++) {
        for (j = 0; j < MX; j++) {
            ad[i][j] = 1.0 / 3.0; // Initialize the data
        }
    }

    long long t0 = start_papi(event_set);
    for (iter = 0; iter < NITER; iter++) {
        for (j = 0; j < MX; j++) {
            for (i = 0; i < MX; i++) {
                ad[i][j] += ad[i][j] * 3.0;
            }
        }
    }

    long long t1 = stop_papi(event_set, num_events, results);

    describe_papi_events(num_events, event_codes, results);
    printf("Total software flops = %f\n", (float) TOT_FLOPS);
    printf("Total hardware flops = %lld\n", (float) results[1]);
    printf("MFlop/s = %f\n", (float) (TOT_FLOPS / MEGA) / (t1 - t0));
}
