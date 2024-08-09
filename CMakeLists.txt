cmake_minimum_required(VERSION 3.27)
project(daw)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
)
FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
    FetchContent_Populate(glfw)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(${glfw_SOURCE_DIR})
endif()

FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG v0.1.36
)
FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
    FetchContent_Populate(glad)
    set(GLAD_PROFILE "core" CACHE STRING "" FORCE)
    set(GLAD_API "gl=3.3" CACHE STRING "" FORCE)
    add_subdirectory(${glad_SOURCE_DIR})
endif()

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.90.9-docking
)
FetchContent_Declare(
        imgui_knobs
        GIT_REPOSITORY https://github.com/altschuler/imgui-knobs.git
        GIT_TAG main
)
FetchContent_GetProperties(imgui)
FetchContent_GetProperties(imgui_knobs)
if(NOT imgui_POPULATED OR NOT imgui_knobs_POPULATED)
    FetchContent_Populate(imgui)
    FetchContent_Populate(imgui_knobs)
    add_library(imgui
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
            ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
            ${imgui_knobs_SOURCE_DIR}/imgui-knobs.cpp
            )
    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends ${imgui_SOURCE_DIR}/misc/cpp ${imgui_knobs_SOURCE_DIR})
    target_link_libraries(imgui glfw)
endif()


FetchContent_Declare(
        portaudio
        GIT_REPOSITORY https://github.com/PortAudio/portaudio
        GIT_TAG v19.7.0
)
FetchContent_GetProperties(portaudio)
if(NOT portaudio_POPULATED)
    FetchContent_Populate(portaudio)
    add_subdirectory(${portaudio_SOURCE_DIR})
    include_directories(${portaudio_SOURCE_DIR}/include)
endif()


FetchContent_Declare(
        midifile
        GIT_REPOSITORY https://github.com/craigsapp/midifile
        GIT_TAG master
)
FetchContent_GetProperties(midifile)
if (NOT midifile_POPULATED)
    FetchContent_Populate(midifile)
    add_subdirectory(${midifile_SOURCE_DIR})
    include_directories(${midifile_SOURCE_DIR}/include)
endif()

# file dialog

add_library(tinyfiledialogs STATIC
        lib/tinyfiledialogs/tinyfiledialogs.c
        lib/tinyfiledialogs/tinyfiledialogs.h
        )
target_include_directories(tinyfiledialogs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/tinyfiledialogs)


add_executable(daw main.cpp
        ld/wavload.h
        ld/ldp.h
        ld/filetools.h
        ld/synth.h
        ld/audio.h
)

target_link_libraries(daw glfw glad imgui midifile portaudio tinyfiledialogs)
target_include_directories(daw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/lib/tinyfiledialogs)

