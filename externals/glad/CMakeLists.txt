# https://github.com/Dav1dde/glad/issues/186
cmake_minimum_required(VERSION 3.0)
project(Glad)

add_library(glad include/glad/glad.h src/glad.c)
target_include_directories(glad PUBLIC include/)
