cmake_minimum_required(VERSION 3.14)
project(hypergraphs)

set(CMAKE_CXX_STANDARD 14)

add_executable(hypergraphs main.cpp hypergraph.cpp partitions.cpp partitions.h gainContainer.cpp gainContainer.h fiducciaMatteyses.cpp fiducciaMatteyses.h)