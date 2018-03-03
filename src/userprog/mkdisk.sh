#!/bin/bash

cd build
# This script creates a filesystem and formats it
echo "This script creates a filesystem and formats it ctrl+c now to exit"
read bs_var
pintos-mkdisk filesys.dsk --filesys-size=2
pintos -f -q

