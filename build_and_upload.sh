#!/bin/bash

declare -A pods=(
    ["web"]="0.0.3"
    ["stat"]="0.0.1"
    ["senders"]="0.0.1"
    )

for i in ${!pods[@]}
do rm -rf $i.tar
podman build -t $i:${pods[$i]} -f $i.Dockerfile
podman save $i:${pods[$i]} -o $i.tar
scp -P 2202 $i.tar dashamail:/tmp
done;

declare -A hosts=(
    ["web"]="192.168.3.2"
    ["stat"]="192.168.3.3"
    ["senders"]="192.168.3.4"
)

for i in ${!hosts[@]}
do sshpass scp dashamail:/tmp/$i.tar ${hosts[$i]}:/tmp/$i.tar
sshpass -e ssh ${hosts[$i]} 'sudo docker load --input=/tmp/$i.tar'
done;
