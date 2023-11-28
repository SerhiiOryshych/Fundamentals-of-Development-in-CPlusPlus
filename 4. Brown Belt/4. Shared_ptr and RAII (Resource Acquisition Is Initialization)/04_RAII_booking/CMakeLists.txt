cmake_minimum_required(VERSION 3.24)
project(04_RAII_booking)

set(CMAKE_CXX_STANDARD 17)

add_executable(04_RAII_booking main.cpp old_booking_providers.h old_trip_manager.h new_booking_providers.h)
