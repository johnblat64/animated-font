cmake_minimum_required(VERSION 3.23)
project(animated_font_test)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_BUILD_RPATH_USE_ORIGIN  TRUE)

add_executable(animated_font_test main.cpp external/imgui/imgui.cpp external/imgui/imgui_demo.cpp external/imgui/imgui_draw.cpp external/imgui/imgui_tables.cpp external/imgui/imgui_widgets.cpp external/imgui/backends/imgui_impl_sdl.cpp  external/imgui/backends/imgui_impl_sdlrenderer.cpp external/imgui/misc/cpp/imgui_stdlib.cpp)

set_property(
        TARGET ${PROJECT_NAME}
        PROPERTY BUILD_RPATH $ORIGIN/external/SDL2;
)

include_directories(
        PRIVATE external
        PRIVATE external/imgui
        PRIVATE ${PROJECT_SOURCE_DIR}
)

target_link_directories(
        ${PROJECT_NAME}
        PRIVATE external/SDL2/lib
)

add_subdirectory(external/SDL2)


target_link_libraries(animated_font_test SDL2 SDL2main)

