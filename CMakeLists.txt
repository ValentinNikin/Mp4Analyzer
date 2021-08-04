cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME Mp4Analyzer)

project(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME} 
    main.cpp
    utils/CliParser.hpp
    utils/CliParser.cpp
    Mp4Analyzer.hpp
    Mp4Analyzer.cpp
)