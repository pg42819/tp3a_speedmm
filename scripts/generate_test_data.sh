#!/usr/bin/env bash
# Run the matrix program with the input matrix and save output
# for testing in subsequent runs

current_dir=$( cd "$( dirname ${BASH_SOURCE[0]} )" && pwd )
source ${current_dir}/set_env.sh

${PROJECT_BIN_DIR}/matrix_1 -f ${PROJECT_DATA_DIR}/random_4096.csv -o ${PROJECT_DATA_DIR}/expected.csv
