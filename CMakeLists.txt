cmake_minimum_required(VERSION 3.1)
project(dragonet)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(lcm REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_library(${PROJECT_NAME} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/dragonet_linux.cc)

target_link_libraries(${PROJECT_NAME} lcm)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/inc/dragonet.h)

add_executable(${PROJECT_NAME}_echo ${CMAKE_CURRENT_SOURCE_DIR}/tools/dragonet_echo/dragonet_echo.cc)
target_link_libraries(${PROJECT_NAME}_echo dragonet)

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_echo
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
    RUNTIME DESTINATION bin)

export(PACKAGE ${PROJECT_NAME})
