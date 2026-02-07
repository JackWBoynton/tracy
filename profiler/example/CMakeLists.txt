cmake_minimum_required(VERSION 3.25)

project(tracy-profiler-example LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} main.cpp)

# Tracy public headers for client macros
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../../public
)

# Link to the profiler library and GLFW
target_link_libraries(${PROJECT_NAME} PRIVATE
    tracy-profiler-lib
    TracyGlfw3
)

# ImGui backends
target_sources(${PROJECT_NAME} PRIVATE
    ${ImGui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${ImGui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Tracy client - compile directly with TRACY_ENABLE
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../../public/TracyClient.cpp
)
target_compile_definitions(${PROJECT_NAME} PRIVATE TRACY_ENABLE)
