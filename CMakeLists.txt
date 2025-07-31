cmake_minimum_required(VERSION 3.22)
 
project(interval_map LANGUAGES CXX)

set(SRC
	interval_map.hpp
	main.cpp)

add_executable(${PROJECT_NAME} ${SRC})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
