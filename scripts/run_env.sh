#!/usr/bin/env bash
# Should be sourced AFTER qsub
# Sets intel env (only not intel module) and papi env to run in a node
current_dir=$( cd "$( dirname ${BASH_SOURCE[0]} )" && pwd )
source ${current_dir}/intel_env.sh
# DO NOT MODULE LOAD intel/2020 or segmentation fault
echo module load papi/5.5.0
module load papi/5.5.0
papi_avail -a

# size range form matrix_run2.sh
export MATRIX_SIZE_MIN=64
export MATRIX_SIZE_MAX=8192
export MATRIX_SIZE_FACTOR=2
printenv | grep "MATRIX_SIZE"
echo ready to run
