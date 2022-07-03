

macro(run_my_code_generator TargetName CompilationDatabaseDir GeneratedFileOutputDir ExportApi InputHeaders OutputFiles)
    unset(GeneratedHeaderFiles)
    unset(GeneratedSourceFiles)
    foreach(OriginalHeader ${${InputHeaders}})
        string(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${OriginalHeader})
        set(GeneratedHeaderFile ${CMAKE_CURRENT_BINARY_DIR}/${FILE_NAME}.gen.h)
        set(GeneratedSourceFile ${CMAKE_CURRENT_BINARY_DIR}/${FILE_NAME}.gen.cpp)
        list(APPEND GeneratedHeaderFiles ${GeneratedHeaderFile})
        list(APPEND GeneratedSourceFiles ${GeneratedSourceFile})
        if(NOT EXISTS ${GeneratedHeaderFile})
            file(WRITE ${GeneratedHeaderFile} "#pragma once")
        endif()
    endforeach()
    # message(${OutputFiles})
    set(${OutputFiles} ${GeneratedHeaderFiles} ${GeneratedSourceFiles})
    # message(${OutputFiles})
    message("${MY_CODE_GENERATOR_EXECUTABLE} ${CompilationDatabaseDir} ${GeneratedFileOutputDir} -${ExportApi} ${${InputHeaders}}")
    add_custom_target(${TargetName}
        ALL
        COMMAND ${MY_CODE_GENERATOR_EXECUTABLE} ${CompilationDatabaseDir} ${GeneratedFileOutputDir} -${ExportApi} ${${InputHeaders}}
        DEPENDS ${${InputHeaders}}
        BYPRODUCTS ${${OutputFiles}}
        COMMENT "MyCodeGenerator working ..."
    )
endmacro()

macro(find_my_code_generator SearchDir)
    find_program(
        MY_CODE_GENERATOR_EXECUTABLE
        NAMES
            MyCodeGenerator
        DOC "MyCodeGenerator executable location"
        PATHS ${SearchDir}
    )
    if(MY_CODE_GENERATOR_EXECUTABLE)
        message("CODE_GENERATOR_EXECUTABLE ${REFL_GENERATOR_EXECUTABLE}")
    else()
        message(FATAL_ERROR "CODE_GENERATOR_EXECUTABLE ${REFL_GENERATOR_EXECUTABLE}")
    endif()
endmacro()