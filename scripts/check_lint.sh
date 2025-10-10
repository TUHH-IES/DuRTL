#! /usr/bin/bash

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd "$parent_path"

if ! [ -d ../build_clang/ ]; then
  mkdir ../build_clang
fi

cd ../build_clang || exit

CC=clang CXX=clang++ cmake -DENABLE_COVERAGE=TRUE -G Ninja ..
sed -i -e 's/@.*modmap//g' compile_commands.json
python3 ../scripts/compile_commands.py compile_commands.json
run-clang-tidy -header-filter=.* > clang_tidy_report.log
