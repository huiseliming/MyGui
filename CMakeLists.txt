cmake_minimum_required(VERSION 3.20)

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
else()
    message(FATAL_ERROR "NOT FIND ENVIRONMENT VARIABLE VCPKG_ROOT")
endif()
if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
elseif(UNIX)
    set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "")
elseif(APPLE)
    set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "")
else()
    message(FATAL_ERROR "NOT SUPPORTED VCPKG_TARGET_TRIPLET")
endif(WIN32)

message("CMAKE_TOOLCHAIN_FILE: " ${CMAKE_TOOLCHAIN_FILE})
message("VCPKG_TARGET_TRIPLET: " ${VCPKG_TARGET_TRIPLET})

project(EasyGui LANGUAGES C CXX)

if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE PublicSource public/*.h)
file(GLOB_RECURSE PrivateSource private/*.h private/*.cpp)

add_subdirectory(third_party)

set(ImGuiSource
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_vulkan.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_vulkan.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_glfw.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp)

add_library(EasyGui ${PublicSource} ${PrivateSource} ${ImGuiSource})

target_include_directories(EasyGui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/public>
    $<INSTALL_INTERFACE:public>
)
target_include_directories(EasyGui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(EasyGui PUBLIC spdlog::spdlog spdlog::spdlog_header_only)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(EasyGui PUBLIC glfw)

find_package(glm CONFIG REQUIRED)
target_link_libraries(EasyGui PUBLIC glm::glm)

find_package(Vulkan REQUIRED)
target_link_libraries(EasyGui PUBLIC Vulkan::Vulkan)

find_package(minhook CONFIG REQUIRED)
target_link_libraries(EasyGui PRIVATE minhook::minhook)

add_custom_command(TARGET EasyGui POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/assets
  ${CMAKE_CURRENT_BINARY_DIR}
)

include(GenerateExportHeader)
generate_export_header(EasyGui
    EXPORT_MACRO_NAME EASYGUI_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/public/EasyGuiExport.h
)

add_subdirectory(tests)
