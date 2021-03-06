#!/bin/sh

rm -r Build
mkdir Build
cd Build
echo Compiling...
g++-6 --std=c++14 -I ../Include  -c ../Source/Entrance.cpp ../Source/RaspiIO.cpp ../Source/SSD1322.cpp
echo Linking...
g++-6 --std=c++14 Entrance.o RaspiIO.o SSD1322.o -o test
echo Testing...
sudo ./test