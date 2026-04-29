cmake_minimum_required(VERSION 3.25)

option(NO_APP "Disable the profiler application" OFF)
option(NO_FILESELECTOR "Disable the file selector" OFF)
option(NO_STB_IMAGE "Disable stb_image (use if host project already provides it)" OFF)
option(GTK_FILESELECTOR "Use the GTK file selector on Linux instead of the xdg-portal one" OFF)
option(LEGACY "Instead of Wayland, use the legacy X11 backend on Linux" OFF)
option(NO_ISA_EXTENSIONS "Disable ISA extensions (don't pass -march=native or -mcpu=native to the compiler)" OFF)
option(NO_STATISTICS "Disable calculation of statistics" OFF)

option(SELF_PROFILE "Enable self-profiling" OFF)
set(SANITIZE "" CACHE STRING "Sanitizer parameters")

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/version.cmake)

set(CMAKE_CXX_STANDARD 20)

project(
    tracy-profiler-lib
    LANGUAGES C CXX
    VERSION ${TRACY_VERSION_STRING}
)

if(SELF_PROFILE)
    add_definitions(-DTRACY_ENABLE)
    add_compile_options(-g -O3 -fno-omit-frame-pointer)
endif()

if(SANITIZE)
    add_compile_options(-fsanitize=${SANITIZE} -fno-omit-frame-pointer)
    add_link_options(-fsanitize=${SANITIZE} -fno-omit-frame-pointer)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/vendor.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/server.cmake)

include(ExternalProject)
ExternalProject_Add(embed
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/helpers
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(
    OUTPUT data
    COMMAND ${CMAKE_COMMAND} -E make_directory data
)

function(Embed LIST NAME FILE)
    add_custom_command(
        OUTPUT data/${NAME}.cpp data/${NAME}.hpp
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/embed ${NAME} ${CMAKE_CURRENT_LIST_DIR}/${FILE} data/${NAME}
        DEPENDS data embed ${CMAKE_CURRENT_LIST_DIR}/${FILE}
    )
    list(APPEND ${LIST} data/${NAME}.cpp)
    return(PROPAGATE ${LIST})
endfunction()

set(SERVER_FILES
    TracyAchievementData.cpp
    TracyAchievements.cpp
    TracyBadVersion.cpp
    TracyColor.cpp
    TracyConfig.cpp
    TracyEmbed.cpp
    TracyEventDebug.cpp
    TracyFileselector.cpp
    TracyFilesystem.cpp
    TracyImGui.cpp
    TracyManualData.cpp
    TracyMarkdown.cpp
    TracyMicroArchitecture.cpp
    TracyMouse.cpp
    TracyProtoHistory.cpp
    TracySourceContents.cpp
    TracySourceTokenizer.cpp
    TracySourceView.cpp
    TracyStorage.cpp
    TracyTexture.cpp
    TracyTimelineController.cpp
    TracyTimelineItem.cpp
    TracyTimelineItemCpuData.cpp
    TracyTimelineItemGpu.cpp
    TracyTimelineItemPlot.cpp
    TracyTimelineItemThread.cpp
    TracyUserData.cpp
    TracyUtility.cpp
    TracyView.cpp
    TracyView_Annotations.cpp
    TracyView_Callstack.cpp
    TracyView_Compare.cpp
    TracyView_ConnectionState.cpp
    TracyView_ContextSwitch.cpp
    TracyView_CpuData.cpp
    TracyView_FindZone.cpp
    TracyView_FlameGraph.cpp
    TracyView_FrameOverview.cpp
    TracyView_FrameTimeline.cpp
    TracyView_FrameTree.cpp
    TracyView_GpuTimeline.cpp
    TracyView_Locks.cpp
    TracyView_Manual.cpp
    TracyView_Memory.cpp
    TracyView_Messages.cpp
    TracyView_Navigation.cpp
    TracyView_NotificationArea.cpp
    TracyView_Options.cpp
    TracyView_Playback.cpp
    TracyView_Plots.cpp
    TracyView_Ranges.cpp
    TracyView_Samples.cpp
    TracyView_Statistics.cpp
    TracyView_Timeline.cpp
    TracyView_TraceInfo.cpp
    TracyView_Utility.cpp
    TracyView_ZoneInfo.cpp
    TracyView_ZoneTimeline.cpp
    TracyWeb.cpp
)

if(NOT EMSCRIPTEN)
    list(APPEND SERVER_FILES
        TracyLlm.cpp
        TracyLlmApi.cpp
        TracyLlmChat.cpp
        TracyLlmEmbeddings.cpp
        TracyLlmTools.cpp
    )
endif()


list(TRANSFORM SERVER_FILES PREPEND "src/profiler/")

list(APPEND SERVER_FILES
    # TODO(jack): this previously was only in the app
    src/Fonts.cpp
    src/ini.c
)

if(NOT NO_STB_IMAGE)
    list(APPEND SERVER_FILES src/stb_image_impl.cpp)
endif()

Embed(SERVER_FILES Manual ../manual/tracy.md)

Embed(SERVER_FILES SystemPrompt src/llm/system.prompt.md)
Embed(SERVER_FILES ToolsJson src/llm/tools.json)
Embed(SERVER_FILES FontFixed src/font/FiraCode-Retina.ttf)
Embed(SERVER_FILES FontIcons src/font/Font\ Awesome\ 6\ Free-Solid-900.otf)
Embed(SERVER_FILES FontNormal src/font/Roboto-Regular.ttf)
Embed(SERVER_FILES FontBold src/font/Roboto-Bold.ttf)
Embed(SERVER_FILES FontItalic src/font/Roboto-Italic.ttf)
Embed(SERVER_FILES FontBoldItalic src/font/Roboto-BoldItalic.ttf)
Embed(SERVER_FILES FontEmoji src/font/NotoEmoji-Regular.ttf)
    

set(INCLUDES "${CMAKE_CURRENT_BINARY_DIR}")
set(LIBS "")

add_library(${PROJECT_NAME} STATIC ${COMMON_FILES} ${SERVER_FILES})

# Expose include directories for consumers of the library
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/..
)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC
    TracyServer
    TracyImGui
    nlohmann_json::nlohmann_json
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Threads::Threads
    md4c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${tidy_SOURCE_DIR}/include
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${md4c_SOURCE_DIR}/src
)

include_directories(${INCLUDES})

if(NO_FILESELECTOR)
    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_NO_FILESELECTOR)
elseif(NOT EMSCRIPTEN)
    target_link_libraries(${PROJECT_NAME} PRIVATE nfd::nfd)
endif()


if(NOT EMSCRIPTEN)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        TracyLibcurl
        base64
        tidy-static
        TracyPugixml
        usearch
    )
endif()

if (NOT NO_APP)
    add_subdirectory(app)
endif()

option(BUILD_EXAMPLE "Build the embedded profiler example" OFF)
if (BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

# Graphics backend selection (set one to ON)
option(TRACY_BACKEND_OPENGL3 "Use OpenGL3 backend" OFF)
option(TRACY_BACKEND_VULKAN "Use Vulkan backend (requires SetTextureCallbacks)" OFF)
option(TRACY_BACKEND_METAL "Use Metal backend (requires SetTextureCallbacks)" OFF)
option(TRACY_BACKEND_DIRECTX11 "Use DirectX 11 backend (requires SetTextureCallbacks)" OFF)
option(TRACY_BACKEND_DIRECTX12 "Use DirectX 12 backend (requires SetTextureCallbacks)" OFF)

# For OpenGL with glad loader (e.g. HelloImGui) - set this and provide glad target
option(TRACY_OPENGL_GLAD "Use glad for OpenGL loading" OFF)

if(TRACY_BACKEND_OPENGL3)
    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_BACKEND_OPENGL3)
    if(TRACY_OPENGL_GLAD AND TARGET glad)
        target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_OPENGL_GLAD)
        target_link_libraries(${PROJECT_NAME} PUBLIC glad)
    elseif(APPLE)
        target_link_libraries(${PROJECT_NAME} PUBLIC "-framework OpenGL")
    elseif(WIN32)
        target_link_libraries(${PROJECT_NAME} PUBLIC opengl32)
    elseif(NOT EMSCRIPTEN)
        find_package(OpenGL REQUIRED)
        target_link_libraries(${PROJECT_NAME} PUBLIC OpenGL::GL)
    endif()
elseif(TRACY_BACKEND_VULKAN)
    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_BACKEND_VULKAN)
elseif(TRACY_BACKEND_METAL)
    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_BACKEND_METAL)
elseif(TRACY_BACKEND_DIRECTX11)
    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_BACKEND_DIRECTX11)
elseif(TRACY_BACKEND_DIRECTX12)
    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_BACKEND_DIRECTX12)
endif()

include(GNUInstallDirs)

set(_profiler_install_targets ${PROJECT_NAME} TracyServer TracyImGui TracyGetOpt)
foreach(_dep_target libzstd_static capstone_static capstone base64 tidy-static freetype pugixml libcurl_static md4c md4c-html)
    if(TARGET ${_dep_target})
        list(APPEND _profiler_install_targets ${_dep_target})
    endif()
endforeach()
foreach(_t ${_profiler_install_targets})
    if(TARGET ${_t})
        set_target_properties(${_t} PROPERTIES EXCLUDE_FROM_ALL FALSE)
    endif()
endforeach()
install(TARGETS ${_profiler_install_targets}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY src/profiler/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/profiler
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../server/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/server
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../public/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/public
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
install(FILES src/Fonts.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
if(PPQSort_SOURCE_DIR)
    install(DIRECTORY ${PPQSort_SOURCE_DIR}/include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.h")
endif()
if(TARGET libzstd_static)
    get_target_property(_zstd_inc libzstd_static INTERFACE_INCLUDE_DIRECTORIES)
    if(_zstd_inc)
        foreach(_dir ${_zstd_inc})
            install(FILES ${_dir}/zstd.h ${_dir}/zstd_errors.h ${_dir}/zdict.h
                    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} OPTIONAL)
        endforeach()
    endif()
endif()
