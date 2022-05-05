#!/usr/bin/env bash
# Usage: "Simulate" someone walking, by POSTing a series of equally seperated lat,lon points that follow a straight path
# Author: Javier Solis (javsolis@mit.edu)
# Date: April 26, 2022

# === CONFIG VARIABLES ===

START_LAT=42.3591871
START_LON=-71.0931501
# (Represents MIT's Lobby 7 Location)

DELTA_TIME=0.01 # in seconds

# With both of these combined, I'm saying that people walk at 1.11m/sec, roughly the average walking speed
DELTA_DIST_M=5 # in meters
BEARING=180 # degrees clockwise from North
NUM_PTS=50 # how many data points to have

USER=Joe
URL=http://608dev-2.net/sandbox/sc/team44/map/main.py # server URL

# === END OF CONFIG ===

NEW_LAT=$START_LAT
NEW_LON=$START_LON

COUNTER=0

while [  $COUNTER -lt $NUM_PTS ]; do
DELTA_DEGS=`python3 distance.py $NEW_LAT $NEW_LON $BEARING $DELTA_DIST_M`

NEW_LAT=`echo $DELTA_DEGS | awk '{ print $1 }'`
NEW_LON=`echo $DELTA_DEGS | awk '{ print $2 }'`

POST_MSG="user=$USER&lat=$NEW_LAT&lon=$NEW_LON"

curl -d $POST_MSG -X POST $URL

echo POST \#$COUNTER: \($NEW_LAT, $NEW_LON\)

COUNTER=$(($COUNTER+1))

sleep $DELTA_TIME
done