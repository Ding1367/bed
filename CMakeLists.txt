cmake_minimum_required(VERSION 3.10)
project(bed VERSION 0.0.1 LANGUAGES C)

add_executable(bed
    src/editor.c src/editor.h
    src/ui.c     src/ui.h
    src/main.c
)
target_compile_options(bed PRIVATE -Wall -Wextra -Werror)
set_target_properties(bed PROPERTIES
    C_STANDARD 11
)
