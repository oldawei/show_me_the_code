#!/bin/bash

# compiler: ./build.sh
# run qemu: ./build.sh qemu
# clean:    ./build.sh clean

if [ ! -f ./source/readme.txt ];then
    echo "Copy files..."
    cp -r ../s032/include ./
    cp -r ../s032/source ./
    cp -r ../s032/qemu.lds ./
    #cp -r ../s032/Makefile ./
fi

cp -r ./source036/* ./source/

if [ $# -eq 1 ]; then
    echo "make $1"
    make $1
else
    echo "make"
    make
fi

if [ $# -eq 1 ]; then
    if [ $1 == 'clean' ]; then
        echo "clean files..."
        rm -rf ./qemu.lds
        #rm -rf ./Makefile
        rm -rf ./include
        rm -rf ./source
    fi
fi