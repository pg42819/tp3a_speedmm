#include <float.h>
#include <math.h>
#include "matrix.h"
//#include "matrix_config.h"
#include "matrix_types.h"
#include <x86intrin.h>

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
    {
        __m256d sum;
        __m256d vector1, vector2, vresult, vtemp, vhigh;

        for (size_t i = ii; i < ii + bsize; ++i) {
            // omp simd aligned had no result
//#pragma omp simd aligned(matrix1,matrix2,result:4)
            for (size_t j = jj; j < jj + bsize; ++j) {
                //https://en.wikipedia.org/wiki/Advanced_Vector_Extensions#CPUs_with_AVX
                // AVX supports four 64-bit double-precision floating point numbers.
                for (size_t k = kk; k < kk + bsize; k += 4) {
                    // this ALMOST works but give results that do not correspond to expected
                    vector1 = _mm256_load_pd(&matrix1[i][k]); // matrix1[i][k]
                    vector2 = _mm256_load_pd(&matrix2[j][k]); // matrix2[j][k]
                    vtemp = _mm256_mul_pd(vector1, vector2);
                    // extract higher four floats
                    vhigh = _mm256_extractf128_pd(vtemp, 1); // high 256
                    // add higher four floats to lower floats
                    vresult = _mm256_add_pd(_mm256_castps256_ps128(vtemp), vhigh);
                    // horizontal add of that result
                    vresult = _mm256_hadd_pd(vresult, vresult);
                    // another horizontal add of that result
                    vresult = _mm256_hadd_pd(vresult, vresult);
                    result[i][j] += _mm256_cvtsd_f64(vresult);
                }
            }
        }
    }
}

long dot_multiply_matrices_blocked(size_t bsize, double matrix1[][N], double matrix2[][N], double result[][N])
{
//#pragma omp parallel shared(matrix1, matrix2, result, bsize)
    {
//#pragma omp single
        {
            for (size_t ii = 0; ii < N; ii += bsize) {
                for (size_t kk = 0; kk < N; kk += bsize) {
                    for (size_t jj = 0; jj < N; jj += bsize) {
                        // Run the block multiplication in parallel OMP tasks
                        // making sure that the right block-sections in the matrix are identified
                        // as  as ingoing to the task or outomcing or both. in separate
                        // This is based on the OpenMP documentation example found in:
                        // https://www.openmp.org/wp-content/uploads/openmp-examples-5.0.0.pdf
//                        #pragma omp task shared(matrix1, matrix2, result, bsize)  \
                            firstprivate(ii, jj, kk) \
                            depend(in: matrix1[ii:bsize][kk:bsize], matrix2[kk:bsize][jj:bsize]) \
                            depend(inout: result[ii:bsize][jj:bsize])
                        multiply_block(ii, jj, kk, bsize, matrix1, matrix2, result);
                    }
                }
            }
        }
    }
     return 1; // forget about flops - we'll add it from known values
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
