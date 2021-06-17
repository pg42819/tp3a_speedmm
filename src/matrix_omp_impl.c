#include <float.h>
#include <math.h>
#include "matrix.h"
//#include "matrix_config.h"
#include "matrix_types.h"

/**
 * Perform a dot-multiplication on two square matrices of a given size in i j k (natural) order
 * with blocking in blocks of bsize
 *
 * IMPORTANT: The result matrix must be zeroed for this function to succeed
 *
 * @param bsize size of blocks to use in blocking algo
 * @param matrix1 left-side of the dot-multiplication
 * @param matrix2 right-side of the dot-multiplication
 * @param result preallocated zeroed matrix into which to store the results
 * @return number of floating point operations performed or -1 if there is a problem
 */
void multiply_block(int ii, int jj, int kk, size_t bsize, double matrix1[][N], double matrix2[][N], double result[][N])
{
    double counter = 1.0f;
    {
        for (size_t i = ii; i < ii+bsize; ++i) {
            for (size_t j = jj; j < jj+bsize; ++j) {
                for (size_t k = kk; k < kk+bsize; k+=8) {
                    result[i][j] += matrix1[i][k] * matrix2[j][k]
                            + matrix1[i][k+1] * matrix2[j][k+1]
                            + matrix1[i][k+2] * matrix2[j][k+2]
                            + matrix1[i][k+3] * matrix2[j][k+3]
                            + matrix1[i][k+4] * matrix2[j][k+4]
                            + matrix1[i][k+5] * matrix2[j][k+5]
                            + matrix1[i][k+6] * matrix2[j][k+6]
                            + matrix1[i][k+7] * matrix2[j][k+7];
                }
            }
        }
    }
}

long dot_multiply_matrices_blocked(size_t bsize, double matrix1[][N], double matrix2[][N], double result[][N])
{
#pragma omp parallel shared(matrix1, matrix2, result, bsize)
    {
#pragma omp single
        {
            for (size_t ii = 0; ii < N; ii += bsize) {
                for (size_t jj = 0; jj < N; jj += bsize) {
                    for (size_t kk = 0; kk < N; kk += bsize) {
                        // Run the block multiplication in parallel OMP tasks
                        // making sure that the right block-sections in the matrix are identified
                        // as  as ingoing to the task or outomcing or both. in separate
                        // This is based on the OpenMP documentation example found in:
                        // https://www.openmp.org/wp-content/uploads/openmp-examples-5.0.0.pdf
                        #pragma omp task shared(matrix1, matrix2, result, bsize)  \
                            firstprivate(ii, jj, kk) \
                            depend(in: matrix1[ii:bsize][kk:bsize], matrix2[kk:bsize][jj:bsize]) \
                            depend(inout: result[ii:bsize][jj:bsize])
                        multiply_block(ii, jj, kk, bsize, matrix1, matrix2, result);
                    }
                }
            }
        }
    }
     return 1; // forget about flops
}


/**
 * Perform a dot-multiplication on two square matrices of a given N in the specified order
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
long dot_multiply_matrices(enum loop_order order, double matrix1[][N], double matrix2[][N], double result[][N])
{
    int bsize = config->block_size;
    if (bsize < 1) {
        ERROR("block-size (-b) must be specified to use the blocking implementation");
        return MATRIX_FAILED;
    }
    if (N % bsize != 0) {
        ERROR("matrix N (%zu) must be a whole multiple of block-size (-b %d) to use the blocking implementation", N, bsize);
        return MATRIX_FAILED;
    }
    INFO("Running matrix_mult %d x %d in blocks of %d", N, N, bsize);
    switch(order) {
        case ijk:
            return dot_multiply_matrices_blocked(bsize, matrix1, matrix2, result);
        default:
            ERROR("Various orders implmented in block algorithm");
    }
}
