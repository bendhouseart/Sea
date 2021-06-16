#!/bin/bash

infile=$1

start=`date +%s`
ls $infile
end=`date +%s`

pid=`echo $$`
host=`hostname`
echo "ls_start,${infile},${start},${pid},${host}"
echo "ls_end,${infile},${start},${pid},${host}"
