#! /usr/bin/bash

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
echo "$parent_path"
cd "$parent_path"
cd .. || exit
find src -name *.cc -o -name *.h -o -name *.hpp -o -name *.cpp | xargs -I FILE clang-format -n -Werror FILE

find tests -name *.cc -o -name *.h -o -name *.hpp -o -name *.cpp | xargs -I FILE clang-format -n -Werror FILE

find include -name *.cc -o -name *.h -o -name *.hpp -o -name *.cpp | xargs -I FILE clang-format -n -Werror FILE
