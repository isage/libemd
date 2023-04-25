cmake_minimum_required(VERSION 3.20)

project(libemd)

enable_language(C)

add_library(emd STATIC
  src/emdlib/freadwrite_safe.c
  src/emdlib/cpp_token_read.c
  src/emdlib/emd_token_read.c
  src/emdlib/scanemddef.c
)

add_executable(dumpemd
  src/example/example_parser.c
  src/example/main.c
)

target_include_directories(dumpemd PRIVATE "src/")
target_link_libraries(dumpemd emd)