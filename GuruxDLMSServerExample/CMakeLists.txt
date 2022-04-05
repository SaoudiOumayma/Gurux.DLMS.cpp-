cmake_minimum_required (VERSION 3.8)

project(DlmsServerExample)

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/getopt.c
        src/GXDLMSBase.cpp)

target_link_libraries(${PROJECT_NAME} sub::dlms pthread)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} sub::dlms)
endif ()

target_include_directories( ${PROJECT_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
    ${dlms_SOURCE_DIR}/include
)
