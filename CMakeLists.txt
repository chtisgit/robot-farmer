cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(robot_farmer CXX)

list(APPEND
    CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/cmake"
)

set(SOURCE
    ${SOURCE}
    main.cpp
    crawler.cpp
    strexception.cpp
    log.cpp
    threadpool.cpp
    curlprovider.cpp
    workset.cpp
)

set(HEADERS
    ${HEADERS}
    crawler.h
    strexception.h
    log.h
    color.h
    util.h
    tld.h
    threadpool.h
    workset.h
    curlprovider.h
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -g -O2")
set(CMAKE_LINK_FLAGS "-L.")

add_executable(robot_farmer ${SOURCE} ${HEADERS})
target_link_libraries(robot_farmer curl pthread)
