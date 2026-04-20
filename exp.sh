#!/usr/bin/env bash

current_directory=$(pwd)
graphs_dir=./Indigo3Suite/graphGen/generatedGraphs/apg

if [ ! -d "$graphs_dir" ]; then
  echo "Error: directory $graphs_dir not found"
  exit 1
fi

filename_timestamp=$(date +'%m-%d-%Y_%H-%M-%S').out
echo "Timestamp for filename: $filename_timestamp"

for file_path in "$graphs_dir"/*; do
  if [ -f "$file_path" ]; then
    echo "Processing file: $file_path"
    ./cmake-build-debug-wsl/Karger "$file_path" >> "$filename_timestamp" 2>&1
  fi
done