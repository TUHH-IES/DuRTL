FROM ubuntu:24.04 AS build

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y build-essential g++-13 git cmake bison flex gcovr iverilog yosys zip unzip curl pkg-config tar ninja-build clang-format-18 clang-tidy-18 clang-18 iwyu
RUN update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-18 10
RUN update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 10
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 10
RUN update-alternatives --install /usr/bin/run-clang-tidy run-clang-tidy /usr/bin/run-clang-tidy-18 10
RUN git clone https://github.com/microsoft/vcpkg.git
RUN ./vcpkg/bootstrap-vcpkg.sh

FROM build
COPY vcpkg.json vcpkg.json
COPY vcpkg-configuration.json vcpkg-configuration.json
RUN ./vcpkg/vcpkg install
RUN CC=clang CXX=clang++ ./vcpkg/vcpkg install