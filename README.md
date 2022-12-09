# AoC 2022 solutions in c++

## Build requirements

* c++20 compiler
* cmake >= 2.22
* make or ninja

## How to build

Run: 

* config

```cmake
cmake -S <source folder> -B <your build folder> -DCMAKE_BUILD_TYPE=Release
```
* build

```cmake
cmake --build <your build folder> --parallel <Njobs>
```
