#!/bin/sh
OBJ=$1
# BSD version of basename is different to GNU version
# awk simulating basename (with suffix removal)
OBJ_NAME=`echo ${OBJ} | awk -F/ '{print $NF}' | cut -d. -f1`
OBJ_DIR=`dirname ${OBJ}`

GCNO_FILE=${OBJ_DIR}/${OBJ_NAME}.gcno
GCDA_FILE=${OBJ_DIR}/${OBJ_NAME}.gcda
COV_FILE=${OBJ_DIR}/${OBJ_NAME}.cov

if [ -f ${GCNO_FILE} -a -f ${GCDA_FILE} ]
	then
	llvm-cov-3.4 -gcno=${GCNO_FILE} -gcda=${GCDA_FILE} > ${COV_FILE}
elif [ -f ${GCNO_FILE} ]
	then
	llvm-cov-3.4 -gcno=${GCNO_FILE} > ${COV_FILE}
fi
	