#!/bin/bash

# Output CSV file
output_csv="resultsThrust.csv"

# Thrust code executable file, compiled with nvcc
thrust_exec="./thrust.out"

# Sizes and processes configurations
sizes=("20" "50" "100" "500" "1000" "2000" "5000" "10000" "20000" "50000" "100000" "200000" "500000" "1000000" "2000000" "5000000" "10000000" "20000000" "50000000" "100000000")
processes=("1" "2" "4" "8")

# Create CSV header if file does not exist
if [ ! -f "$output_csv" ]; then
    touch "$output_csv"
    echo "M,T1,T2,T4,T8,S2,S4,S8" >> "$output_csv"
fi

# Loop through configurations and run MPI code
for size in "${sizes[@]}"; do
    # Initialize variables for execution times and speedups
    t1=""
    t2=""
    t4=""
    t8=""
    s2=""
    s4=""
    s8=""

    # Run Thrust code and capture the time output for each process
    for np in "${processes[@]}"; do
        export OMP_NUM_THREADS="$np"
        # echo "Running with $OMP_NUM_THREADS threads"
        execution_time=$("$thrust_exec" "$size")

        case "$np" in
            1) t1="$execution_time";;
            2) t2="$execution_time";;
            4) t4="$execution_time";;
            8) t8="$execution_time";;
        esac
    done

    # Calculate speedups
    s2=$(awk "BEGIN { print $t1 / $t2 }")
    s4=$(awk "BEGIN { print $t1 / $t4 }")
    s8=$(awk "BEGIN { print $t1 / $t8 }")

    # Append data to CSV file
    echo "$size,$t1,$t2,$t4,$t8,$s2,$s4,$s8" >> "$output_csv"

    echo "M: $size, T1: $t1, T2: $t2, T4: $t4, T8: $t8, S2: $s2, S4: $s4, S8: $s8"
done

echo "CSV file generated: $output_csv"
