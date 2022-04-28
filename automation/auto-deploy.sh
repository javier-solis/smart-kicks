#!/usr/bin/env bash
# Author: javsolis@mit.edu
# Date: Apr 27, 2022
# Package Dependecies: rsync, sshpass
# User Dependecies: Having put the password as 1 line in a file thats NOT inside this 
#                   directory (see below for example)

if [ "$#" -eq  "0" ]
then
    echo "No arguments supplied, but expected the directory of where your smart-kicks repo is"
    exit 1
fi

cd $1.git/hooks/
echo "sshpass -f ./../608pass.txt rsync -ar ./server/ team44@608dev-2.net:~/" > pre-push

chmod +x pre-push

echo "DONE"