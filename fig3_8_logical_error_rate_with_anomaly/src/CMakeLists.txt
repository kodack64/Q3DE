# Copyright 2022 NTT CORPORATION

cmake_minimum_required(VERSION 3.10)

file(GLOB SIM_SRC "*.hpp" "*.h" "*.cpp")
file(GLOB BLOSSOM "./blossom5/*.cpp" "./blossom5/*.h" "./blossom5/MinCost/*.cpp" "./blossom5/MinCost/*.h")
list(FILTER BLOSSOM EXCLUDE REGEX ".*example.cpp$")

add_executable(main ${SIM_SRC} ${BLOSSOM})
source_group("blossom" FILES ${BLOSSOM})
