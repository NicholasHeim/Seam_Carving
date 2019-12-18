cmake_minimum_required(VERSION 3.10.2)
project(project4)
set(CMAKE_CXX_STANDARD 11)

file(GLOB IMAGES
  "*.pgm"
)
file(COPY ${IMAGES} DESTINATION "./")

set(SOURCE_FILES main.cpp)

add_executable(carve ${SOURCE_FILES})