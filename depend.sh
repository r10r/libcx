#!/bin/sh --posix
readonly OBJ_DIR=`dirname $1`
shift 1

# append dependency information
readonly RULE=`cc -MM -MG "$@"`
readonly OBJ=`echo "$RULE" | cut -d':' -f 1`
readonly SRC=`echo "$RULE" | cut -d':' -f 2`

cat <<EOF
${OBJ_DIR}/${RULE}

SRC += ${SRC}
EOF
