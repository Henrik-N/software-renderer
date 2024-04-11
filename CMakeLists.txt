cmake_minimum_required(VERSION 3.22.1)
project(renderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # enable compile_commands.json export - clangd lsp can use this

find_package(SDL2 REQUIRED)

file(GLOB_RECURSE MY_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)


add_executable(${PROJECT_NAME} ${MY_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIR}, src/public)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})

