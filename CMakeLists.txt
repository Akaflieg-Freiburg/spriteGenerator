#
# Set up cmake
#

cmake_minimum_required(VERSION 3.16)


#
# Project data
#

project(spriteGenerator VERSION 1.0.0)


#
# Installation
#

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# Qt Setup
#

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


#
# External Packages
#

find_package(Qt6 COMPONENTS Core Svg REQUIRED)


#
# Add executable and libraries
#

qt_add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Svg)
