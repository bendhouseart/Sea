#!/bin/bash

#set -e
set -u
#set -x

base_source=""
sources_arr=()
n_sources=0
conf_file=${SEA_HOME}/sea.ini
flush_file=${SEA_HOME}/.sea_flushlist
evict_file=${SEA_HOME}/.sea_evictlist
tmpfile=$(mktemp /tmp/.sea_flush.inprog.XXXXXX)

fe_old () {

    found_files=$1 # files that currently exist in source mounts
    OLDTIME=$2 # user specified expiration time
    task=$3 # mv / cp / rm (depending on which list)
    flist="" # list containing files for flush/eviction

    for f in $found_files
    do
        # code adapted from here https://stackoverflow.com/a/28341234
        # Get current and file times
        CURTIME=$(date +%s)
        FILETIME=$(stat $f -c %Y || true && continue)

        if [[ $FILETIME == "" ]]
        then
            continue
        fi

        # calculate how old the file is (based on access time)
        TIMEDIFF=$(($CURTIME - $FILETIME))

        # Check if file older
        if [ $TIMEDIFF -ge $OLDTIME ]; then
            echo "$task $f to $base_source"

            # some string formatting
            if [[ "$flist" != "" ]]
            then
                flist+=" "
            fi

            flist+=$f
           
        fi
    done

    # if we have some files that older that OLDTIME, flush/evict them
    if [[ ${flist} != "" ]]
    then

        # process each image individually (as opposed to all at once)
        if [[ $task == "mv" ]]
        then 
            mv $flist ${base_source} || true
        elif [[ $task == "cp" ]]
        then 
            cp $flist ${base_source} || true
        elif [[ $task == "rm" ]]
        then 
            rm $flist || true
        fi

        # wait for all images to be processed
        #wait
    fi
}



get_rgx () {
    SOURCE=$1
    FILE=$2
    rgx=""

    if [ -f $FILE ]
    then
        while IFS="" read -r re || [ -n "$re" ]
        do
            if [[ $rgx != "" ]]
            then
                rgx+=" "
            fi

            rgx+=$SOURCE/$re

        done < $FILE

    fi
    echo $rgx
}

flush () {
        OLDTIME=$1 # user specified expiration time
        task=$2 # the name of the calling task. either "process" or "cleanup"

        re_flush="" # regex of files to flush
        re_evict="" # regex of files to pin to local disk (don't flush or evict)
        re_all="" # regex for all files
        flush_files="" # files to flush only
        evict_files="" # files to evict only
        fe_files="" # files to flush and evict
        all_files="" # all files in source mounts
        sources_str="" # string containing all source paths

        # load all potential regex from file (concatenate the regex to existing source mounts)
        for s in "${sources_arr[@]}"
        do
            # load rgx of files that need to be flushed
            if [[ $re_flush != "" ]]
            then
                re_flush+=" "
            fi
            re_flush+=$(get_rgx $s $flush_file)

            # do the same for files that need to be pinned
            if [[ $re_evict != "" ]]
            then
                re_evict+=" "
            fi
            re_evict+=$(get_rgx $s $evict_file)

            if [[ $re_all != "" ]]
            then
                re_all+=" "
            fi
            re_all+=$s/*
        done

        for rgx in $re_all
        do
            if [[ $all_files != "" ]]
            then
                all_files+=" "
            fi
            all_files+=$(find ${rgx} -type f -follow 2> /dev/null || true)
        done

        # if .sea_flushlist file contains regex
        for rgx in $re_flush
        do
            if [[ $flush_files != "" ]]
            then
                flush_files+=" "
            fi
            flush_files+=$(echo $all_files | grep -Eo $rgx || true)
        done
        #echo "re_flush $re_flush"
        #echo "flush_files $flush_files"

        # if .sea_evictlist file contains regex
        for rgx in $re_evict
        do
            if [[ $evict_files != "" ]]
            then
                evict_files+=" "
            fi
            evict_files+=$(echo $all_files | grep -Eo $rgx || true)
        done

        tmp_flush=""
        tmp_evict=""

        # find the files that only need to be flushed as well as those that need
        # to be flushed and evicted
        for ff in $flush_files
        do
            match=0
            for ef in $evict_files
            do
                if [[ $ff == $ef ]]
                then
                    match=1

                    if [[ $fe_files != "" ]]
                    then
                        fe_files+=" "
                    fi
                    fe_files+=$ff
                    break
                fi
            done
            if [[ $match == 0 ]]
            then
                if [[ $tmp_flush != "" ]]
                then
                    tmp_flush+=" "
                fi
                tmp_flush+=$ff
            fi
        done

        # find files that only need to be evicted
        for ef in $evict_files
        do
            match=0
            for fe in $fe_files
            do
                if [[ $ef == $fe ]]
                then
                    match=1
                    break
                fi
            done
            if [[ $match == 0 ]]
            then
                if [[ $tmp_evict != "" ]]
                then
                    tmp_evict+=" "
                fi
                tmp_evict+=$ef
            fi
        done

        flush_files=$tmp_flush
        evict_files=$tmp_evict

        #echo "mv files $fe_files"
        #echo "cp $flush_files"
        #echo "rm $evict_files"

        # check to see if there are any candidate files for flush/eviction and process them
        if [[ $fe_files != "" ]]
        then
            fe_old "${fe_files}" "$OLDTIME" "mv"
        fi
        if [[ $flush_files != "" ]]
        then
            fe_old "${flush_files}" "$OLDTIME" "cp"
        fi
        if [[ $evict_files != "" ]]
        then
            fe_old "${evict_files}" "$OLDTIME" "rm"
        fi
}


flush_process () {
    OLDTIME=$1
    echo "starting flusher"
    while [[ -f $tmpfile ]]
    do
        flush $OLDTIME "process"
        sleep 5 &
        wait $!
    done
    echo "flush process terminated"
    echo "starting cleanup"
    time flush 0 "cleanup"
}

cleanup () {
    echo "Cleaning up Sea mount"
    time flush 0 "cleanup"
}


get_sources () {

    # refers to the last index and not the total number of sources
    n_lvls="$(($(awk -F "=" '/n_levels/ {print $2}' ${SEA_HOME}/sea.ini | tr -d ' ;') - 1))"

    echo $n_lvls
    for ((i=0;i<${n_lvls};i++))
    do
        source_lvl=$(cat ${conf_file} | grep "^\s*source_$i" | cut -d "=" -f 2 | tr -d ' ;')
        IFS=',' read -ra curr_sources <<< "$source_lvl"

        sources_arr+=(${curr_sources[@]})

        echo source_$i ${sources_arr[@]}
    done

    base_source=$(cat ${conf_file} | grep "^\s*source_$i" | cut -d "=" -f 2 | tr -d ' ;')
    echo base dir ${base_source}

}

#get_sources
#flush 5 "test func"

trap '[ -f $tmpfile ] && rm $tmpfile' SIGTERM SIGINT EXIT

get_sources
flush_process 5
