#!/bin/bash

declare -A MATRIXL
declare -A MATRIXR

function readLeft() # iterates MATRIXL
{
    local ROW=0
    local COL=0
    local LINE
    local VAL

    while read LINE
    do
        COL=0
        for VAL in ${LINE}
        do
            MATRIXL[${ROW},${COL}]=${VAL}
            ((COL++))
        done
        ((ROW++))
    done < $1

    MATRIXL[ROWS]=${ROW}
    MATRIXL[COLS]=${COL}
}

function readRight() # iterates MATRIXR
{
    local ROW=0
    local COL=0
    local LINE
    local VAL

    while read LINE
    do
        COL=0
        for VAL in ${LINE}
        do
            MATRIXR[${ROW},${COL}]=${VAL}
            ((COL++))
        done
        ((ROW++))
    done < $1
    MATRIXR[ROWS]=${ROW}
    MATRIXR[COLS]=${COL}
}


if [ $# -lt 1 ]
then
    echo "Not enough arguments."
    exit 1
fi

if [ $1 = "dims" ] # prints dimensions of matrix
then
    readLeft $2
    echo -e "Rows: ${MATRIXL[ROWS]}\nColumns: ${MATRIXL[COLS]}"
    exit 0
fi

if [ $1 = "mean" ] # prints average of each column in matrix
then
    declare -a MEAN
    readLeft $2
    for (( ROW=0 ; ROW<${MATRIXL[ROWS]} ; ROW++ ))
    do
        for (( COL=0 ; COL<${MATRIXL[COLS]} ; COL++ ))
        do
            MEAN[${COL}]=$(( MEAN[${COL}]+MATRIXL[${ROW},${COL}] )) # find sum of each column
        done
    done

    for (( COL=0 ; COL<${MATRIXL[COLS]} ; COL++ ))
    do
        MEAN[${COL}]=$(( MEAN[${COL}]/${MATRIXL[ROWS]} )) # divide by number of rows in original matrix
        if [ ${COL} -ne 0 ]
        then
            printf " "
        fi
        printf "%d" ${MEAN[${COL}]}
    done

    printf "\n"

    exit 0
fi

if [ $1 = "transpose" ]
then
    declare -A TRANSPOSE
    readLeft $2
    for (( ROW=0 ; ROW<${MATRIXL[COLS]} ; ROW++ ))
    do
        for (( COL=0 ; COL<${MATRIXL[ROWS]} ; COL++ ))
        do
            TRANSPOSE[${ROW},${COL}]=${MATRIXL[${COL},${ROW}]}
        done
    done
    for (( ROW=0 ; ROW<${MATRIXL[COLS]} ; ROW++ ))
    do
        for (( COL=0 ; COL<${MATRIXL[ROWS]} ; COL++ ))
        do
            if [ ${COL} -ne 0 ]
            then
                printf " "
            fi
            printf "%d" ${TRANSPOSE[${ROW},${COL}]}
        done
        
        printf "\n"
    done

    exit 0
fi

if [ $1 = "add" ]
then
    if [ $# -lt 3 ]
    then
        echo "Not enough files given."
        exit 1
    fi

    declare -A SUM
    readLeft $2
    readRight $3

    if [ ${MATRIXL[ROWS]} -ne ${MATRIXR[ROWS]} ] || [ ${MATRIXL[COLS]} -ne ${MATRIXR[COLS]} ]
    then
        echo "Both matrices must have the same dimensions."
        exit 1
    fi

    for (( ROW=0 ; ROW<${MATRIXL[ROWS]} ; ROW++ ))
    do
        for (( COL=0 ; COL<${MATRIXL[COLS]} ; COL++ ))
        do
            SUM[${ROW},${COL}]=$(( ${MATRIXL[${ROW},${COL}]}+${MATRIXR[${ROW},${COL}]} ))
        done
    done

    for (( ROW=0 ; ROW<${MATRIXL[ROWS]} ; ROW++ ))
    do
        for (( COL=0 ; COL<${MATRIXL[COLS]} ; COL++ ))
        do
            if [ ${COL} -ne 0 ]
            then
                printf " "
            fi
            printf "%d" ${SUM[${ROW},${COL}]}
        done
        printf "\n"
    done

    exit 0
fi

if [ $1 = "mult" ]
then
    if [ $# -lt 3 ]
    then
        echo "Not enough files given."
        exit 1
    fi

    declare -A MULT
    readLeft $2
    readRight $3

    if [ ${MATRIXL[COLS]} -ne ${MATRIXR[ROWS]} ]
    then
        echo "Columns in first matrix must match rows in second matrix."
        exit 1
    fi

    for (( I=0 ; I<${MATRIXL[ROWS]} ; I++ ))
    do
        for (( J=0 ; J<${MATRIXR[COLS]} ; J++ ))
        do
            for (( K=0 ; K<${MATRIXL[COLS]} ; K++ ))
            do
                TMP=$(( MATRIXL[${I},${K}]*MATRIXR[${K},${J}] ))
                MULT[${I},${J}]=$(( MULT[${I},${J}]+${TMP} ))
            done
        done
    done

    for (( ROW=0 ; ROW<${MATRIXL[ROWS]} ; ROW++ ))
    do
        for (( COL=0 ; COL<${MATRIXR[COLS]} ; COL++ ))
        do
            if [ ${COL} -ne 0 ]
            then
                printf " "
            fi
            printf "%d" ${MULT[${ROW},${COL}]}
        done
        printf "\n"
    done

    exit 0
fi
