cmake_minimum_required (VERSION 3.14)

project (DLMSFramework)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_FLAGS -m32)

add_subdirectory (development)
add_subdirectory (GuruxDLMSServerExample)