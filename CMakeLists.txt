cmake_minimum_required(VERSION 3.20)

project(libemd)

#enable_language(C CPP)

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

find_package(fmt)

add_executable(ngp-libgen
  src/ngp-libgen/sha1.c
  src/ngp-libgen/emd-parse.cpp
  src/ngp-libgen/main.cpp
)

add_executable(vita-emd-to-yaml
  src/vita-emd-to-yaml/yamlemitter.c
  src/vita-emd-to-yaml/sha1.c
  src/vita-emd-to-yaml/vita-import.c
  src/vita-emd-to-yaml/vita-import-emd-parse.c
  src/vita-emd-to-yaml/vita-emd-to-yaml.c
)

target_include_directories(dumpemd PRIVATE "src/")
target_link_libraries(dumpemd emd)

target_include_directories(emitter PRIVATE "src/")
target_link_libraries(emitter emd)

target_include_directories(ngp-libgen PRIVATE "src/")
target_link_libraries(ngp-libgen emd fmt::fmt)

target_include_directories(vita-emd-to-yaml PRIVATE "src/")
target_link_libraries(vita-emd-to-yaml emd yaml)