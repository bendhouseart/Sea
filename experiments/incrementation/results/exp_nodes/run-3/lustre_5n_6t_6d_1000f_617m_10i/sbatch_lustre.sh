#!/bin/bash
#SBATCH --time=90:55:00
#SBATCH --account=vhs
#SBATCH --job-name=lustre_5n_6t_6d_1000f_617m_10i
#SBATCH --nodes=5
#SBATCH --nodelist=comp02,comp03,comp04,comp06,comp07
#SBATCH --output=./results/exp_nodes/run-3/lustre_5n_6t_6d_1000f_617m_10i/slurm-%x-%j.out

source /home/vhs/Sea/.venv/bin/activate
    

srun -N5 ../scripts/clear_client_pc.sh
    
start=`date +%s.%N`
srun -N 1 bash ./results/exp_nodes/run-3/lustre_5n_6t_6d_1000f_617m_10i/n0_sea_parallel.sh &
srun -N 1 bash ./results/exp_nodes/run-3/lustre_5n_6t_6d_1000f_617m_10i/n1_sea_parallel.sh &
srun -N 1 bash ./results/exp_nodes/run-3/lustre_5n_6t_6d_1000f_617m_10i/n2_sea_parallel.sh &
srun -N 1 bash ./results/exp_nodes/run-3/lustre_5n_6t_6d_1000f_617m_10i/n3_sea_parallel.sh &
srun -N 1 bash ./results/exp_nodes/run-3/lustre_5n_6t_6d_1000f_617m_10i/n4_sea_parallel.sh &
wait

end=`date +%s.%N`

runtime=$( echo "$end - $start" | bc -l )

echo "Runtime: $runtime"
