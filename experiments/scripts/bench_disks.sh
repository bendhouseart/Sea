#!/bin/bash

device=$1
bench_file=$2
sync; echo 3 | sudo tee /proc/sys/vm/drop_caches

write_out=$((dd if=/dev/urandom of=$device bs=2M count=512 conv=fsync,notrunc status=progress)  2>&1)
echo ${write_out}
write_bw=`echo ${write_out} | awk '{print $(NF-1),",",$NF}' | tr -d ' '`
echo ${device},write,${write_bw} >> ${bench_file}

#ssh cgpu01 "sync; echo 3 | sudo tee /proc/sys/vm/drop_caches"
cached_out=$((dd if=$device of=/dev/null bs=2M count=512 status=progress) 2>&1)
echo ${cached_out}
cached_bw=`echo ${cached_out} | awk '{print $(NF-1),",",$NF}' | tr -d ' '`
echo ${device},cached,${cached_bw} >> ${bench_file}

echo 3 | sudo tee /proc/sys/vm/drop_caches
if [[ $device == *"lustre"* ]]
then
    ssh ct01 "sudo ansible-playbook /home/vhs/Sea/experiments/scripts/clear_data_pc.yml"
fi

read_out=$((dd if=$device of=/dev/null bs=2M count=512 status=progress) 2>&1)
echo ${read_out}
read_bw=`echo ${read_out} | awk '{print $(NF-1),",",$NF}' | tr -d ' '`
echo ${device},read,${read_bw} >> ${bench_file}

rm $device

