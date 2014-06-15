#!/bin/sh
# Automates wstest and leaks checking for automated load testing

WSTEST_JSON="reports/libcx/index.json"
TESTNUM=0
SERVER_PID=$1
OUT_DIR=data

run_wstest()
{
    wstest -m fuzzingclient
    cp $WSTEST_JSON ${OUT_DIR}/${TESTNUM}.json
    local failed=`cat $WSTEST_JSON | grep "\"FAILED\"" | wc -l | tr -d " "`
    echo "Failed: $failed"
    return $failed
}

run_leaks()
{
    local leaks=`leaks $SERVER_PID| grep 'total leaked bytes' | cut -d' ' -f 3`
    echo "Leaks: $leaks"
    return $leaks
}

while true
do
    echo "== [ Test $TESTNUM ]"
    run_wstest >> ${OUT_DIR}/${TESTNUM}.log || break
    run_leaks >> ${OUT_DIR}/${TESTNUM}.log || break
    ((TESTNUM++))
done
    



        
    
    
