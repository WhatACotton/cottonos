#!/bin/bash

# commit args
COMMIT_MSG="$1"

# Define directories to compare
DIRS=(
    "vitis/os/src:workspace/os/cottonos_os/src"
)

for DIR_PAIR in "${DIRS[@]}"; do
    IFS=':' read -r DIR_A DIR_B <<< "$DIR_PAIR"
    
    if [ ! -d "$DIR_B" ]; then
        echo "Directory $DIR_B does not exist. Exiting."
        exit 1
    fi

    if ! diff -qr "$DIR_A" "$DIR_B" > /dev/null; then
        echo "Differences found between $DIR_A and $DIR_B. Copying changes from $DIR_B to $DIR_A..."
        
        rsync -av --exclude='.git/' "$DIR_B/" "$DIR_A/"
        
        echo "Files copied from $DIR_B to $DIR_A."
    else
        echo "No differences found between $DIR_A and $DIR_B."
    fi
done