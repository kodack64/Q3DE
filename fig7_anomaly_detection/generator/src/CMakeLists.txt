# Copyright 2022 NTT CORPORATION

cmake_minimum_required(VERSION 3.10)

file(GLOB SIM_SRC "*.hpp" "*.h")
file(GLOB BLOSSOM "./blossom5/*.cpp" "./blossom5/*.h" "./blossom5/MinCost/*.cpp" "./blossom5/MinCost/*.h")
list(FILTER BLOSSOM EXCLUDE REGEX ".*example.cpp$")

add_executable(surface_code_3d_anomaly ${SIM_SRC} ${BLOSSOM} main_anomaly.cpp)
add_executable(surface_code_3d_anomaly_long ${SIM_SRC} ${BLOSSOM} main_anomaly_long.cpp)
source_group("blossom" FILES ${BLOSSOM})
