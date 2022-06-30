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

project(MyCodeGenerator LANGUAGES C CXX)

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

find_package(LLVM CONFIG REQUIRED)

file(GLOB CodeGeneratorSources *.cpp *.h)
add_executable(MyCodeGenerator ${CodeGeneratorSources})

set_target_properties(MyCodeGenerator 
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set_target_properties(MyCodeGenerator 
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_CURRENT_SOURCE_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_CURRENT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_CURRENT_SOURCE_DIR}"
  )
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

target_include_directories(MyCodeGenerator PUBLIC ${LLVM_INCLUDE_DIRS})
target_link_directories(MyCodeGenerator PUBLIC ${LLVM_LIBRARY_DIRS})
target_link_libraries(MyCodeGenerator PUBLIC libclang)

find_package(inja CONFIG REQUIRED)
target_link_libraries(MyCodeGenerator PRIVATE pantor::inja)

add_custom_command(TARGET MyCodeGenerator POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${LLVM_BINARY_DIR}/bin/libclang.dll
  $<TARGET_FILE_DIR:MyCodeGenerator>
)

add_custom_command(TARGET MyCodeGenerator POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${LLVM_BINARY_DIR}/bin/zlib1.dll
  $<TARGET_FILE_DIR:MyCodeGenerator>
)

# get_cmake_property(VariableNames VARIABLES)
# foreach(VariableName ${VariableNames})
#     message(STATUS "${VariableName}=${${VariableName}}")
# endforeach()
