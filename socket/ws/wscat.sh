#!/bin/sh
WORKDIR=$(dirname $0)
export NODE_PATH=/usr/local/share/npm/lib/node_modules

node $WORKDIR/wscat.js $@

