#!/bin/sh
if [ "${ACTION}" = clean ]
then
	rm -f  ${BUILT_PRODUCTS_DIR}/bootstrap2/PEGParser.cpp
	rm -f  ${BUILT_PRODUCTS_DIR}/bootstrap2/PEGParser.h
fi
