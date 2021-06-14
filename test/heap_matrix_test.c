
#include <stdio.h>

#define N 10

void fill_array(float matrix[][N])
{
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            matrix[i][j] = i * 1000 + j;
        }
    }
}

void print_array(float matrix[][N])
{
    for (int i = 0; i < N; ++i) {
        printf("\n");
        for (int j = 0; j < N; ++j) {
            printf("%f.2 ", matrix[i][j]);
        }
    }
}

/**
 * Tests creation of arrays on heap
 */
int main ()
{
    float (*arr)[N] = malloc(N * N * sizeof(float));
    fill_array(arr);
    print_array(arr);
}

//    printf("Total software flops = %f\n", (float) TOT_FLOPS);
//    printf("Total hardware flops = %lld\n", (float) results[1]);
//    printf("MFlop/s = %f\n", (float) (TOT_FLOPS / MEGA) / (t1 - t0));
