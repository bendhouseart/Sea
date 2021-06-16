#!/bin/bash

executable=$1
cond_file=$2
results_file=$3
runs=$4
exp_name=$5

sea_home=/home/vhs/Sea
out_fldr="/mnt/lustre/vhs/output/"
sea_mount="/mnt/lustre/vhs/mount/"
rdir="./results"
library="/home/vhs/Sea/build/sea.so"
#executable="python increment.py"
flush_out=flushlist.out
bench_out=benchmarks.out
alldisks=("/disk0/vhs/seatmp/" "/disk1/vhs/seatmp/" "/disk2/vhs/seatmp/" "/disk3/vhs/seatmp/" "/disk4/vhs/seatmp/" "/disk5/vhs/seatmp/")
cnodes=("comp02" "comp03" "comp04" "comp06" "comp07")

mkdir -p ${rdir}
ncond=$(jq ". | length" ${cond_file})

echo "experiment,repetition,runtime,flush_time,disk_files,total_flush" > ${results_file}

format_config () {
    ndisks=$1
    nthreads=$2
    fsize=$3
    exp_fldr=$4
    strategy=$5
    niterations=$6

    conf=${exp_fldr}/sea.ini
    flushlist=${exp_fldr}/.sea_flushlist
    evictlist=${exp_fldr}/.sea_evictlist

    disks=""

    for ((x=0;x<ndisks;x++))
    do
        if [[ $x -gt 0 ]]
        then
            disks+=","
        fi
        disks+="${alldisks[$x]}"
    done
    echo '# Sea configuration                                                      
[Sea]                                                                    
mount_dir = '"${sea_mount}"' ;           
n_levels = 3 ;                                                          
source_0 = /dev/shm/seatmp/ ;           
source_1 = '"$disks"' ;
source_2 = '"$out_fldr"' ;
log_level = 0 ; # 4 crashes tests                                        
log_file = /home/vhs/passthrough/sea.log ;
max_fs = '"$fsize"' ;
n_threads = '"$nthreads"' ;' > $conf

    cp $conf ${sea_home}

    final_it=$(($niterations-1))

    if [[ $strategy == "cp" ]]
    then
        echo "*" > $flushlist
    elif [[ $strategy == "mem" ]]
    then
        echo "" > $flushlist
        #TODO: uncomment
        echo "${final_it}*.nii" > $flushlist
    fi
    echo "${final_it}*.nii" > $evictlist

    cp $flushlist $evictlist ${sea_home}

}

launch_exp () {
    name=$1
    exp_dir=$2
    sfile=$3
    run=$4

    # launch experiment and obtain job id
    #TODO:remove nodelist
    slurm_id=$(sbatch ${sfile} | grep -Eo "[0-9]*")
    echo "Launched job ${name} run ${run} with id ${slurm_id}"

    # sleep to give enough time for experiment to start
    sleep 120

    # wait until experiment completes
    running=$(squeue --job ${slurm_id} | wc -l)
    while [[ ${running} == 2 ]]
    do
        sleep 10
        running=$(squeue --job ${slurm_id} | wc -l)
    done 

    echo "Slurm job complete"

    all_out="slurm-$name-${slurm_id}.out"
    all_file="${exp_dir}/${all_out}"
    flush_file="${exp_dir}/${flush_out}"
    bench_file="${exp_dir}/${bench_out}"

    grep -E "start,|end," ${all_file} > ${bench_file}

    res=$(grep -i runtime ${all_file}| grep -Eo "[0-9]*\.[0-9]+" )
    flushtime=0
    sddwrites=0
    totalflush=0

    if [[ $name != "lustre" ]]
    then
        grep -Ei "mv |cp |rm " ${all_file} > ${flush_file}
        flushtime=$(grep 'real' ${all_file} | grep "[0-9].*")
        flushtime=$(echo ${flushtime:5})
        ssdwrites=$(grep 'disk' ${flush_file} | sort --unique | wc -l)
        totalflush=$(cat ${flush_file} | sort --unique | wc -l)
    fi

    file_count=$(ls ${out_fldr} | wc -l)
    echo -e "Number of files currently on lustre: ${file_count}"
    echo "$name,$run,$res,$flushtime,$ssdwrites,$totalflush" >> ${results_file}
}

main () {

    run=$1
    declare -a EXP_ORDER=( $(for((i=0;i<ncond;i++)); do echo $i; done;) )

    echo "Preparing experiment launch"

    EXP_ORDER=( $(shuf -e "${EXP_ORDER[@]}") )
    for i in "${EXP_ORDER[@]}"
    do
        name=$(jq -r ".[$i] .name" ${cond_file})
        ndisks=$(jq ".[$i] .ndisks" ${cond_file})
        nnodes=$(jq ".[$i] .nnodes" ${cond_file})
        nthreads=$(jq ".[$i] .nthreads" ${cond_file})
        nfiles=$(jq ".[$i] .nfiles" ${cond_file})
        in_fldr=$(jq -r ".[$i] .in_fldr" ${cond_file})
        niterations=$(jq ".[$i] .niterations" ${cond_file})
        fsize=$(jq ".[$i] .fsize" ${cond_file})
        strategy=$(jq -r ".[$i] .strategy" ${cond_file})

        nodelist=""

        for ((x=0;x<nnodes;x++))
        do
            if [[ $x -gt 0 ]]
            then
                nodelist+=","
            fi
            nodelist+="${cnodes[$x]}"
        done


        echo "Experiment $i Run $run conditions
name: $name
strategy: $strategy
file input folder: ${in_fldr}
number of disks: $ndisks
number of nodes: $nnodes
number of threads: $nthreads
number of files: $nfiles
file size in bytes: $fsize
number of iterations: ${niterations}"
        exp_fldr="${rdir}/${exp_name}/run-${run}/${name}"
        rm -rf $exp_fldr/*
        echo -e "Experiment directory: ${exp_fldr}\n"

        fnode=$(($nfiles / $nnodes))
        rem=$(($nfiles % $nnodes))

        if [[ $strategy == "lustre" ]]
        then
            prefix=$executable
            out_mount=$out_fldr

        else
            prefix="LD_PRELOAD=$library $executable"
            out_mount=$sea_mount
        fi

        fcount=0
        node=0
        nprev=-1
        echo "Files to be executed per node: $fnode  remainder: $rem"

        job_dir="${exp_fldr}/jobs"
        mkdir -p ${job_dir}

        sl_script="${exp_fldr}/sbatch_${strategy}.sh"

        echo '#!/bin/bash
#SBATCH --time=90:55:00
#SBATCH --account=vhs
#SBATCH --job-name='"$name"'
#SBATCH --nodes='"$nnodes"'
#SBATCH --nodelist='"$nodelist"'
#SBATCH --output='"${exp_fldr}"'/slurm-%x-%j.out

source '"${sea_home}"'/.venv/bin/activate
    ' > ${sl_script}

        if [[ $strategy == "lustre" ]]
        then
            echo '
srun -N'"$nnodes"' ../scripts/clear_client_pc.sh
    ' >> ${sl_script}
        else
            echo 'export SEA_HOME='"${sea_home}"'

srun -N'"$nnodes"' rm -rf /disk0/vhs/seatmp /disk1/vhs/seatmp /disk2/vhs/seatmp /disk3/vhs/seatmp /disk4/vhs/seatmp /disk5/vhs/seatmp /dev/shm/seatmp
srun -N'"$nnodes"' echo "Clearing cache" && sync && echo 3 | sudo tee /proc/sys/vm/drop_caches

echo "Creating temp source mount directories"
srun -N'"$nnodes"' mkdir /dev/shm/seatmp
srun -N'"$nnodes"' mkdir /disk0/vhs/seatmp /disk1/vhs/seatmp /disk2/vhs/seatmp /disk3/vhs/seatmp /disk4/vhs/seatmp /disk5/vhs/seatmp' >> ${sl_script}
        fi

        echo 'start=`date +%s.%N`' >> ${sl_script}

        # flag to see if remainder was allotted to node (if applicable)
        if [[ $rem == 0 ]]
        then
            node_rem=true
        else
            node_rem=false
        fi

        for j in $(ls ${in_fldr} | head -n ${nfiles})
        do

            if [[ $fcount -ge $fnode ]]
            then

                if [[ $(($node + 1)) != $nnodes && ${node_rem} == true ]]
                then
                    echo "Number of files processed by node $node: $fcount"
                    fcount=0
                    node=$(($node + 1))

                    if [[ rem -gt 0 ]]
                    then
                        node_rem=false
                    fi
                else
                    rem=$(($rem - 1))
                    node_rem=true
                fi
            fi

            node_jobs="${job_dir}/jobs_n${node}.txt"

            if [[ $prefix == *"increment"* ]]
            then
                echo "$prefix ${in_fldr}$j ${out_mount} ${niterations}" >> ${node_jobs}
            else
                echo "$prefix ${in_fldr}$j" >> ${node_jobs}
            fi


            parallel_script=${exp_fldr}/n${node}_sea_parallel.sh

            if [[ $node != $nprev ]]
            then 
                echo "parallel --jobs ${nthreads} < ${node_jobs}" >> ${parallel_script}

                if [[ ${strategy} == "lustre" ]]
                then
                    echo "srun -N 1 bash ${parallel_script} &" >> ${sl_script}
                else
                    echo 'srun -N 1 bash ${SEA_HOME}/bin/sea_launch.sh '"${parallel_script}"' &' >> ${sl_script}
                fi
            fi
            nprev=$node

            fcount=$(($fcount + 1))
        done
        # for final node
        echo -e "Number of files processed by node $node: $fcount\n"

        echo 'wait

end=`date +%s.%N`

runtime=$( echo "$end - $start" | bc -l )

echo "Runtime: $runtime"' >> ${sl_script}

        if [[ $strategy != "lustre" ]]
        then
            echo 'echo "Removing directories"
srun -N'"${nnodes}"' rm -rf /disk0/vhs/seatmp /disk1/vhs/seatmp /disk2/vhs/seatmp /disk3/vhs/seatmp /disk4/vhs/seatmp /disk5/vhs/seatmp /dev/shm/seatmp' >> ${sl_script}
        fi
        # give lustre a break before removing all the files
        sleep 120
        echo "Removing files from output directory"
        rm ${out_fldr}/*;

        if [[ $strategy != "lustre" ]]
        then
            format_config $ndisks $nthreads $fsize $exp_fldr $strategy $niterations
        fi

        # clear data node pages cache
        sudo ansible-playbook ../scripts/clear_data_pc.yml
        launch_exp $name ${exp_fldr} ${sl_script} $run

    done
}

for((r=0;r<runs;r++))
do
    echo "Executing repetition $r"
    main $r
done
