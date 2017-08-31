#!/bin/bash

PIDFILE=/var/run/templeton_esp.pid

case $1 in
   start)
      /home/pi/templeton_esp/main.py 2>/dev/null &
      echo $! > ${PIDFILE} 
   ;;
   stop)
      kill `cat ${PIDFILE}`
      rm ${PIDFILE}
   ;;
   *)
      echo "usage: {start|stop}" ;;
esac
exit 0
