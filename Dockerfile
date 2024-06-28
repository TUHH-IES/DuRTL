FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y build-essential g++-12 git cmake bison flex gcovr iverilog yosys zip unzip curl pkg-config tar ninja-build clang-format-16 clang-tidy-16 clang-16 iwyu
RUN update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-16 10
RUN update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-16 10
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-16 10
RUN update-alternatives --install /usr/bin/run-clang-tidy run-clang-tidy /usr/bin/run-clang-tidy-16 10
RUN mkdir vcpkgconf
WORKDIR vcpkgconf
RUN git clone https://github.com/microsoft/vcpkg.git
COPY vcpkg.json vcpkg.json
COPY vcpkg-configuration.json vcpkg-configuration.json
RUN ./vcpkg/bootstrap-vcpkg.sh
RUN ./vcpkg/vcpkg install
RUN CC=clang CXX=clang++ ./vcpkg/vcpkg install