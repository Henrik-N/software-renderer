cmake_minimum_required(VERSION 3.28)
project(renderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR}, src/public)


add_executable(renderer
        src/main.cpp
        src/_application.cpp
        src/_math.cpp
        src/_mesh.cpp
        src/_time.cpp
        src/_window.cpp
        src/public/_application.h
        src/public/_color.h
        src/public/_common.h
        src/public/_math.h
        src/public/_mesh.h
        src/public/_time.h
        src/public/_window.h
)



target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
