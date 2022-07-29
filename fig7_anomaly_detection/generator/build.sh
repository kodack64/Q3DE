# Copyright 2022 NTT CORPORATION

mkdir ./build
cd build
cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=Release ..
cd ../

cmake --build ./build --target all --config Release
