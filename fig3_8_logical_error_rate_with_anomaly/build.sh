# Copyright 2022 NTT CORPORATION

mkdir build
cd build
cmake -G "Unix Makefiles" ..
cd ../

cmake --build ./build --config Release
