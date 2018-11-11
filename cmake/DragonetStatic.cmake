include_directories(${CMAKE_CURRENT_LIST_DIR}/../inc)
add_library(dragonet STATIC ${CMAKE_CURRENT_LIST_DIR}/../src/dragonet_freertos.cc)
