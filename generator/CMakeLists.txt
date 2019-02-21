cmake_minimum_required(VERSION 3.5)

# Project Name
PROJECT(generate)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_executable(${PROJECT_NAME} main.cpp generators.cpp)
