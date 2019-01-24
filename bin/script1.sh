#! /bin/bash

printf "Homework 1\n\nProblem 6\n\n"&>file1
printf "\n******User Information******\n\n" 1>>file1
lslogins -u 1>>file1
printf "\n******OS Information******\n\n" 1>>file1
uname -o 1>>file1
lsb_release -i 1>>file1
lsb_release -d 1>>file1
lsb_release -r 1>>file1
lsb_release -c 1>>file1
printf "\nKernel Information\n\n" 1>>file1
uname -s 1>>file1
uname -r 1>>file1
gcc --version 1>>file1
uname -v 1>>file1
echo "Architecture: `uname -m`" 1>>file1
printf "\nFile System Memory\n\n" 1>>file1
df -h 1>>file1
