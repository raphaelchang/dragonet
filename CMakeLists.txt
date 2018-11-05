cmake_minimum_required(VERSION 3.1)
project(dragonet)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(lcm REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_library(${PROJECT_NAME} SHARED src/dragonet_linux.cc)
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/inc DESTINATION include FILES_MATCHING PATTERN "*.h*")
#add_library(${PROJECT_NAME} STATIC src/dragonet_freertos.cc)
