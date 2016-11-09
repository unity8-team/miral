#!/bin/bash
width=1920
height=1080
output=screencast.mp4
socket=${XDG_RUNTIME_DIR}/mir_socket
fifofile=$(mktemp)

while [ $# -gt 0 ]
do
    if [ "$1" == "--socket" ]; then shift; socket=$1; fi
    if [ "$1" == "--output" ]; then shift; output=$1; fi
    if [ "$1" == "--width"  ]; then shift; width=$1;  fi
    if [ "$1" == "--height" ]; then shift; height=$1; fi
    shift
done

echo width = ${width}
echo height = ${height}
echo output = ${output}
echo socket = ${socket}
echo fifofile = ${fifofile}

if ! which mirscreencast >> /dev/null ; then echo "Need mirscreencast - run \"sudo apt install mir-utils\""; exit 1 ;fi
if ! which mencoder >> /dev/null ;      then echo "Need mencoder - run \"sudo apt install mencoder\""; exit 1 ;fi

if [ -e ${output} ]; then echo "Output exists, moving to ${output}~"; mv ${output} ${output}~ ;fi
while [ ! -e "${socket}" ]; do echo "waiting for ${socket}"; sleep 1 ;done

mkfifo ${fifofile}
mirscreencast -n -1 -m ${socket} -f ${fifofile}& mirscreencast_pid=$!

sleep 1; # don't lose the next message in the spew from mirscreencast
read -rsp $'\n\nPress enter when recording complete...'
kill ${mirscreencast_pid}

mencoder -demuxer rawvideo -rawvideo fps=60:w=${width}:h=${height}:format=bgra -ovc x264 -o ${output} ${fifofile}
rm ${fifofile}
