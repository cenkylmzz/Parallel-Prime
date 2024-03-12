#!/bin/bash

# Define the executable file
EXECUTABLE="./main.o"

# Specify different values of M and Chunk sizes
M_VALUES=(100 250 500 1000 2000 3000 5000 7500 10000 50000 1000000 1500000 3000000 5000000)
CHUNK_SIZES=(10 50 100 1000 5000 10000 50000 100000)

# Loop over M values
for M in "${M_VALUES[@]}"; do
    # Loop over Chunk sizes
    for CHUNK_SIZE in "${CHUNK_SIZES[@]}"; do
        echo "Running with M=$M and Chunk Size=$CHUNK_SIZE"
        
        # Execute the C++ program with the current M and Chunk Size
        $EXECUTABLE $M $CHUNK_SIZE

        # Add a separator for better visibility
        echo "-------------------------------------------"
    done
done
