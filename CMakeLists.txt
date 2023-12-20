cmake_minimum_required(VERSION 3.27.4)
project(BreakOut)

find_package(SDL2 REQUIRED)

set(SOURCES
    src/main.c
)

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -O2 -Wall -Wextra"
)

add_executable(BreakOut ${SOURCES})

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)


target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
include_directories(${SDL2_INCLUDE_DIRS})
