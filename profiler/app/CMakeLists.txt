cmake_minimum_required(VERSION 3.25)


set(CMAKE_CXX_STANDARD 20)

project(
    tracy-profiler
    LANGUAGES C CXX
    VERSION ${TRACY_VERSION_STRING}
)

set(PROFILER_FILES
    src/ConnectionHistory.cpp
    src/Filters.cpp
    src/HttpRequest.cpp
    src/ImGuiContext.cpp
    src/IsElevated.cpp
    src/main.cpp
    src/ResolvService.cpp
    src/RunQueue.cpp
    src/WindowPosition.cpp
    src/winmain.cpp
    src/winmainArchDiscovery.cpp
)


set(INCLUDES "${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_LIST_DIR}/src")
set(LIBS "")

if(USE_WAYLAND)
    pkg_check_modules(WAYLAND REQUIRED egl wayland-egl wayland-cursor xkbcommon)
    set(INCLUDES "${INCLUDES};${WAYLAND_INCLUDE_DIRS}")
    set(LIBS "${LIBS};${WAYLAND_LIBRARIES}")
    set(PROFILER_FILES ${PROFILER_FILES}
        src/BackendWayland.cpp
    )

    include(${CMAKE_CURRENT_LIST_DIR}/../cmake/FindWaylandScanner.cmake)

    CPMAddPackage(
        NAME wayland-protocols
        GIT_REPOSITORY https://gitlab.freedesktop.org/wayland/wayland-protocols.git
        GIT_TAG 1.47
        DOWNLOAD_ONLY YES
    )

    ecm_add_wayland_client_protocol(PROFILER_FILES
        PROTOCOL ${wayland-protocols_SOURCE_DIR}/stable/xdg-shell/xdg-shell.xml
        BASENAME xdg-shell
    )
    ecm_add_wayland_client_protocol(PROFILER_FILES
        PROTOCOL ${wayland-protocols_SOURCE_DIR}/staging/xdg-activation/xdg-activation-v1.xml
        BASENAME xdg-activation
    )
    ecm_add_wayland_client_protocol(PROFILER_FILES
        PROTOCOL ${wayland-protocols_SOURCE_DIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
        BASENAME xdg-decoration
    )
    ecm_add_wayland_client_protocol(PROFILER_FILES
        PROTOCOL ${wayland-protocols_SOURCE_DIR}/staging/fractional-scale/fractional-scale-v1.xml
        BASENAME fractional-scale
    )
    ecm_add_wayland_client_protocol(PROFILER_FILES
        PROTOCOL ${wayland-protocols_SOURCE_DIR}/stable/viewporter/viewporter.xml
        BASENAME viewporter
    )
    ecm_add_wayland_client_protocol(PROFILER_FILES
        PROTOCOL ${wayland-protocols_SOURCE_DIR}/staging/cursor-shape/cursor-shape-v1.xml
        BASENAME cursor-shape
    )
    ecm_add_wayland_client_protocol(PROFILER_FILES
        PROTOCOL ${wayland-protocols_SOURCE_DIR}/unstable/tablet/tablet-unstable-v2.xml
        BASENAME tablet
    )
    ecm_add_wayland_client_protocol(PROFILER_FILES
        PROTOCOL ${wayland-protocols_SOURCE_DIR}/staging/xdg-toplevel-icon/xdg-toplevel-icon-v1.xml
        BASENAME xdg-toplevel-icon
    )
elseif(EMSCRIPTEN)
    set(PROFILER_FILES ${PROFILER_FILES}
        src/BackendEmscripten.cpp
    )
else()
    set(PROFILER_FILES ${PROFILER_FILES}
        src/BackendGlfw.cpp
        ${ImGui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    )
endif()

include_directories(${INCLUDES})
link_libraries(${LIBS})

if(SELF_PROFILE)
    set(PROFILER_FILES ${PROFILER_FILES}
        ../public/TracyClient.cpp
    )
endif()

if(WIN32)
    set(PROFILER_FILES ${PROFILER_FILES}
        win32/Tracy.manifest
        win32/Tracy.rc
    )
    add_executable(${PROJECT_NAME} WIN32 ${PROFILER_FILES})
    set_property(DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME} ${PROFILER_FILES})
endif()

# Link to the profiler library to get its headers and dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE tracy-profiler-lib)

if(NOT DEFINED GIT_REV)
    set(GIT_REV "HEAD")
endif()

find_package(Git)
if(Git_FOUND)
    add_custom_target(git-ref
        COMMAND ${CMAKE_COMMAND} -E echo "#pragma once" > GitRef.hpp.tmp
        COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_CURRENT_SOURCE_DIR} log -1 "--format=namespace tracy { static inline const char* GitRef = %x22%h%x22; }" ${GIT_REV} >> GitRef.hpp.tmp || echo "namespace tracy { static inline const char* GitRef = \"unknown\"; }" >> GitRef.hpp.tmp
        COMMAND ${CMAKE_COMMAND} -E copy_if_different GitRef.hpp.tmp GitRef.hpp
        BYPRODUCTS GitRef.hpp GitRef.hpp.tmp
        VERBATIM
    )
    add_dependencies(${PROJECT_NAME} git-ref)
else()
    message(WARNING "git not found, using 'unknown' as git ref.")
    add_custom_command(
        OUTPUT GitRef.hpp
        COMMAND ${CMAKE_COMMAND} -E echo "#pragma once" > GitRef.hpp
        COMMAND ${CMAKE_COMMAND} -E echo "namespace tracy { static inline const char* GitRef = \"unknown\"; }" >> GitRef.hpp
        VERBATIM
    )
    target_sources(${PROJECT_NAME} PUBLIC GitRef.hpp)
endif()

if(NOT EMSCRIPTEN)
    if(NOT USE_WAYLAND)
        target_link_libraries(${PROJECT_NAME} PRIVATE TracyGlfw3)
    endif()
endif()

if(EMSCRIPTEN)
    target_link_options(${PROJECT_NAME} PRIVATE -pthread -sASSERTIONS=0 -sINITIAL_MEMORY=384mb -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=4gb -sSTACK_SIZE=1048576 -sWASM_BIGINT=1 -sPTHREAD_POOL_SIZE=8 -sEXPORTED_FUNCTIONS=_main,_nativeOpenFile -sEXPORTED_RUNTIME_METHODS=ccall -sENVIRONMENT=web,worker --preload-file embed.tracy)

    file(DOWNLOAD https://share.nereid.pl/i/embed.tracy ${CMAKE_CURRENT_BINARY_DIR}/embed.tracy EXPECTED_MD5 ca0fa4f01e7b8ca5581daa16b16c768d)
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/wasm/index.html DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/wasm/httpd.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY_FILE ${CMAKE_CURRENT_LIST_DIR}/../icon/icon.svg ${CMAKE_CURRENT_BINARY_DIR}/favicon.svg)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
