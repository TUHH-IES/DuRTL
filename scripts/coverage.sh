#!/bin/bash
rm -r build
mkdir build
(cd build; 
cmake -DENABLE_COVERAGE=TRUE ..
make
(cd tests; ./tester)
gcovr --html-details covreport.html -r ..  --filter ../src --exclude ../src/main.cc .
firefox covreport.html
)
