cmake_minimum_required(VERSION 3.15)
project(AxionPlot C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(EMSCRIPTEN)
    set(PLATFORM "Web" CACHE STRING "" FORCE)
    set(GRAPHICS "OPENGL_ES_2" CACHE STRING "" FORCE)
endif()

set(AXION_WINDOW_WIDTH 1920 CACHE STRING "Default window width")
set(AXION_WINDOW_HEIGHT 1080 CACHE STRING "Default window height")
set(AXION_RENDER_SCALE 2 CACHE STRING "Default render supersampling (1-3)")

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
    src/display.c
    src/expr.c
    src/graph.c
    src/ui.c
    lib/tinyexpr.c
)

target_include_directories(AxionPlot PRIVATE src lib)
target_link_libraries(AxionPlot PRIVATE raylib)

target_compile_definitions(AxionPlot PRIVATE
    AXION_WINDOW_WIDTH=${AXION_WINDOW_WIDTH}
    AXION_WINDOW_HEIGHT=${AXION_WINDOW_HEIGHT}
    AXION_RENDER_SCALE=${AXION_RENDER_SCALE}
)

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
        -sMIN_WEBGL_VERSION=2
        --shell-file "${CMAKE_SOURCE_DIR}/web/shell.html"
    )
endif()
