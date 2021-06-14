#!/usr/bin/env bash
# Run the matrix program once
# E.g. simple_run.sh -s 1024 --transpose --ikj
current_dir=$( cd "$( dirname ${BASH_SOURCE[0]} )" && pwd )
source ${current_dir}/set_env.sh

program=$1
shift
matrix_program=${MATRIX_BIN_DIR}/matrix_${program}
if [ ! -f ${matrix_program} ]; then
  echo "There is no program at ${matrix_program}. Try a different number"
  return
fi

#intel_vars_script=/share/apps/intel/parallel_studio_xe_2019/compilers_and_libraries_2019/linux/bin/compilervars.sh
#if [ -f ${intel_vars_script} ]; then
#  source ${intel_vars_script} intel64
#fi
echo running ${matrix_program} $*
${matrix_program} $*
