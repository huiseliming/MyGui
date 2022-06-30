
find_package(LLVM CONFIG REQUIRED)

file(GLOB_RECURSE CodeGeneratorSources *.cpp *.h)
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
