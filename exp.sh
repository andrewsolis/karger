#!/usr/bin/env bash

current_directory=$(pwd)
graphs_dir="$current_directory/graphGen/generatedGraphs"

if [ ! -d "$graphs_dir" ]; then
  echo "Error: directory $graphs_dir not found"
  exit 1
fi

for file_path in "$graphs_dir"/*; do
  if [ -f "$file_path" ]; then
    echo "Processing file: $file_path"
    cmake-build-debug/Karger "$file_path"
  fi
done