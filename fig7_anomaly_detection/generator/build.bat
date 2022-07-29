REM Copyright 2022 NTT CORPORATION

mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" ..
cd ../

cmake --build ./build --target ALL_BUILD --config Release
