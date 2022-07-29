# Copyright 2022 NTT CORPORATION

cmake_minimum_required(VERSION 3.10)

project(surface_code_3d LANGUAGES CXX)

##### set warnings #####
set(WARNING_CPP "-Wall -Wextra -Werror=undef -Wlogical-op -Wmissing-include-dirs \
 -Wpointer-arith -Winit-self -Wfloat-equal -Wsuggest-attribute=noreturn \
 -Werror=missing-prototypes -Werror=implicit-function-declaration -Werror=missing-declarations -Werror=return-type \
 -Werror=incompatible-pointer-types -Werror=format=2 -Wredundant-decls -Wmissing-noreturn \
 -Wimplicit-fallthrough=5 -Wshadow -Wendif-labels -Wstrict-aliasing=2 -Wwrite-strings -Werror=overflow -Wdate-time \
 -fdiagnostics-color=auto")


##### set output directory of release build #####
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
if(MSVC)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../lib)
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../bin)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../bin)
endif()


##### set flags for compile #####
if(MSVC)
	# ignore warning about template export and non-Unicode files
	add_compile_options(/wd4251)
	add_compile_options(/wd4819)
	# static link to multithread runtimes
	set(variables 
		CMAKE_CXX_FLAGS_DEBUG
		CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_RELWITHDEBINFO
		CMAKE_CXX_FLAGS_MINSIZEREL)
	foreach(variable ${variables})
		if(${variable} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
		endif()
	endforeach()
else()
	# set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
	# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mtune=native -march=native -mfpmath=both")
endif()

##### show configurations #####
message(STATUS "CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE}")


##### add directories #####
add_subdirectory(src)

