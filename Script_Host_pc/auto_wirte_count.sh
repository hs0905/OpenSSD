#!/bin/bash

# 실패 카운트를 초기화
failure_count=0

# 성공 횟수를 초기화
success_count=0

# 실험을 시작하기 전에 필요한 환경 설정을 수행합니다.
setup_environment() {
    export LIBHUGEFS_DIR=/usr/lib
    sudo sh -c 'echo "1024" >  /proc/sys/vm/nr_overcommit_hugepages'
    sudo sh -c 'echo "1024" >  /proc/sys/vm/nr_hugepages'
    sudo rm -rf /home/cpl/ariane/read_file/*
}

# 실험 실행 함수
run_experiment() {
    # 실험 스크립트 (write_evict_ssmall.sh) 실행
    ./write_evict_ssmall.sh
    # 스크립트의 종료 상태를 확인합니다.
    if [ $? -ne 0 ]; then
        echo "An error occurred during the experiment."
        let "failure_count++"
        return 1
    else
        let "success_count++"
    fi
    return 0
}

# 환경 설정을 호출합니다.
setup_environment

# 실험을 지속적으로 실행합니다.
while true; do
    run_experiment
    # 실험 함수가 실패했으면 반복을 멈춥니다.
    if [ $? -ne 0 ]; then
        echo "Experiment failed after $success_count successful attempts."
        break
    fi
done

# 실패 카운트를 반환합니다.
echo "Failure count: $failure_count"
