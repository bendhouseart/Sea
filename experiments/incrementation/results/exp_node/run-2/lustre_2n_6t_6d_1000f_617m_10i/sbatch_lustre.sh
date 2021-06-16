#!/bin/bash
#SBATCH --time=90:55:00
#SBATCH --account=vhs
#SBATCH --job-name=lustre_2n_6t_6d_1000f_617m_10i
#SBATCH --nodes=2
#SBATCH --nodelist=comp02,comp03
#SBATCH --output=./results/exp_node/run-2/lustre_2n_6t_6d_1000f_617m_10i/slurm-%x-%j.out

source /home/vhs/Sea/.venv/bin/activate
    

srun -N2 ../scripts/clear_client_pc.sh
    
start=`date +%s.%N`
srun -N 1 bash ./results/exp_node/run-2/lustre_2n_6t_6d_1000f_617m_10i/n0_sea_parallel.sh &
srun -N 1 bash ./results/exp_node/run-2/lustre_2n_6t_6d_1000f_617m_10i/n1_sea_parallel.sh &
wait

end=`date +%s.%N`

runtime=$( echo "$end - $start" | bc -l )

echo "Runtime: $runtime"
