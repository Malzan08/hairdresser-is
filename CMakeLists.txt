cmake_minimum_required(VERSION 3.16)
project(hairdresser_app VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
    src/main.cpp
)

add_executable(hairdresser_app ${PROJECT_SOURCES})

target_link_libraries(hairdresser_app PRIVATE
    Qt6::Widgets
    Qt6::Sql
)
