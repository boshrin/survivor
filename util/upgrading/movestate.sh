#!/bin/sh

# Script to move v0.8.x check and alert state to v0.9 format.
# The scheduler must not be running while this script is run.
#
# Usage: movestate.sh <statedir>

# Version: $Revision: 0.1 $
# Author: Benjamin Oshrin
# Date: $Date: 2004/03/02 17:49:39 $

if [ "$1" = "" ]; then
    echo "Usage:" $0 "<statedir>"
    exit
fi

# First, move the service check state.  This is easiest.

for i in $1/service/*
do
    if [ -r $i/lastcheck ]; then
	/bin/mv $i/lastcheck $i/checkstatus
    fi
done

# Next, move the host check state.

for i in $1/host/*/*
do
    /bin/cat $i/checkstatus >> $i/lastcheck
    /bin/mv $i/lastcheck $i/checkstatus
done

# Finally, move the host alert state.

for i in $1/host/*/*
do
    if [ -r $i/alertstatus ] && [ -r $i/lastalert ]; then
	RC=`head -1 $i/alertstatus`
	CONSECUTIVE=`tail -1 $i/alertstatus`
	TIME=`head -1 $i/lastalert`
	CRC=`head -2 $i/lastalert | tail -1`
	NOTIFY=`tail -1 $i/lastalert`

	/bin/rm $i/alertstatus $i/lastalert
	echo ${TIME} > $i/alertstatus
	echo ${CRC} >> $i/alertstatus
	echo ${CONSECUTIVE} >> $i/alertstatus
	echo ${RC}:unknown:${NOTIFY} >> $i/alertstatus
	chmod 664 $i/alertstatus
    fi
done

exit 0
