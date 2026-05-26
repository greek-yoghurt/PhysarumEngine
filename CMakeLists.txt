cmake_minimum_required(VERSION 3.10)
project(PhysarumEngine)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(PhysarumEngine
    src/main.cpp
    src/glad.c
)

target_link_libraries(PhysarumEngine
    ${CMAKE_SOURCE_DIR}/lib/libglfw3.a
    opengl32
)
