#!/bin/bash
#SBATCH --time=90:55:00
#SBATCH --account=vhs
#SBATCH --job-name=lustre_3n_6t_6d_1000f_617m_10i
#SBATCH --nodes=3
#SBATCH --nodelist=comp02,comp03,comp04
#SBATCH --output=./results/exp_rw_10i/run-3/lustre_3n_6t_6d_1000f_617m_10i/slurm-%x-%j.out

source /home/vhs/Sea/.venv/bin/activate
    

srun -N3 ../scripts/clear_client_pc.sh
    
start=`date +%s.%N`
srun -N 1 bash ./results/exp_rw_10i/run-3/lustre_3n_6t_6d_1000f_617m_10i/n0_sea_parallel.sh &
srun -N 1 bash ./results/exp_rw_10i/run-3/lustre_3n_6t_6d_1000f_617m_10i/n1_sea_parallel.sh &
srun -N 1 bash ./results/exp_rw_10i/run-3/lustre_3n_6t_6d_1000f_617m_10i/n2_sea_parallel.sh &
wait

end=`date +%s.%N`

runtime=$( echo "$end - $start" | bc -l )

echo "Runtime: $runtime"
