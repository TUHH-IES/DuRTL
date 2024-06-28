
## Build
### Part 1
#### Linux
* `sudo apt install git g++ cmake bison flex curl zip unzip tar pkg-config`

#### Windows
* Download the latest release of WinFlexBison from: https://github.com/lexxmark/winflexbison/releases/latest and extract the content of the downloaded archive in `thirdparty/win_flex_bison`
* Install git from: https://gitforwindows.org/
* Download the latest release of the OSS CAD Suite for Windows from: https://github.com/YosysHQ/oss-cad-suite-build/releases/latest and extract the content of the downloaded archive in `thirdparty/oss-cad-suite`

### Part 2
* `git clone https://github.com/microsoft/vcpkg.git thirdparty/vcpkg`
* `mkdir build`
* `cd build`
* `cmake ..`
* `make`

## Usage

* Run all tests using the `tester` executable.
* Run the `main` executable to execute the basic IFT analysis on a design.
  * Run `main -h` to get more information about the command options available.
