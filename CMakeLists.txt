cmake_minimum_required(VERSION 3.10)
project(PhysarumEngine)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(PhysarumEngine
    src/main.cpp
    src/glad.c
)

if(WIN32)
    target_link_libraries(PhysarumEngine
        ${CMAKE_SOURCE_DIR}/lib/windows/libglfw3.a
        opengl32
    )
elseif(APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(PhysarumEngine
        ${CMAKE_SOURCE_DIR}/lib/mac/libglfw3.a
        OpenGL::GL
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()