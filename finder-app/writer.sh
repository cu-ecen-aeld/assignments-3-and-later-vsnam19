#!/bin/bash

# - Write a shell script finder-app/writer.sh as described below
# - Accepts the following arguments: the first argument is a full path to a file (including filename) on the filesystem, referred to below as writefile;
# the second argument is a text string which will be written within this file, referred to below as writestr
# - Exits with value 1 error and print statements if any of the arguments above were not specified
# - Creates a new file with name and path writefile with content writestr, overwriting any existing file and creating the path if it doesn’t exist. 
# Exits with value 1 and error print statement if the file could not be created.
check_params_for_empty() {
  for i in "$@"; do
    if [[ -z "$i" ]]; then
      echo "Error: Missing argument(s)." >&2
      exit 1
    fi
  done
}

check_dir_exist() {
  if [[ -d "$1" ]]; then
    echo "Error: '$1' is a valid directory." >&2
    exit 1
  fi
}

if  [ $# -ne 2 ]; then
    echo "Error: Must have exactly 2 parameters"
    exit 1 
fi

writefile="$1"
writestr="$2"

check_params_for_empty "$writefile" "$writestr"

check_dir_exist "$writefile"

target_dir=`dirname "$writefile"`

mkdir -p "$target_dir"

echo "$writestr" > "$writefile"

if [ $? -eq 0 ]; then
    echo "Write content to $writefile success"
else
    echo "Error: Write content to $writefile failed"
    exit 1
fi