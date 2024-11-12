#!/bin/bash

# if [ ! -d `pwd`/test ];then
#     mkdir `pwd`/test
# fi
if [ -f `pwd`/Pine ];then
    rm -f `pwd`/Pine
fi
if [ -d `pwd`/build ];then
    rm -rf `pwd`/build
fi

mkdir `pwd`/build

cd build
cmake ..
make

if [ -f `pwd`/Pine ];then
cp Pine ../
cd ..
./Pine --flagfile start.conf
fi


