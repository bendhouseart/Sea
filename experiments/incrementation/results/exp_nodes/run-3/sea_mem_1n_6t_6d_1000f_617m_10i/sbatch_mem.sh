#!/bin/bash
#SBATCH --time=90:55:00
#SBATCH --account=vhs
#SBATCH --job-name=sea_mem_1n_6t_6d_1000f_617m_10i
#SBATCH --nodes=1
#SBATCH --nodelist=comp02
#SBATCH --output=./results/exp_nodes/run-3/sea_mem_1n_6t_6d_1000f_617m_10i/slurm-%x-%j.out

source /home/vhs/Sea/.venv/bin/activate
    
export SEA_HOME=/home/vhs/Sea

srun -N1 rm -rf /disk0/vhs/seatmp /disk1/vhs/seatmp /disk2/vhs/seatmp /disk3/vhs/seatmp /disk4/vhs/seatmp /disk5/vhs/seatmp /dev/shm/seatmp
srun -N1 echo "Clearing cache" && sync && echo 3 | sudo tee /proc/sys/vm/drop_caches

echo "Creating temp source mount directories"
srun -N1 mkdir /dev/shm/seatmp
srun -N1 mkdir /disk0/vhs/seatmp /disk1/vhs/seatmp /disk2/vhs/seatmp /disk3/vhs/seatmp /disk4/vhs/seatmp /disk5/vhs/seatmp
start=`date +%s.%N`
srun -N 1 bash ${SEA_HOME}/bin/sea_launch.sh ./results/exp_nodes/run-3/sea_mem_1n_6t_6d_1000f_617m_10i/n0_sea_parallel.sh &
wait

end=`date +%s.%N`

runtime=$( echo "$end - $start" | bc -l )

echo "Runtime: $runtime"
echo "Removing directories"
srun -N1 rm -rf /disk0/vhs/seatmp /disk1/vhs/seatmp /disk2/vhs/seatmp /disk3/vhs/seatmp /disk4/vhs/seatmp /disk5/vhs/seatmp /dev/shm/seatmp
