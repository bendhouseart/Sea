#!/bin/bash
#SBATCH --time=90:55:00
#SBATCH --account=vhs
#SBATCH --job-name=sea_cp_5n_1t_6d_1000f_617m_5i
#SBATCH --nodes=5
#SBATCH --nodelist=comp02,comp03,comp04,comp06,comp07
#SBATCH --output=./results/exp_threads/run-3/sea_cp_5n_1t_6d_1000f_617m_5i/slurm-%x-%j.out

source /home/vhs/Sea/.venv/bin/activate
    
export SEA_HOME=/home/vhs/Sea

srun -N5 rm -rf /disk0/vhs/seatmp /disk1/vhs/seatmp /disk2/vhs/seatmp /disk3/vhs/seatmp /disk4/vhs/seatmp /disk5/vhs/seatmp /dev/shm/seatmp
srun -N5 echo "Clearing cache" && sync && echo 3 | sudo tee /proc/sys/vm/drop_caches

echo "Creating temp source mount directories"
srun -N5 mkdir /dev/shm/seatmp
srun -N5 mkdir /disk0/vhs/seatmp /disk1/vhs/seatmp /disk2/vhs/seatmp /disk3/vhs/seatmp /disk4/vhs/seatmp /disk5/vhs/seatmp
start=`date +%s.%N`
srun -N 1 bash ${SEA_HOME}/bin/sea_launch.sh ./results/exp_threads/run-3/sea_cp_5n_1t_6d_1000f_617m_5i/n0_sea_parallel.sh &
srun -N 1 bash ${SEA_HOME}/bin/sea_launch.sh ./results/exp_threads/run-3/sea_cp_5n_1t_6d_1000f_617m_5i/n1_sea_parallel.sh &
srun -N 1 bash ${SEA_HOME}/bin/sea_launch.sh ./results/exp_threads/run-3/sea_cp_5n_1t_6d_1000f_617m_5i/n2_sea_parallel.sh &
srun -N 1 bash ${SEA_HOME}/bin/sea_launch.sh ./results/exp_threads/run-3/sea_cp_5n_1t_6d_1000f_617m_5i/n3_sea_parallel.sh &
srun -N 1 bash ${SEA_HOME}/bin/sea_launch.sh ./results/exp_threads/run-3/sea_cp_5n_1t_6d_1000f_617m_5i/n4_sea_parallel.sh &
wait

end=`date +%s.%N`

runtime=$( echo "$end - $start" | bc -l )

echo "Runtime: $runtime"
echo "Removing directories"
srun -N5 rm -rf /disk0/vhs/seatmp /disk1/vhs/seatmp /disk2/vhs/seatmp /disk3/vhs/seatmp /disk4/vhs/seatmp /disk5/vhs/seatmp /dev/shm/seatmp
