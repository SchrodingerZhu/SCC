cmake_minimum_required(VERSION 3.3)
project(simplified_c)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3 -g -flto")
cmake_policy(PUSH)
cmake_policy(SET CMP0076 OLD)
add_subdirectory(frontend)
add_subdirectory(backend)
cmake_policy(POP)

include_directories(frontend/include backend/include backend/parallel-hashmap/parallel_hashmap)

add_executable(scc main.cpp codegen.cpp codegen.h)
target_link_libraries(scc parser vcfg)

enable_testing()
add_executable(simple_test tests/simple_test.cpp codegen.cpp)
target_link_libraries(simple_test parser vcfg)
