#!/usr/bin/env bash

sshpass -f ./../../608pass.txt rsync -ar ./../server/ team44@608dev-2.net:~/

echo "DONE"