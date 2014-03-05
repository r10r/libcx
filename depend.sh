#!/bin/sh --posix
SRC="$1"
DIR=`dirname $SRC`
SUFFIX=o.mk

shift 1
# append dependency information
RULE=`cc -MM -MG "$@"`
#OBJ=`echo "$RULE" | cut -d':' -f 1`
SRC=`echo "$RULE" | cut -d':' -f 2`

cat <<EOF
${DIR}/${RULE}

SRC += ${SRC}
EOF
