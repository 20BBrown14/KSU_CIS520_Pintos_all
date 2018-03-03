#!/bin/bash

cd build
# copies some of the examples to the disk 
echo "This script copies some examples to the disk, hit enter to run"
read bs_var
pintos -p ../../examples/halt -a halt -- -q
pintos -p ../../examples/echo -a echo -- -q
pintos -p ../../examples/cat -a cat -- -q
pintos -p ../../examples/cp -a cp -- -q

# display the files on the pintos disk
pintos -q ls
