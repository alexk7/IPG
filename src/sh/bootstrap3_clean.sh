#!/bin/sh
if [ "${ACTION}" = clean ]
then
	rm -f  ${BUILT_PRODUCTS_DIR}/bootstrap3/PEGParser.cpp
	rm -f  ${BUILT_PRODUCTS_DIR}/bootstrap3/PEGParser.h
fi
