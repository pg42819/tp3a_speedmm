current_dir=$( cd "$( dirname ${BASH_SOURCE[0]} )" && pwd )
source ${current_dir}/prep_env.sh

unset MATRIX_RUN_NOOP
unset MATRIX_RUN_INTERACTIVE
unset MATRIX_RUN_BLOCK

# Not verbose but not quiet = unset verbose
unset MATRIX_RUN_VERBOSE
#export MATRIX_RUN_VERBOSE=silent
set MATRIX_RUN_NO_INTERCHANGE=true # no ijk in block
export MATRIX_RUN_PROG_FIRST=2
export MATRIX_RUN_REPORT=thread_test
export MATRIX_RUN_REPEATS=1
# export MATRIX_RUN_PAPI=PAPI_L1_DCM:PAPI_L2_DCM:PAPI_L3_TCM:PAPI_L2_DCA:PAPI_L3_DCA
export MATRIX_RUN_INPUT=random_4096.csv
export MATRIX_RUN_TEST=expected.csv
export MATRIX_RUN_THREADS_MIN=16
export MATRIX_RUN_THREADS_STEP=16
export MATRIX_RUN_THREADS_MAX=128
export MATRIX_RUN_BLOCK=64

source ${current_dir}/../run_batch.sh