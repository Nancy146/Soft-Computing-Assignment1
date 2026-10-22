cmake_minimum_required(VERSION 3.16)
project(Genetic_Algorithm_Toolbox LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header-only: the GA core is templated on the chromosome type.
add_library(ga_core INTERFACE)
target_include_directories(ga_core INTERFACE GA_Core)

enable_testing()
add_executable(member1_tests tests/test_member1.cpp)
target_link_libraries(member1_tests PRIVATE ga_core)
if (MSVC)
    target_compile_options(member1_tests PRIVATE /W4)
else()
    target_compile_options(member1_tests PRIVATE -Wall -Wextra -pedantic)
endif()
add_test(NAME member1_tests COMMAND member1_tests)
