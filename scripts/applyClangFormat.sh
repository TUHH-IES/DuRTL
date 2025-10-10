#!/bin/bash
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
echo "$parent_path"
cd "$parent_path"

F=`./check_format.sh 2>&1 | grep error | cut -d ":" -f 1 | sort | uniq`
cd "$parent_path"
cd ..
for D in $F; do 
  echo "Applying clang-format to $D"
  #echo "Current dir: "
  #pwd
  BASE=`basename $D`
  clang-format "$D" >".tmp.$BASE"
  #echo "Diff:"
  #diff "$D" ".tmp.$BASE"
  mv ".tmp.$BASE" "$D"
done
