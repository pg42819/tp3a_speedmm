#!/usr/bin/env bash
# Uses
# test_block
# test_block 2
# test_block "2 3 5"
# test_block "2 3 5" --verbose --transpose
testnums=${1:-5}
shift
current_dir=$( cd "$( dirname ${BASH_SOURCE[0]} )" && pwd )
source ${current_dir}/set_env.sh

for testnum in $testnums; do
  echo ${MATRIX_SCRIPTS_DIR}/simple_run.sh 2 -s ${testnum} -q -f ${MATRIX_DATA_DIR}/input_${testnum}.csv --test ${MATRIX_TEST_DIR}/expected_${testnum}.csv $*
  ${MATRIX_SCRIPTS_DIR}/simple_run.sh 2 -s ${testnum} -q -f ${MATRIX_DATA_DIR}/input_${testnum}.csv --test ${MATRIX_TEST_DIR}/expected_${testnum}.csv $*
done
