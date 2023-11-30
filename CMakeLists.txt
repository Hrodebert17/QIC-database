cmake_minimum_required(VERSION 3.26)
project(Qic_database)

set(CMAKE_CXX_STANDARD 23)

add_library(Qic_database
        Qic_database.cpp)

# Set the output name to Qic_database.lib
set_target_properties(Qic_database PROPERTIES OUTPUT_NAME Qic_database)
