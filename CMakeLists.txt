cmake_minimum_required(VERSION 3.14)

project(find-benchmark LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(source_root "${CMAKE_CURRENT_LIST_DIR}/src")

add_library(find
  "${source_root}/find_int_c.cpp"
  "${source_root}/find_int_c_unrolled_8.cpp"
  "${source_root}/find_int_cpp.cpp"
  "${source_root}/find_int_sse2.cpp"
  )
target_include_directories(
  find
  PUBLIC
  "${CMAKE_CURRENT_LIST_DIR}/include"
  )

find_package(benchmark)

add_executable(benchmark-last "${source_root}/main-last.cpp")
target_link_libraries(benchmark-last find benchmark::benchmark)

add_executable(benchmark-random "${source_root}/main-random.cpp")
target_link_libraries(benchmark-random find benchmark::benchmark)

add_executable(tests "${source_root}/main-tests.cpp")
target_link_libraries(tests find)
