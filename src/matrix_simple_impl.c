#include <float.h>
#include <math.h>
#include "matrix.h"

/**
 * Perform a dot-multiplication on two square matrices of a given size in i j k (natural) order
 *
 * IMPORTANT: The result matrix must be zeroed for this function to succeed
 *
 * @param size number of rows (and columns) in the square matrix
 * @param matrix1 left-side of the dot-multiplication
 * @param matrix2 right-side of the dot-multiplication
 * @param result preallocated zeroed matrix into which to store the results
 */
long dot_multiply_matrices_ijk(double matrix1[][N], double matrix2[][N], double result[][N])
{
    long flops = 0;
    #pragma novector
    #pragma noparallel
    progress_start(N);
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            // multiply each of the k elements of row i by the corresponding k element of col j
            // and sum them into the result cell at i,j
            for (size_t k = 0; k < N; ++k) {
                result[i][j] += matrix1[i][k] * matrix2[k][j];
                flops += 2; // + = 1, * = 2
            }
        }
        progress(i, N);
    }
    progress_end(N);
    return flops;
}

/**
 * Perform a dot-multiplication on two square matrices of a given size in i k j order
 *
 * IMPORTANT: The result matrix must be zeroed for this function to succeed
 *
 *
 * @param size number of rows (and columns) in the square matrix
 * @param matrix1 left-side of the dot-multiplication
 * @param matrix2 right-side of the dot-multiplication
 * @param result preallocated matrix into which to store the results
 */
long dot_multiply_matrices_ikj(double matrix1[][N], double matrix2[][N], double result[][N])
{
    long flops = 0;
#pragma novector
#pragma noparallel
    for (size_t i = 0; i < N; ++i) {
        for (size_t k = 0; k < N; ++k) {
            for (size_t j = 0; j < N; ++j) {
                result[i][j] += matrix1[i][k] * matrix2[k][j];
                flops += 2; // + = 1, * = 2
            }
        }
    }
    return flops;
}

/**
 * Perform a dot-multiplication on two square matrices of a given size in j k i order
 *
 * IMPORTANT: The result matrix must be zeroed for this function to succeed
 *
 * @param size number of rows (and columns) in the square matrix
 * @param matrix1 left-side of the dot-multiplication
 * @param matrix2 right-side of the dot-multiplication
 * @param result preallocated zeroed  into which to store the results
 */
long dot_multiply_matrices_jki(double matrix1[][N], double matrix2[][N], double result[][N])
{
    long flops = 0;

#pragma novector
#pragma noparallel
    for (size_t j = 0; j < N; ++j) {
        for (size_t k = 0; k < N; ++k) {
            for (size_t i = 0; i < N; ++i) {
                result[i][j] += matrix1[i][k] * matrix2[k][j];
                flops += 2; // + = 1, * = 2
            }
        }
    }
    return flops;
}

/**
 * Perform a dot-multiplication on two square matrices of a given size in the speciied order
 *
 * IMPORTANT: The result matrix must be zeroed for this function to succeed
 *
 * @param order the loop order: one of ijk, kij, or kji
 * @param size number of rows (and columns) in the square matrix
 * @param matrix1 left-side of the dot-multiplication
 * @param matrix2 right-side of the dot-multiplication
 * @param result preallocated zeroed matrix into which to store the results
 */
long dot_multiply_matrices(enum loop_order order, double matrix1[][N], double matrix2[][N], double result[][N])
{
    switch(order) {
        case ikj:
            return dot_multiply_matrices_ikj(matrix1, matrix2, result);
        case jki:
            return dot_multiply_matrices_jki(matrix1, matrix2, result);
        default:
            return dot_multiply_matrices_ijk(matrix1, matrix2, result);
    }
}
