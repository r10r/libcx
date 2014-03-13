#!/bin/sh --posix
readonly OBJ_DIR=`dirname $1`
shift 1

# append dependency information
# make multiline output single line
readonly RULE=$(cc -MM -MG "$@" | tr -d '\\\n')
readonly OBJ=`echo ${RULE} | cut -d':' -f 1`
readonly SRC=`echo ${RULE} | cut -d':' -f 2`

# both object and dependency file must be updated
# when a source file or header changes
# SRC += adds duplicate headers/includes
# use '$(sort $(SRC))' to remove duplicates
cat <<EOF
${OBJ_DIR}/${OBJ} ${OBJ_DIR}/${OBJ}.mk: ${SRC}

SRC += ${SRC}
EOF
