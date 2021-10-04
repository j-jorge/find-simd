cmake_minimum_required(VERSION 3.14)

project(find-benchmark LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(source_root "${CMAKE_CURRENT_LIST_DIR}/src")

set(sources
  "${source_root}/find_int_c.cpp"
  "${source_root}/find_int_c_unrolled_8.cpp"
  "${source_root}/find_int_cpp.cpp"
  "${source_root}/find_int_sse2.cpp"

  "${source_root}/main.cpp"
  )

set(target_name run-benchmark)

find_package(benchmark)

add_executable(${target_name} ${sources})

target_include_directories(
  ${target_name}
  PUBLIC
  "${CMAKE_CURRENT_LIST_DIR}/include"
  )
target_link_libraries(${target_name} benchmark::benchmark)
