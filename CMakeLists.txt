cmake_minimum_required(VERSION 3.20)
project(mandelbrot C)

set(CMAKE_C_STANDARD 11)

include_directories("/home/rhohen/Librerie/CSFML-2.5.1/include")

find_package(SFML 2.5.1 COMPONENTS system window graphics network audio REQUIRED)

add_executable(mandelbrot main.c)
target_link_libraries(mandelbrot csfml-system csfml-window csfml-graphics csfml-audio csfml-network)