#!/bin/sh
if [ "${ACTION}" = clean ]
then
	rm -f  ${SRCROOT}/../../src/cpp/Common/Parser.cpp.tpl.h
	rm -f  ${SRCROOT}/../../src/cpp/Common/Parser.h.tpl.h
	rm -f  ${SRCROOT}/../../src/cpp/Dev/Dev-PEGParser.cpp
	rm -f  ${SRCROOT}/../../src/cpp/Dev/Dev-PEGParser.h
fi
