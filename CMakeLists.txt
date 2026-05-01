cmake_minimum_required(VERSION 3.12.0)
project(SDL3Project)

option(DOWNLOAD_DEPENDENCIES "Download dependencies" TRUE)

if(DOWNLOAD_DEPENDENCIES)
    include(FetchContent)
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
        GIT_TAG "main"
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(SDL3)
else()
    find_package(SDL3 CONFIG REQUIRED)
endif()

if(ANDROID)
    function(add_executable TARGET)
        add_library(${TARGET} SHARED ${ARGN})
    endfunction()
endif()


file(GLOB SOURCES "src/source/*.cpp")
add_executable(sdl3project
    src/main_gl.cpp
    src/source/renderer.cpp
    src/source/grid.cpp
)


target_include_directories(sdl3project PRIVATE 
    include 
    src
    src/include
)


find_library(LOG_LIB log)

target_link_libraries(sdl3project PRIVATE 
    GLESv2 
    SDL3::SDL3 
    ${LOG_LIB}
)

set_property(TARGET sdl3project PROPERTY WIN32_EXECUTABLE TRUE)

if(EMSCRIPTEN)
    set_property(TARGET sdl3project PROPERTY SUFFIX ".html")
endif()