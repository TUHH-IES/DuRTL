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

### Part 2
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

- Run the `main` executable to execute the basic IFT analysis on a design.
  *  Run `main -h` to get more information about the command options available.

## Correction Notice

We would like to acknowledge an error in our paper regarding CellIFT. Contrary to what we stated, CellIFT is in fact open-source and its design decisions are well-documented. 

The correct information about CellIFT is as follows:
- Source code is available at: https://github.com/comsec-group/cellift-yosys
- Reproduction package is available at: https://github.com/comsec-group/cellift-meta
- Design decisions are explained in the paper (triple artifact-certified): https://www.usenix.org/system/files/sec22-solt.pdf

We apologize for this inaccuracy and thank the CellIFT authors for bringing this to our attention.

## Citation

If you use DuRTL in your research, please cite our paper:

```bibtex
@INPROCEEDINGS{VLSID/SchammerMF25,
  author={Schammer, Lutz and Martino, Gianluca and Fey, Goerschwin},
  booktitle={38th International Conference on VLSI Design and 2024 23rd International Conference on Embedded Systems (VLSID)}, 
  title={DuRTL - Information Flow Analysis Tool for Register Transfer Level Hardware Designs}, 
  year={2025},
  pages={197-202},
  doi={10.1109/VLSID64188.2025.00047}
}
```