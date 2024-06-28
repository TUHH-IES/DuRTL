#! /usr/bin/bash

if ! [ -d ../build/ ]; then
  mkdir ../build
fi

cd ../build || exit
cmake ..
make
cd ../scripts || exit

if ! [ -d ../build/src/results ]; then
  mkdir ../build/src/results
fi

cd ../build/src/ || exit

nohup ./exp_analysis [exp_full_res] &
nohup ./exp_analysis [exp_rand_res] &

# nohup ./exp_analysis full_SPI_Master &
# nohup ./exp_analysis full_SPI_Slave &
# nohup ./exp_analysis full_y86 &
# nohup ./exp_analysis full_yadmc &

# nohup ./exp_analysis random_SPI_Master &
# nohup ./exp_analysis random_SPI_Slave &
# nohup ./exp_analysis random_y86 &
# nohup ./exp_analysis random_yadmc &

wait

python3 latex_table.py
