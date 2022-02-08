cmake_minimum_required(VERSION 3.9)
project(timer_event_generator)

set(CMAKE_CXX_STANDARD 11)
find_package (Threads)

add_executable(exe main.cpp eventGenerator.cpp eventGenerator.hpp itimer.hpp event.hpp)
target_link_libraries (exe ${CMAKE_THREAD_LIBS_INIT})