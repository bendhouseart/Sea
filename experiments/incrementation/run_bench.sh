#!/bin/bash

#set -e
set -u 

typeset -i i END

let END=$1
results_file=$2

export SEA_HOME=/home/vhs/Sea
experiments=("${SEA_HOME}/bin/sea_launch.sh sea_parallel.sh")
experiments+=("bash ./lustre_parallel.sh")
experiments+=("bash ./disk_parallel.sh")
results_dir=./results
all_out=mem_all.out
flush_out=flushlist.out
bench_out=benchmarks.out
base_dir=/mnt/lustre/vhs/output
time_params="%e,%K"
curr_exp=""


create_template() {
    nnodes=$1
    ncpus=$2

}


launch_exp () {
    curr_exp=$1
    num_files=$2

    exp_dir="${results_dir}/${curr_exp}-$i"
    all_file="${exp_dir}/${all_out}"
    flush_file="${exp_dir}/${flush_out}"
    bench_file="${exp_dir}/${bench_out}"
    mkdir -p ${exp_dir}

    if [[ $curr_exp == "disk" ]]
    then
        echo "running disk"
        command time -f ${time_params} ${experiments[2]} &> ${all_file}
    fi


    if [[ $curr_exp != "lustre" && $curr_exp != "disk" ]]
    then
        echo "running sea"
        cp mem_sea.ini ${SEA_HOME}/sea.ini
        command time -f ${time_params} ${experiments[0]} &> ${all_file}
    elif [[ $curr_exp == "lustre" ]]
    then
        echo "running lustre"
        command time -f ${time_params} ${experiments[1]} &> ${all_file}
    fi

    # wait for command to finish
    while [[ $(ls ${base_dir} | wc -l) != ${num_files} ]]; do sleep 10; done

    grep -E "start|end" ${all_file} > ${bench_file}

    res=$(grep "^[0-9]*\.[0-9]*,[0-9]*" ${all_file})
    flushtime=0
    sddwrites=0
    totalflush=0

    if [[ curr_exp != "lustre" ]]
    then
        grep -Ei "mv|cp|rm" ${all_file} > ${flush_file}
        flushtime=$(grep 'real' ${all_file} | grep "[0-9].*")
        flushtime=$(echo ${flushtime:5})
        ssdwrites=$(grep 'disk' ${flush_file} | sort --unique | wc -l)
        totalflush=$(cat ${flush_file} | sort --unique | wc -l)
    fi

    echo "$curr_exp,$i,$res,$flushtime,$ssdwrites,$totalflush" >> ${results_file}
}


order=( 0 1 2 )

echo "experiment,repetition,runtime,max_mem,flush_time,disk_files,total_flush" > ${results_file}

for ((i=0;i<END;i++))
do
    order=( $(shuf -e "${order[@]}") )
    for e in "${order[@]}"
    do
        echo $e $i
        echo "Clearing cache"
        sync; echo 3 | sudo tee /proc/sys/vm/drop_caches
        echo "Removing directories"
        rm /mnt/lustre/vhs/output/*;
    	rm -rf /dev/shm/seatmp
	    rm -rf /disk0/vhs/seatmp /disk1/vhs/seatmp /disk2/vhs/seatmp /disk3/vhs/seatmp /disk4/vhs/seatmp /disk5/vhs/seatmp /dev/shm/seatmp 
        echo "Creating temp source mount directories"
        mkdir /dev/shm/seatmp
	    mkdir /disk0/vhs/seatmp /disk1/vhs/seatmp /disk2/vhs/seatmp /disk3/vhs/seatmp /disk4/vhs/seatmp /disk5/vhs/seatmp
        echo running experiment ${experiments[0]}

        if [[ $e == 0 ]]
        then
            curr_exp="mem_all"
            cp .sea_flushlist_all ${SEA_HOME}/.sea_flushlist
            echo "" > ${SEA_HOME}/.sea_evictlist
            echo "mem_all ${experiments[0]}" 
            launch_exp "$curr_exp" 390

        elif [[ $e == 1 ]]
        then
            curr_exp="mem_final"
            cp .sea_flushlist_mem ${SEA_HOME}/.sea_flushlist
            cp .sea_evictlist_mem ${SEA_HOME}/.sea_evictlist
            echo "mem_final ${experiments[0]}"
            launch_exp "$curr_exp" 39

        elif [[ $e == 2 ]]
        then
            curr_exp="lustre"
            echo "lustre ${experiments[1]}"
            launch_exp "$curr_exp" 390

        else
            curr_exp="disk"
            echo "disk ${experiments[2]}"
            launch_exp "$curr_exp" 0

        fi
        sleep 20
        echo "done experiment $curr_exp repetition $i"
    done
done
