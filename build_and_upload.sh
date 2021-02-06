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

sshpass scp dashamail:/tmp/web.tar 192.168.3.2:/tmp/web.tar
sshpass scp dashamail:/tmp/stat.tar 192.168.3.3:/tmp/stat.tar
sshpass scp dashamail:/tmp/senders.tar 192.168.3.4:/tmp/senders.tar