#!/bin/bash
clear
# run from /src/userprog

# first make examples
echo "do the laundry list"
echo "make examples"
echo "make clean in userprog"
echo "recreate the disk, formatting it"
echo "finally it copies some examples to the disk"
echo "last chance to ctrl+c otherwise hit any key"
read bs_var
cd ../examples
make clean	
make

cd -
make clean
make

cd build
#make the file system and format it
pintos-mkdisk filesys.dsk --filesys-size=2
pintos -f -q

#copy some of our examples to the disk we just created
pintos -p ../../examples/halt -a halt -- -q
pintos -p ../../examples/echo -a echo -- -q
pintos -p ../../examples/cat -a cat -- -q
pintos -p ../../examples/cp -a cp -- -q
pintos -p ../../examples/lorem -a lorem -- -q

pintos -q ls
