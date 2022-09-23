cmake_minimum_required(VERSION 3.23)
project(animated_font_test)

set(CMAKE_CXX_STANDARD 14)

add_executable(animated_font_test main.cpp)
target_link_libraries(animated_font_test SDL2 SDL2main)
