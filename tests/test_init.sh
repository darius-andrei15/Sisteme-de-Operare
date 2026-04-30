#!/bin/bash

./tools/fileops.sh init

for dir in bin src include data logs reports tmp/obj tests doc tools; do
    if [ ! -d "$dir" ]; then
        echo "Directorul $dir lipsește!"
        exit 1 
    fi
done

exit 0 
