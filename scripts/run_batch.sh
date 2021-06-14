#!/usr/bin/env bash
# Run the matrix program multiple times and collect results
current_dir=$( cd "$( dirname ${BASH_SOURCE[0]} )" && pwd )
source ${current_dir}/set_env.sh

multirun() {
  echo "metrics report  :  ${metrics_file}"
  local program
  local full_label
  local let run_number=1

  echo "LOOPING over $num_progs programs"
  for ((prognum=${prog_first}; prognum<=${prog_last}; prognum++)) do
    echo "- LOOP prog # $prognum "
    progname=matrix_${prognum}
    program=${PROJECT_BIN_DIR}/${progname}
    local thread_label=""
    echo "- LOOPING over threads from $min_threads up to $max_threads step = $thread_step"
    for ((t=${min_threads}; t<=${max_threads}; t+=${thread_step})) do
      echo "- - LOOP thread # $t "
      # always start at 1 then jump evenly
      if [ "$t" -eq "0" ];then
        threads=1
      else
        thread_label="__t=${t}"
        threads=$t
      fi

      # subtract 1 for round numbers
      export OMP_NUM_THREADS=$threads
#      export OMP_SCHEDULE=static #,chunk_size
      echo "OMP_NUM_THREADS: $OMP_NUM_THREADS"
      local omp_label="${label}${progname}${thread_label}"

      local order
      echo "- - LOOPING over orders or blocks, $order_loop"
      for order in "${order_loop[@]}"; do
        local order_arg="${order_arg_prefix}${order}"
        echo "- - - LOOP order # $order"
#          local sub_label="${omp_label}__order_${order}${block_label}${papi_label}"
          local sub_label="${omp_label}"
          full_label="${sub_label}"
          singlerun
          let run_number++
      done
    done
  done
}

singlerun() {
  local out_arg=""
  if [ -n "${out}" ]; then
    local filename="$(echo ${full_label} | sed 's/[ \/\+\.]/_/g')"
    out_arg="-o ${out}/${filename}.csv"
  fi

  local in_arg=""
  if [ -n "${MATRIX_RUN_INPUT}" ]; then
    in_arg="-f ${PROJECT_DATA_DIR}/${MATRIX_RUN_INPUT}"
  fi
  for ((run=1; run<=${repeats}; run++)) do
    echo "RUNNING $full_label repetition $run of $repeats"
    run_label="${full_label}__rep_$run"
    to_run="${program} --giga ${order_arg} ${debug_arg} ${verbose_arg} ${papi_arg} ${in_arg} ${out_arg} -m ${metrics_file} ${test_args} -l ${run_label}"
    echo "About to: ${to_run}"
    if $interactive; then
      askcontinue
    fi
    if [ -z ${MATRIX_RUN_NOOP+x} ]; then
      ${to_run}
      if [ $? -eq 0 ]; then
        echo "### Run $run_number  ++++++ OK ++++++"
      else
        echo "### Run $run_number  !!!!! FAIL !!!!!"
      fi
    else
      echo "DRY RUN. Not doing anything"
    fi
  done
}

askcontinue() {
  local question=${1:-"Do you want to continue?"}
	read -p "$question (waiting 5s then defaulting to YES)? " -n 1 -r -t 5
	echo    # (optional) move to a new line
	# if no reply (timeout) or yY then yes, else leave it false
	if [[ -z "$REPLY" || $REPLY =~ ^[Yy]$ ]];then
		return 0
	fi
	return 1
}

run_matrix() {
  # Metrics go in same file to build a full result set
  min_threads=${MATRIX_RUN_THREADS_MIN:-16}
  max_threads=${MATRIX_RUN_THREADS_MAX:-32}
  thread_step=${MATRIX_RUN_THREADS_STEP:-16} # must be non-zero to be non infinite
#  matrix_sizes=( "$1" "$2" "$3" "$4" )

  label=""
  # run all: matrix_omp1 and matrix_omp2
  num_progs=1
  multirun
}

if [ -z ${MATRIX_RUN_INTERACTIVE+x} ]; then
  interactive=false
else
  interactive=true
fi

if [ -z ${MATRIX_RUN_TEST+x} ]; then
  test_args=""
else
  test_args="-t ${PROJECT_DATA_DIR}/${MATRIX_RUN_TEST}"
fi

if [ -z ${MATRIX_RUN_VERBOSE+x} ]; then
  verbose_arg=""
else
  verbose_arg="--${MATRIX_RUN_VERBOSE}"
fi

if [ -z ${MATRIX_RUN_DEBUG+x} ]; then
  debug_arg=""
else
  echo "VERBOSE"
  debug_arg="--debug"
fi

if [ -n "${MATRIX_RUN_BLOCK}" ]; then
  order_arg_prefix="-b "
  if [ "${MATRIX_RUN_BLOCK}" == "loop" ]; then
#    order_loop=( 4 8 16 32 64 128)
    order_loop=( 16 128 256 512)
    echo "Looping block size over [${order_loop}]"
  else
    echo "Setting block size to [${MATRIX_RUN_BLOCK}]"
    order_loop=( ${MATRIX_RUN_BLOCK} )
  fi
else
  # for "--ijk" etc
  order_arg_prefix="--"
  if [ -z ${MATRIX_RUN_NO_INTERCHANGE+x} ]; then
    order_loop=( ijk ikj jki )
    echo "LOOPING ALL INTERCHANGES: $order_loop"
  else
    echo "SKIPPING loop interchange"
    order_loop=( ijk )
  fi
fi

if [ -n "${MATRIX_RUN_PAPI}" ]; then
  papi_arg="--papi ${MATRIX_RUN_PAPI}"
  papi_label="__papi_${MATRIX_RUN_PAPI}"
else
  papi_arg=""
  papi_label="__no_papi"
fi

if [ -n "${MATRIX_RUN_PROG_FIRST}" ]; then
  prog_first=${MATRIX_RUN_PROG_FIRST}
else
  echo "set MATRIX_RUN_PROG_FIRST to the number of the first program 1=simple, 2=block..."
  prog_first=1
fi

if [ -n "${MATRIX_RUN_PROG_LAST}" ]; then
  prog_last=${MATRIX_RUN_PROG_LAST}
else
  echo "set MATRIX_RUN_PROG_LAST to the number of the first program 1=simple, 2=block..."
  prog_last=${prog_first}
fi

if [ -n "${MATRIX_RUN_REPEATS}" ]; then
  repeats=${MATRIX_RUN_REPEATS}
  echo "Running same data $repeats times"
else
  repeats=5
fi

report_name=${MATRIX_RUN_REPORT:-1}
metrics_file=${PROJECT_METRICS_DIR}/"matrix_metrics${report_name}.csv"

if [ -f "${metrics_file}" ]; then
  if [ -z ${MATRIX_RUN_NOOP+x} ]; then
    echo "Found an existing metrics file at ${metrics_file}"
    askcontinue "Do you want to delete the report at ${metrics_file} and start clean?" && rm -f $metrics_file
  fi
fi

run_matrix

askcontinue "Want to see the results?"
cat ${metrics_file}