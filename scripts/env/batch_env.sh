current_dir=$( cd "$( dirname ${BASH_SOURCE[0]} )" && pwd )
source ${current_dir}/prep_env.sh

unset MATRIX_RUN_NOOP
unset MATRIX_RUN_INTERACTIVE
export MATRIX_RUN_BLOCK=2
export MATRIX_RUN_NO_INTERCHANGE=true
export MATRIX_RUN_PROG_FIRST=1
export MATRIX_RUN_PROG_LAST=4
export MATRIX_RUN_REPORT=_batch
export MATRIX_RUN_REPEATS=5
export MATRIX_RUN_PAPI=PAPI_L1_DCM:PAPI_L2_DCM:PAPI_L3_TCM:PAPI_L2_DCA:PAPI_L3_DCA!PAPI_FP_INS:PAPI_FP_OPS:PAPI_TOT_INS
export MATRIX_RUN_INPUT=random_4096.csv
export MATRIX_RUN_TEST=expected.csv
export MATRIX_RUN_VERBOSE=silent

source ${current_dir}/../run_batch.sh