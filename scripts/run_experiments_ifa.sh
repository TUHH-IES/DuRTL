#! /usr/bin/bash

if ! [ -d ../build/ ]; then
  mkdir ../build
fi

cd ../build || exit
cmake -DCMAKE_BUILD_TYPE= Debug ..
make exp_ifa
cd ../scripts || exit

if ! [ -d ../build/src/results ]; then
  mkdir ../build/src/results
fi

cd ../build/src/ || exit

# for n in {1..10}; 
# do
   nohup ./exp_ifa 5 SPI_Master clk &
  #  nohup ./exp_ifa 1 y86 clk &
# done
wait

nohup gprof exp_ifa > prof.output