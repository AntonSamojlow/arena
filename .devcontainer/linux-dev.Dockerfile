FROM mcr.microsoft.com/powershell:lts-ubuntu-22.04
ARG UBUNTU_CODE_NAME=jammy

# shortcut shell to pwsh calling 'apt-get' with some flags (in case of security warnings, consider adding '--allow-unauthenticated')
SHELL ["/usr/bin/pwsh", "-Command", "$ErrorActionPreference = 'Stop';", "apt-get", "--yes", "--fix-missing"]
ARG DEBIAN_FRONTEND=noninteractive

# --- Install Dev Tools from default apt repo ---

RUN update
RUN install git
# general build tooling
RUN install ninja-build gdb curl zip pkg-config
# for Sonar (java runtime)
RUN install default-jre
# for Nuget access of vcpkg binary caching
RUN install mono-complete
# for gcov style coverage
RUN install gcovr
# for installing from other apt repos
RUN install gpg wget software-properties-common

# --- Install *latest* version of CMake from https://apt.kitware.com/ ---

RUN ["/bin/sh", "-c", "wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg > /dev/null"]
RUN ["/bin/sh", "-c", "echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list > /dev/null"]
RUN update
RUN ["/usr/bin/pwsh", "-Command", "$ErrorActionPreference = 'Stop';" ,"rm", "/usr/share/keyrings/kitware-archive-keyring.gpg"]
RUN install kitware-archive-keyring
RUN install cmake
RUN ["/usr/bin/pwsh", "-Command", "$ErrorActionPreference = 'Stop';" ,"cmake", "--version"]

# --- Install *specified* version of gcc from https://packages.ubuntu.com ---
ARG GCC_VERSION=12
RUN install "g++-$env:GCC_VERSION" "gcc-$env:GCC_VERSION"
# reset shell
SHELL ["/usr/bin/pwsh", "-Command", "$ErrorActionPreference = 'Stop';"]
# add new symlinks for the installed versions (vcpkg default triples use g++)
RUN update-alternatives --install /usr/bin/gcc gcc "/usr/bin/gcc-$env:GCC_VERSION" 20
RUN update-alternatives --install /usr/bin/g++ g++ "/usr/bin/g++-$env:GCC_VERSION" 20

# --- Install *specified* version of llvm, clang, etc. from https://apt.llvm.org/ ---
ARG CLANG_VERSION=15
RUN $v = $env:CLANG_VERSION; \
  $n = $env:UBUNTU_CODE_NAME; \
  wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -; \
  add-apt-repository "deb http://apt.llvm.org/$n/ llvm-toolchain-$n-$v main"; \
  apt-get update; \
  apt-get --yes --fix-missing install llvm-$v clang-$v clang-tidy-$v lldb-$v lld-$v libclang-$v-dev;
# set symlink only for clang-tidy (clang-tidy cmake script uses 'default' version)
RUN update-alternatives --install /usr/bin/clang-tidy clang-tidy "/usr/bin/clang-tidy-$env:CLANG_VERSION" 20
RUN Get-Command "clang-$env:CLANG_VERSION" | Write-Host
