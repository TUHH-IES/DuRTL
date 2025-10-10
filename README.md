
## Build
### Part 1
Install the project dependencies.
#### Linux
- Install the project dependencies by executing the following command:
  - **Debian/Ubuntu**: `sudo apt install git g++ cmake bison flex curl zip unzip tar pkg-config iverilog yosys`

#### Windows
- Install the project dependencies by downloading the following tools:
  - Download the latest release of the OSS CAD Suite for Windows from: https://github.com/YosysHQ/oss-cad-suite-build/releases/latest and extract the content of the downloaded archive in `thirdparty/oss-cad-suite`
  - Install MSVC by installing Visual Studio Community Edition - https://visualstudio.microsoft.com - select the installer 'Desktop Development with C++'

### Part 3
Clone the project repository:

- `git clone https://github.com/TUHH-IES/DuRTL.git`

### Part 3
Compile the project by executing the following commands:

- `git clone https://github.com/microsoft/vcpkg.git thirdparty/vcpkg`
- `mkdir build`
- `cd build`
- `cmake ..`

## Usage

- Run all tests using the `tester` executable.

- The following two flows are for creating your own TESTCASE as an execution environment or a new executable:
1. **IFT Flow**
   1. Necessary inputs include:
      * Verilog design file
      * Corresponding design testbench file
      * Json netlist for the design

   2. Simulate the design to verify its functionality

   3. Generate JSON for the design using YOSYS(more about the commands in the documentation- https://yosyshq.readthedocs.io/projects/yosys/en/latest/):
      ``` 
      DESIGN= design.v
      JSON=`basename ${DESIGN} .v`.json
      echo "
        read_verilog -sv ${DESIGN} #read modules from Verilog file
        hierarchy -auto-top #check, expand and clean up design hierarchy along with automatically determing the top module
        proc #translate processes to netlists
        memory_collect #translate memories to basic cells
        opt #perform simple optimizations
        write_json ${JSON} #write design to a JSON file" \
        | yosys
      ``` 

   4. IFT for the design:
      * Use a `nlohmann::json` object to set general parameters for IFT
        * `tag_size` - size of the tag_vector in bits (default: 32, maximum(tested): 2048)
        * `ift` - enable IFT (default: true)
      * Generate design object by parsing JSON file : 
        * Use the `Design` class from `design.hpp` to create a design object
        * function `Design::parse_json` to parse the JSON file and populate the design object
      * Generate VCD data for tag generation
        * Use the `ducode::simulate_design` function to simulate the design and generate simulation data (VCD format)
        * define a stepsize for the simulation data (e.g. 1000 steps)

          alternatively, you can use `ducode::get_stepsize` to get the stepsize of the design + testbench
        * Use `ducode::get_timesteps_per_simulation_run` to get the number of timesteps per simulation run
        * Use `ducode::get_number_of_simulation_runs` to get the number of simulation runs
        * Create a `TagGenerator` object (full-resolution or random) to generate tags for signals (you can specify which signals to tag by passing a list of signal names; however only primary inputs can be tagged right now)
      * Generate IFT design and IFT testbench
        * Use `Design::export_design` to export the design 

          Parameter `ift` decides if the design is exported with IFT
        * Create a `Testbench` object for the design
          * Use `Testbench::export_testbench` to export the testbench
            * Parameter `ift` decides if the testbench is exported with IFT
      * Use `ducode::simulate_design` with the exported IFT design and testbench to generate VCD data including the IFT data
      * Create a `SignalTagTracker` object (VCDSignalTagTracker for simulation-based IFT) that stores the VCD data internally
      * Create a `DesignInstance` object
        * contains a flattened graph of the design
        * contains analysis functions to analyze the IFT data on the design
      
      * Refer to the existing TESTCASE - `hierarchy_ift_flow` in `tests/ift/ift_flow.cpp`, as a template for the above steps

   5. All temp generated data understanding
      * `ift_design.v` - Verilog file of the IFT design 
      * `ift_design_tb.v` - Verilog file of the IFT testbench
      * `output.vcd` - VCD file generated from the simulation of the IFT design and IFT testbench

2. **SMT Flow -- not thoroughly tested, yet, and known to have bugs** 
   1. Necessary inputs include:
      1. Verilog design file
      2. Json netlist for the design
   2. Create the necessary Z3 solver objects
      1. Use `z3::solver` to create a Z3 solver object
      2. Use `z3::context` to create a Z3 context object
      3. Use `z3::expr_vector` to create a vector of Z3 expressions for the signals in the design
      4. Create a `std::unordered_map<std::string, z3::expr>` to map signal names to Z3 expressions
   3. Parse the design JSON file to create a `Design` object
      1. Use the `Design::parse_json` function to parse the JSON file and populate the design object
   4. Create a `DesignInstance` object
      1. This object contains a flattened graph of the design and analysis functions to analyze the design
   5. Use `DesignInstance::export_smt2` function to create an SMT2 model inside the solver-object from 2.
      1. The signal names are created for every signal(net) of the design
      2. The signal names have the format `|D1_top_module1_signal_a_time_0|` where `D1` is the design ID, `top_module1` is the top module name, `signal_a` is the signal name, and `time_0` is the time step (or unrolling step) of the signal
         1. The function `ducode::create_smt2_signal_name` can be used to create the signal names
         2. Use the unordered_map created in step 2.4 to map to access the corresponding Z3 expressions
      3. For Information Flow Analysis, call `DesignInstance::export_smt2_ift` a second time with another design ID e.g. `D2` to create the IFT model
         1. The signal names for the IFT model will have the format `|D2_top_module1_signal_a_time_0|`
   6. Use `DesignInstance` object to call an analysis function
   7. OR add your own z3 constraints and call `solver.check()` to check the satisfiability of the constraints on the design (see 5.2.2 for accessing signal values)

- **Limitations SMT**
  - design must have a single clock domain
  - only positive edge triggered flipflops are supported
    - the current method of unrolling the design for SMT assumes the above to be true which causes the next state of a flipflop to be updated at every time step
    - from this follows that only synchronous designs are supported
  - so: sequential elements with asynchronous set/reset are not supported currently!

   - there are simple checks implemented one can use to check if the design meets the above limitations
     - `DesignInstance::check_clock_signal` checks if all flipflops in the design use the same clock signal
       - currently limited to designs with a single module only (i.e. no module instantiations)
         - this can be extended in the future if needed
     - `DesignInstance::check_clock_edge` checks if all flipflops in the design are positive edge triggered
     - both functions return a boolean value
     - these functions traverse all flipflops through the flattened instance graph of the design

     - `Module::check_clock_signal_consistency` checks if all flipflops in a module use the same clock signal
       - this function traverses all flipflops through the module graph of the module
     - `Module::check_clock_rising_edge_consistency` checks if all flipflops in a module are positive edge triggered
       - this function traverses all flipflops through the module graph of the module
**Limitations SMT End**

- general: 
   - signals are uniquely identified by their name and the module hierarchy they belong to.


- Run the `main` executable to execute the basic IFT analysis on a design.
  *  Run `main -h` to get more information about the command options available.


## Documentation

- Ubuntu: run `doyxgen doc/doxyfile` from the main directory
- Documentation replicating the experiments/feature tests performed with DuRTL for research work in /doc/useCases
- Supporting documentation related to Build, Release and Software Management plan included in /doc/strategy

## Contributing

Check the formatting of the code base:
 - scripts/format_check.sh
Check the linter errors:
 - scripts/linter_script.sh
