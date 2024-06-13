cmake_minimum_required(VERSION 3.22.1)
project(renderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # enable compile_commands.json export - clangd lsp can use this
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")


set(PLATFORM_LINUX   (${CMAKE_SYSTEM_NAME} STREQUAL "Linux"))
set(PLATFORM_WINDOWS (${CMAKE_SYSTEM_NAME} STREQUAL "Windows"))

if (PLATFORM_LINUX)
    add_compile_definitions(PLATFORM_LINUX=1)
    add_compile_definitions(COLORED_LOG=1)
endif()


# add executable
set(PRECOMPILED_HEADER_FILES ${CMAKE_SOURCE_DIR}/src/public/_common.h)
file(GLOB_RECURSE MY_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${MY_SOURCES})


find_package(SDL2 REQUIRED)

target_precompile_headers(${PROJECT_NAME} PRIVATE
    ${PRECOMPILED_HEADER_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${SDL2_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src/public)

target_link_libraries(${PROJECT_NAME} 
    ${SDL2_LIBRARY}
    ${PLATFORM_LIB}
)
