cmake_minimum_required(VERSION 3.17)
project(pp_tp2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -O3 ${OpenMP_CXX_FLAGS}")

find_package(OpenMP REQUIRED)
add_executable(pp_tp2 main.cpp ArgParse.cpp ArgParse.hpp MillerRabinSeq.cpp MillerRabinSeq.hpp FileParse.cpp FileParse.hpp)
target_link_libraries(pp_tp2 gmp gmpxx ${OpenMP_CXX_FLAGS})