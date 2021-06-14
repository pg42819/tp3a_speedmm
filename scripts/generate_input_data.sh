#!/usr/bin/env bash
# Run the matrix program with identity matrix to
# generate a csv file with a fixed random matrix

current_dir=$( cd "$( dirname ${BASH_SOURCE[0]} )" && pwd )
source ${current_dir}/set_env.sh

${PROJECT_BIN_DIR}/matrix_1 --identity -o ${PROJECT_DATA_DIR}/random_4096.csv
