#!/usr/bin/env bash
# Should be sourced BEFORE make BEFORE qsub
# Sets intel compile env and papi env to compile
current_dir=$( cd "$( dirname ${BASH_SOURCE[0]} )" && pwd )
# DO NOT SOURCE THIS OR COMPILER LICENSE ERROR:source ${current_dir}/intel_env.sh
echo module load intel/2020
module load intel/2020
echo module load gcc/5.3.0
module load gcc/5.3.0
echo export LC_ALL=C
export LC_ALL=C
echo module load papi/5.5.0
module load papi/5.5.0
echo module load cuda/10.1
module load cuda/10.1
papi_avail -a

echo ready for make
