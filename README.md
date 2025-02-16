# CGraphicsCore

### Prerequisites
- C++
- Python
- Git

### Clone repository
```shell
git clone --recursive https://github.com/VedantS-32/CGraphicsCore.git
```

### Build Instructions
- Go to "script" folder
- Run CGraphicsSetup script for your platform
- Then run GenerateProject for your platform
- By default it will generate Makefile, please change Script/GenerateProject to generate project files for your IDE
``` shell
vendor\premake\bin\premake5.exe gmake2 #<-- Replace gmake2 with vs2022 for Visual Studio Solution
```
- If you have generated Visual Studio Solution, .sln file will be in root directory
- if you have generated Makefile, open terminal in root directory and enter following command to build CGraphicsCore. After building go to CGraphicsSandbox and launch the binary (CGrpahicsSandbox.exe for Windows).
``` shell
make -j #Number of core you want to allocate for compilation
```
- Additionally you can specify build configuration and compiler
``` shell
make CC= C_Compiler CXX= C++_Compiler config=BuildConfig -j Cores
```
- Example
``` shell
    make CC=gcc CXX=g++ config=release -j 4
```
