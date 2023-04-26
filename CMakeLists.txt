cmake_minimum_required(VERSION 3.20)

project(libemd)

enable_language(C)

add_library(emd STATIC
  src/emdlib/freadwrite_safe.c
  src/emdlib/cpp_token_read.c
  src/emdlib/emd_token_read.c
  src/emdlib/emd_token_write.c
  src/emdlib/scanemddef.c
)

add_executable(dumpemd
  src/example/example_parser.c
  src/example/main.c
)

add_executable(emitter
  src/emitter/main.c
)

add_executable(vita-libs-gen-emd
  src/vita-libs-gen-emd/sha1.c
  src/vita-libs-gen-emd/vita-import.c
  src/vita-libs-gen-emd/vita-import-emd-parse.c
  src/vita-libs-gen-emd/vita-libs-gen-emd.c
)

target_include_directories(dumpemd PRIVATE "src/")
target_link_libraries(dumpemd emd)

target_include_directories(emitter PRIVATE "src/")
target_link_libraries(emitter emd)

target_include_directories(vita-libs-gen-emd PRIVATE "src/")
target_link_libraries(vita-libs-gen-emd emd)