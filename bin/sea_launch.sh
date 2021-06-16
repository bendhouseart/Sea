#!/bin/bash

set -e
set -u

trap "exit" INT TERM
#trap "kill 0" EXIT

conf_file=${SEA_HOME}/sea.ini
curr_dir=`dirname "$(readlink -f "$0")"`

n_sources="$(($(awk -F "=" '/n_sources/ {print $2}' ${conf_file} | tr -d ' ;') - 1))"

for ((i=0;i<${n_sources};i++))
do
    source=($(cat ${conf_file} | grep "source_$i" | cut -d "=" -f 2 | tr -d ' ;'))
    mkdir -p ${source} 
done

${curr_dir}/sea_flusher.sh &
pid=$!
echo "flusher pid" ${pid}

#trap 'trap - SIGTERM && kill $pid' SIGINT SIGTERM EXIT

echo "Launching script $1"
bash $1
echo "killing flush" && kill $pid

wait $pid

