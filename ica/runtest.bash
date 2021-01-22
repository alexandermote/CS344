#!/bin/bash

function show_help
{
    echo "$0 [-h] [-b <begin>] [-e <end ] [-v]"
    echo "  Today's date is " `date`
    exit 1
}

VERBOSE=0
START=0
LAST=0

while getopts "hb:e:v" arg
do
    case ${arg} in
	"h")
	    show_help
	    ;;
	"b")
	    START=${OPTARG}
	    ;;
	"e")
	    LAST=${OPTARG}
	    ;;
	"v")
	    VERBOSE=1
	    ;;
    esac
done

if [ ${START} -eq 0 -a ${LAST} -eq 0 ]
then
    show_help
fi

if [ ${START} -lt 1 ]
then
    START=1
fi

if [ ${LAST} -lt 1 ]
then
    LAST=10
fi

if [ ${LAST} -lt ${START} ]
then
    show_help
fi

if [ ${VERBOSE} -ne 0 ]
then
    echo "Begin value = ${START}    End value = ${LAST}"
fi

for (( VAL=${START}; VAL<=${LAST}; VAL++ ))
do
    ./foo ${VAL}
    RETVAL=$?
    if [ ${RETVAL} -eq 0 ]
    then
	echo "    ${VAL} was successful."
    elif [ ${VERBOSE} -ne 0 ]
    then
	echo "    ${VAL} returned ${RETVAL}."
    fi
done
