#!/bin/bash

rm -rf src/LogInsight/build
cd src/LogInsight
mkdir build
cmake .
make
