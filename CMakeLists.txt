cmake_minimum_required(VERSION 3.14)
project(
  Manual-Memory-Allocator
  VERSION 1.0
  DESCRIPTION "memory allocation functions implementations"
  LANGUAGES C CXX)

file(GLOB SOURCES "src/*.c")

add_executable(main ${SOURCES})
include_directories(include)
