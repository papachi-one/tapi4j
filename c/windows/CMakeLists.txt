cmake_minimum_required(VERSION 3.17)
project(tapi4j C)
set(CMAKE_C_STANDARD 99)
add_library(tapi4j SHARED one_papachi_tapi4j_TapI4j.c)
include_directories("$ENV{JAVA_HOME}/include")
include_directories("$ENV{JAVA_HOME}/include/win32")
