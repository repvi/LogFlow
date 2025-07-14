#universnal
set(SOURCES
    "src/logger.c" 
    "src/genList.c"
)

if(DEFINED IDF_TARGET)
    idf_component_register(
        SRCS ${SOURCES}
        INCLUDE_DIRS "include"
        internal_include "internal_include"
    )
    
else()
    cmake_minimum_required(VERSION 3.10)
    
    set(CMAKE_C_COMPILER "gcc")
    set(CMAKE_CXX_COMPILER "g++")

    project(Logger C)

    set(CMAKE_C_STANDARD 11)

    # Create static logger library
    add_library(logger STATIC ${SOURCES})

    target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_include_directories(logger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/internal_include)

    # Create executable to run logger tests
    add_executable(LoggerTest test/example.c)
    target_link_libraries(LoggerTest logger)
    target_include_directories(LoggerTest PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/internal_include
    )
endif()