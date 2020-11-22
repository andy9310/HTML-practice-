#!/bin/bash
n_host=$1
n_player=$2
declare -A host_id
for ((i = 0 ;i <= $n_host;i++ ));do  #creat fifo file
    pipe="fifo_$i.tmp"
    if [[ ! -p $pipe ]]; then
        mkfifo $pipe
        fd=$(($i+4))
        exec {fd}<>$pipe
    fi
    keylist[$i]=$(($i+100)) #should be random
    host_id[${keylist[$i]}]=$i
done
#exec host and ramdom generate key for host
for ((i = 1;i <= $n_host;i++));do
    exec ./host $i ${keylist[$i]} 0 &
done
sleep 1
#making combinations
cnt=0
for ((a = 1;a <= $(($n_player-7));a++));do
for ((b = $(($a+1));b <= $(($n_player-6));b++));do
for ((c = $(($b+1));c <= $(($n_player-5));c++));do
for ((d = $(($c+1));d <= $(($n_player-4));d++));do
for ((e = $(($d+1));e <= $(($n_player-3));e++));do
for ((f = $(($e+1));f <= $(($n_player-2));f++));do
for ((g = $(($f+1));g <= $(($n_player-1));g++));do
for ((h = $(($g+1));h <= $(($n_player));h++));do
    combinations[$cnt]="$a $b $c $d $e $f $g $h"
    #echo "${combinations[$cnt]}"
    let cnt=cnt+1
done
done
done
done
done
done
done
done

#close hosts
for ((i = 0 ; i < $n_host ; i++)); do
    combinations[(($cnt+$i))]="-1 -1 -1 -1 -1 -1 -1 -1"
done
#initail scorelist
for ((i = 0;i <= $n_player;i++));do
    scorelist[$i]=0
done
#send to host first time
for ((i = 0;i < $n_host;i++));do
    tmp=$(($i+1))
    fifo_write="fifo_$tmp.tmp"
    echo ${combinations[$i]} > $fifo_write
    #echo ${combinations[$i]}  #for test
done
#assign to free host and read data and sum up the score.
fifo_read="fifo_0.tmp"
for ((i = $n_host ; i < $(($cnt+$n_host)) ; i++)){
    read key <$fifo_read    # key
    h_id=${host_id[$key]}
    for ((j = 0;j < 8;j++));do
        read line <$fifo_read
        IFS=' ' read -a array <<< "$line" 
        scoreplayer=${array[0]}
        let scorelist[$scoreplayer]=$(( ${scorelist[$scoreplayer]}+8-${array[1]}))
    done
    # => free host
    fifo_write="fifo_$h_id.tmp"
    echo "${combinations[$i]}" > $fifo_write
} 

#print final score
for ((i = 1;i <= $n_player;i++));do
    echo "${i} ${scorelist[i]}" 
done
# remove fifo files
for ((i = 0;i <= $n_host;i++));do
    filepath="fifo_$i.tmp"
    rm $filepath
done
# Wait for all forked process to exit.


    



