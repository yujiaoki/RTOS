#! /bin/sh

sudo su
make
insmod simple.ko
mknod /dev/simple c 235 0
g++ -pthread main.c -o main
./main

