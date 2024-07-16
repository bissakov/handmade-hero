# Handmade Hero

This repository contains the source code for the [Handmade Hero project](https://handmadehero.org/). See [license](https://github.com/bissakov/handmade-hero/blob/master/LICENSE) for licensing information.

## Prerequisites

Project can be potentially built and/or ran on the older platforms. WIP.

- Windows 10/11 32-bit or 64-bit
- Visual Studio Build Tools 2022
- Visual Studio Community 2022
- Have the `vcvarsall.bat` in your PATH. Possible file paths:
  - `path/to/Microsoft Visual Studio/<version>/Community/VC/Auxiliary/Build/vcvarsall.bat`
  - `path/to/Microsoft Visual Studio/<version>/BuildTools/VC/Auxiliary/Build/vcvarsall.bat`
- Python 3.6+
    - Rich (for colorized output). Install with `pip install rich`
    - cpplint (for code style checking). Install with `pip install cpplint`
    - clang-format (for code formatting). Install with `pip install clang-format`

## Building
```bash
# Clone the repository
git clone https://github.com/bissakov/handmade-hero.git

# Change directory
cd handmade-hero

# Prerequisites
python -m pip install rich cpplint clang-format

# Build using CMake
cmake --build .\build\

# Or using Python for x86 or x64 (default is x64)
python build.py --arch x64

# Or using build.bat script
./build.bat x64

# Run the project
./build/win32-handmade-hero.exe
```
