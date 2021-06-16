#!/bin/bash
#SBATCH --time=90:55:00
#SBATCH --account=vhs
#SBATCH --job-name=lustre_7n_6t_6d_1000f_617m_5i
#SBATCH --nodes=7
#SBATCH --output=./results/exp_lustre_rw/run-4/lustre_7n_6t_6d_1000f_617m_5i/slurm-%x-%j.out

source /home/vhs/Sea/.venv/bin/activate
    

srun -N7 echo "Clearing cache" && sync && echo 3 | sudo tee /proc/sys/vm/drop_caches
    
start=`date +%s.%N`
srun -N 1 bash ./results/exp_lustre_rw/run-4/lustre_7n_6t_6d_1000f_617m_5i/n0_sea_parallel.sh &
srun -N 1 bash ./results/exp_lustre_rw/run-4/lustre_7n_6t_6d_1000f_617m_5i/n1_sea_parallel.sh &
srun -N 1 bash ./results/exp_lustre_rw/run-4/lustre_7n_6t_6d_1000f_617m_5i/n2_sea_parallel.sh &
srun -N 1 bash ./results/exp_lustre_rw/run-4/lustre_7n_6t_6d_1000f_617m_5i/n3_sea_parallel.sh &
srun -N 1 bash ./results/exp_lustre_rw/run-4/lustre_7n_6t_6d_1000f_617m_5i/n4_sea_parallel.sh &
srun -N 1 bash ./results/exp_lustre_rw/run-4/lustre_7n_6t_6d_1000f_617m_5i/n5_sea_parallel.sh &
srun -N 1 bash ./results/exp_lustre_rw/run-4/lustre_7n_6t_6d_1000f_617m_5i/n6_sea_parallel.sh &
wait

end=`date +%s.%N`

runtime=$( echo "$end - $start" | bc -l )

echo "Runtime: $runtime"
