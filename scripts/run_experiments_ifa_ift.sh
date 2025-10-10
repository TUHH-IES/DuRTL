#! /usr/bin/bash

if ! [ -d ../build_release/ ]; then
  mkdir ../build_release
fi

cd ../build_release || exit
cmake -DCMAKE_BUILD_TYPE= Release ..
make exp_ifa
cd ../scripts || exit

if ! [ -d ../build_release/src/results ]; then
  mkdir ../build_release/src/results
fi

cd ../build_release/src/ || exit

# for n in {1..10}; 
# do
  # nohup ./exp_ifa -m 1 -u 5 -f SPI_Master -d SPI_Master -c clk &
  # nohup ./exp_ifa -m 1 -u 5 -f y86 -d y86 -c clk &

  # nohup ./exp_ifa -m 1 -u 10 -f SPI_Master -d SPI_Master -c clk &
  # nohup ./exp_ifa -m 1 -u 10 -f y86 -d y86 -c clk &

  # nohup ./exp_ifa -m 1 -u 15 -f SPI_Master -d SPI_Master -c clk &
  # nohup ./exp_ifa -m 1 -u 15 -f y86 -d y86 -c clk &

  # nohup ./exp_ifa -m 1 -u 20 -f SPI_Master -d SPI_Master -c clk &
  # nohup ./exp_ifa -m 1 -u 20 -f y86 -d y86 -c clk &

  nohup ./exp_ifa -m 2 -u 1 -f SPI_Master -d SPI_Master -c clk &
  nohup ./exp_ifa -m 2 -u 1 -f y86 -d y86 -c clk &

# done