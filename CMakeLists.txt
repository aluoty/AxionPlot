cmake_minimum_required(VERSION 3.15)
project(AxionPlot C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(EMSCRIPTEN)
    set(PLATFORM_WEB TRUE)
endif()

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib)

add_executable(AxionPlot
    src/main.c
    src/camera.c
    src/expr.c
    src/graph.c
    src/ui.c
    lib/tinyexpr.c
)

target_include_directories(AxionPlot PRIVATE src lib)
target_link_libraries(AxionPlot PRIVATE raylib)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set_target_properties(AxionPlot PROPERTIES SUFFIX ".html")

    target_link_options(AxionPlot PRIVATE
        -sUSE_GLFW=3
        -sALLOW_MEMORY_GROWTH=1
        -sASSERTIONS=1
        -sWASM=1
        -sASYNCIFY
        -sGL_ENABLE_GET_PROC_ADDRESS=1
        -sEXPORTED_RUNTIME_METHODS=cwrap,direct_call
        --shell-file "${CMAKE_SOURCE_DIR}/web/shell.html"
    )
endif()
