#!/bin/sh
if [ "${ACTION}" = clean ]
then
	rm -f  ${BUILT_PRODUCTS_DIR}/PEGParser.cpp.tpl.h
	rm -f  ${BUILT_PRODUCTS_DIR}/PEGParser.h.tpl.h
fi
