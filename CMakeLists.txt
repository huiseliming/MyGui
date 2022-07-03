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

project(MyGui LANGUAGES C CXX)

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

set(OriginalHeaders
    ${CMAKE_CURRENT_SOURCE_DIR}/public/Core/Class.h
    ${CMAKE_CURRENT_SOURCE_DIR}/public/Core/Enum.h
    ${CMAKE_CURRENT_SOURCE_DIR}/public/Core/Field.h
    ${CMAKE_CURRENT_SOURCE_DIR}/public/Core/Record.h
    ${CMAKE_CURRENT_SOURCE_DIR}/public/Core/Struct.h
    ${CMAKE_CURRENT_SOURCE_DIR}/public/Core/Type.h
    ${CMAKE_CURRENT_SOURCE_DIR}/public/Core/Object.h
)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
include(my_code_generator)
find_my_code_generator("${CMAKE_CURRENT_SOURCE_DIR}/code_generator/")
run_my_code_generator(
    MyGui-CodeGenerator         #TargetName
    ${CMAKE_BINARY_DIR}         #CompilationDatabaseDir
    ${CMAKE_CURRENT_BINARY_DIR} #GeneratedFileOutputDir
    MYGUI_API                   #ExportApi
    OriginalHeaders             #InputHeaders
    GeneratedFiles              #OutputFiles
)
add_library(MyGui ${PublicSource} ${PrivateSource} ${ImGuiSource} ${GeneratedFiles})
add_dependencies(MyGui MyGui-CodeGenerator)

source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR} 
  PREFIX Source 
  FILES ${PublicSource} ${PrivateSource} ${ImGuiSource}
)

source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR} 
  PREFIX Generated 
  FILES ${GeneratedFiles}
)

target_include_directories(MyGui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/public>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:public>
)
target_include_directories(MyGui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(MyGui PUBLIC spdlog::spdlog spdlog::spdlog_header_only)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(MyGui PUBLIC glfw)

# find_package(glm CONFIG REQUIRED)
# target_link_libraries(MyGui PUBLIC glm::glm)

find_package(Vulkan REQUIRED)
target_link_libraries(MyGui PUBLIC Vulkan::Vulkan)

find_package(minhook CONFIG REQUIRED)
target_link_libraries(MyGui PRIVATE minhook::minhook)

find_package(minhook CONFIG REQUIRED)
target_link_libraries(MyGui PRIVATE minhook::minhook)

find_package(Poco CONFIG REQUIRED Util)
target_link_libraries(MyGui PUBLIC Poco::Util)

# add_custom_command(TARGET MyGui POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy_directory
#   ${CMAKE_CURRENT_SOURCE_DIR}/assets
#   $<TARGET_FILE_DIR:MyGui>
#   ${CMAKE_BINARY_DIR}
# )

include(GenerateExportHeader)
generate_export_header(MyGui
    EXPORT_MACRO_NAME MYGUI_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/public/MyGuiExport.h
)

add_subdirectory(tests)
