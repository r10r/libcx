#!/bin/sh

uncrustify -c ${LIBCX_DIR}/uncrustify.cfg -p uncrustify.debug --replace $@ \
	&& touch .format