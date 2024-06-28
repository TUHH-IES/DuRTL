run_experiments.sh

creates a build folder and calls cmake and make

runs the executable "exp_analysis" twice
    with 
        "[ift_experiments_full_res]"
    and
        "[ift_experiments_rand_res]"
    from the file "src/experiments_analsysis.cpp"

To add another design to the experiments
    create new testcases for full resolution and random resolution in "src/experiments_analysis.cpp"

the experiments store the analysis results in "build/src/results/"

after all experiments are done the script latex_table.py goes through the result files in "build/src/results/" and creates 3 latex tables
in the file latex_tables.tex.