cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
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
<<<<<<< HEAD
    log.cpp
=======
    threadpool.cpp
>>>>>>> d0503e5938195b7e1318b0e50f4f5b3bc4f286fd
)

set(HEADERS
    ${HEADERS}
    crawler.h
    strexception.h
<<<<<<< HEAD
    log.h
    color.h
    util.h
    tld.h
=======
    threadpool.h
    workset.h
>>>>>>> d0503e5938195b7e1318b0e50f4f5b3bc4f286fd
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_LINK_FLAGS "-L.")

add_executable(robot_farmer ${SOURCE} ${HEADERS})
target_link_libraries(robot_farmer curl pthread)
target_compile_features(robot_farmer PRIVATE cxx_range_for)
