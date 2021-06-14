#!/usr/bin/env bash
# Run the matrix program once
# E.g. simple_run.sh -s 1024 --transpose --ikj
current_dir=$( cd "$( dirname ${BASH_SOURCE[0]} )" && pwd )
source ${current_dir}/set_env.sh

matrix_program=${MATRIX_BIN_DIR}/matrix_1
if [ ! -f ${matrix_program} ]; then
  echo "There is no program at ${matrix_program}. Try a different number"
  exit 1
fi

#intel_vars_script=/share/apps/intel/parallel_studio_xe_2019/compilers_and_libraries_2019/linux/bin/compilervars.sh
#if [ -f ${intel_vars_script} ]; then
#  source ${intel_vars_script} intel64
#fi
run_command="${matrix_program} -s 3 --papi PAPI_L1_DCM:PAPI_L2_DCM!PAPI_L3_TCM!PAPI_FP_OPS:PAPI_FP_INS --debug $*"
echo "Running ${run_command}"
${run_command}
