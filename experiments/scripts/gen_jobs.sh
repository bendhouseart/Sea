#!/usr/bin/bash


# Script to generate gnu parallel input files of type
# "LD_PRELOAD=$library python increment.py $input_folder/bigbrain_0_0_0.nii $mount"
nnodes=$1
nfiles=$2
input_folder=$3
mount=$4
iterations=$5
preload=$6


library="/home/vhs/Sea/build/sea.so"
executable="python increment.py"

fnode=$(($nfiles / $nnodes))
rem=$(($nfiles % $nnodes))
echo $fnode $rem

fcount=0
echo $fcount
node=0
for i in $(ls ${input_folder} | head -n ${nfiles})
do
    echo $fcount $fnode
    if [[ $fcount == $fnode ]]
    then
        fcount=0

        if [[ $(($node + 1)) != $nnodes ]]
        then
            node=$(($node + 1))
        fi
    fi

    echo "LD_PRELOAD=$library $executable ${input_folder}/$i $mount" >> testmp_$node.txt
    fcount=$(($fcount + 1))
done





