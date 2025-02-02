# Copyright 2021 Nordic Semiconductor
# SPDX-License-Identifier: Apache-2.0

process_ids=(); exit_code=0

trap ctrl_c INT

function ctrl_c() {
  echo "Aborted by CTRL-C"
  conf=${conf:-prj_conf}

  for process_id in ${process_ids[@]}; do
    kill -15 $process_id
  done
}

function Execute(){
  if [ ! -f $1 ]; then
    echo -e "  \e[91m`pwd`/`basename $1` cannot be found (did you forget to\
 compile it?)\e[39m"
    exit 1
  fi
  timeout 300 $@ & process_ids+=( $! )
}

function Skip(){
  for i in "${SKIP[@]}" ; do
    if [ "$i" == "$1" ] ; then
      return 0
    fi
  done

  return 1
}

function RunTest(){
  verbosity_level=${verbosity_level:-2}
  extra_devs=${EXTRA_DEVS:-0}

  cd ${BSIM_OUT_PATH}/bin

  idx=0

  s_id=$1
  shift 1

  testids=()
  for testid in $@ ; do
    if [ "$testid" == "--" ]; then
      shift 1
      break
    fi

    testids+=( $testid )
    shift 1
  done

  test_options=$@

  for testid in ${testids[@]} ; do
    if Skip $testid; then
      echo "Skipping $testid (device #$idx)"
      let idx=idx+1
      continue
    fi

    echo "Starting $testid as device #$idx"
    conf=${conf:-prj_conf}

    if [ ${overlay} ]; then
        exe_name=./bs_${BOARD}_tests_bluetooth_bsim_mesh_${conf}_${overlay}
    else
        exe_name=./bs_${BOARD}_tests_bluetooth_bsim_mesh_${conf}
    fi

    Execute \
      ${exe_name} \
      -v=${verbosity_level} -s=$s_id -d=$idx -RealEncryption=1 \
      -testid=$testid ${test_options}
    let idx=idx+1
  done

  count=$(expr $idx + $extra_devs)

  echo "Starting phy with $count devices"

  Execute ./bs_2G4_phy_v1 -v=${verbosity_level} -s=$s_id -D=$count

  for process_id in ${process_ids[@]}; do
    wait $process_id || let "exit_code=$?"
  done

  if [ "$exit_code" != "0" ] ; then
    exit $exit_code #the last exit code != 0
  fi
}

: "${BSIM_OUT_PATH:?BSIM_OUT_PATH must be defined}"

#Give a default value to BOARD if it does not have one yet:
BOARD="${BOARD:-nrf52_bsim}"
