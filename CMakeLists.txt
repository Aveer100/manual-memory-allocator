cmake_minimum_required(VERSION 3.14)
project(
  Manual-Memory-Allocator
  VERSION 1.0
  DESCRIPTION "Memory allocation and deallocation function implementations"
  LANGUAGES C CXX)

file(GLOB SOURCES "src/*.c")

add_compile_options(-fsanitize=address,undefined -g -fno-omit-frame-pointer)
add_link_options(-fsanitize=address,undefined)

add_executable(testCases ${SOURCES} tests/allocator_test.c)
include_directories(include)
enable_testing()
add_test(NAME AllocatorUnitTests COMMAND testCases)


