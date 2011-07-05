#!/bin/sh
if [ "${ACTION}" = clean ]
then
	rm -f  ${SRCROOT}/../../src/cpp/IPG-Dev/Parser.cpp.tpl.h
	rm -f  ${SRCROOT}/../../src/cpp/IPG-Dev/Parser.h.tpl.h
	rm -f  ${SRCROOT}/../../src/cpp/IPG-Dev/PEGParser.cpp
	rm -f  ${SRCROOT}/../../src/cpp/IPG-Dev/PEGParser.h
fi
