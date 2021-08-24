this is my final setup for vulkan.

this build is tested on WINDOWS only!
but should work on linux as well.

## What you need :
make sure that vulkan sdk is installed and that there is enviroment path for it. ( should be if you marked "add path to enviroment" )
make sure that CMake is installed.

## How to use:
### step 1:
clone repository
### step 2: Option 1: (using the bash files)
> sh configure.sh
> 
> sh build.sh
>
> sh run.sh

( or the linux way )

### Option 2: (Compiling the cmake manually)
> cmake -S . -B out/build/ -G
> 
> cd out/build
>
> make
>
> ./pragma.exe

#### Done

### Setup: (C hange project name etc... )

in CMakeLists.txt, change project name to watever ( switch watever to the name you choose ),

then change the name in config.h.in, from pragma to that name. i.e ( pragma_VERSION_MAJOR => watever_VERSION_MAJOR )

in run.sh change pragma_name to watever.

#### Done