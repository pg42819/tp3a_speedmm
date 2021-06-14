#include <float.h>
#include <math.h>
#include "matrix.h"
//#include "matrix_config.h"
#include "matrix_types.h"

/**
 * Perform a dot-multiplication on two square matrices of a given size in i j k (natural) order
 * with blocking in blocks of bsize with vector enhancements
 *
 * IMPORTANT: The result matrix must be zeroed for this function to succeed
 *
 * @param bsize size of blocks to use in blocking algo
 * @param matrix1 left-side of the dot-multiplication
 * @param matrix2 right-side of the dot-multiplication
 * @param result preallocated zeroed matrix into which to store the results
 */
void dot_multiply_matrices_vecijk(size_t bsize, double matrix1[][N], double matrix2[][N], double result[][N])
{
    long flops = 0;
    double r = 0.0f;
    unsigned kk = 0, jj = 0, i = 0, j = 0, k = 0;
// tell the compiler to ignore dependencies between vectors
#pragma ivdep
#pragma code_align(8)
#pragma vector aligned
    for (kk = 0; kk < N; kk += bsize) {
#pragma ivdep
#pragma vector aligned
        for (jj = 0; jj < N; jj += bsize) {
#pragma ivdep
#pragma vector aligned
            for (i = 0; i < N; i++) {
#pragma ivdep
#pragma vector aligned
                for (j = jj; j < jj + bsize; j++) {
                    r = result[i][j];
#pragma ivdep
#pragma vector aligned
                    for (size_t k = kk; k < kk + bsize; k++) {
                        r += matrix1[i][k] * matrix2[k][j];
                    }
                    result[i][j] = r;
                }
            }
        }
    }
}

/**
 * Perform a dot-multiplication on two square matrices of a given size in i k j order
 *
 * IMPORTANT: The result matrix must be zeroed for this function to succeed
 *
 *
 * @param matrix1 left-side of the dot-multiplication
 * @param matrix2 right-side of the dot-multiplication
 * @param result preallocated matrix into which to store the results
 * @return number of doubleing point operations performed or -1 if there is a problem
 */
long dot_multiply_matrices_bikj(size_t bsize, double matrix1[][N], double matrix2[][N], double result[][N])
{
    return MATRIX_NOT_YET_IMPLEMENTED; // not yet implemented
}

/**
 * Perform a dot-multiplication on two square matrices of a given size in j k i order
 *
 * IMPORTANT: The result matrix must be zeroed for this function to succeed
 *
 * @param matrix1 left-side of the dot-multiplication
 * @param matrix2 right-side of the dot-multiplication
 * @param result preallocated zeroed  into which to store the results
 * @return number of doubleing point operations performed or -1 if there is a problem
 */
long dot_multiply_matrices_bjki(size_t bsize, double matrix1[][N], double matrix2[][N], double result[][N])
{
    return MATRIX_NOT_YET_IMPLEMENTED; // not yet implemented
}

/**
 * Perform a dot-multiplication on two square matrices of a given size in the specified order
 *
 * Relies on the global config to get the block_size (so that it can match the signature
 * of the same function in other implementation modules)
 *
 * IMPORTANT: The result matrix must be zeroed for this function to succeed
 *
 * @param order the loop order: one of ijk, kij, or kji
 * @param matrix1 left-side of the dot-multiplication
 * @param matrix2 right-side of the dot-multiplication
 * @param result preallocated zeroed matrix into which to store the results
 * @return number of doubleing point operations performed or -1 if there is a problem
 */
long dot_multiply_matrices(enum loop_order order,
                           double matrix1[][N], double matrix2[][N], double result[][N])
{
    int bsize = config->block_size;
    if (bsize < 1) {
        ERROR("block-size (-b) must be specified to use the blocking implementation");
        return MATRIX_FAILED;
    }
    if (N % bsize != 0) {
        ERROR("matrix size (%zu) must be a whole multiple of block-size (-b %d) to use the blocking implementation", N, bsize);
        return MATRIX_FAILED;
    }
    dot_multiply_matrices_vecijk(bsize, matrix1, matrix2, result);
    return 0; // don't count flops for vector version
}
