#!/bin/bash

# Intended to be run from Git Bash on Windows

srcroot=$(dirname "$0")

pushd "$srcroot/OpenScanLib"
doxygen doc/Doxyfile
popd

pushd "$srcroot/OpenScanDeviceLib"
doxygen doc/Doxyfile
popd
