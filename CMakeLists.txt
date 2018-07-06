cmake_minimum_required(VERSION 3.1)
project(dragonet)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_library(${PROJECT_NAME}_linux STATIC src/dragonet_linux.cc)
add_library(${PROJECT_NAME}_freertos STATIC src/dragonet_freertos.cc)
