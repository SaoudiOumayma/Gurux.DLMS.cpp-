cmake_minimum_required (VERSION 3.14)

project(dlms)

file(GLOB SOURCES src/**.cpp)
add_library(${PROJECT_NAME} ${SOURCES})

add_library(sub::dlms ALIAS ${PROJECT_NAME})

target_include_directories( ${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
