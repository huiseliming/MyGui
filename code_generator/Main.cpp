#include "clang-c/Index.h"
#include "clang-c/CXCompilationDatabase.h"
#include <iostream>
#include <vector>


std::ostream& operator<<(std::ostream& stream, const CXString& str)
{
    stream << clang_getCString(str);
    clang_disposeString(str);
    return stream;
}

std::string ApiName;

int main(int ArgC, char* ArgV[])
{
    if (ArgC < 4)
    {
        std::cout << "command example :" << std::endl;
        std::cout << "MyCodeGenerator path/to/compilation_database -XXX_API path/to/header1.h path/to/header2.h path/to/header3.h" << std::endl;
        return EXIT_FAILURE;
    }
    CXCompilationDatabase_Error error;
    CXCompilationDatabase compilation_database = clang_CompilationDatabase_fromDirectory(ArgV[1], &error);
    if (CXCompilationDatabase_NoError != error)
    {
        std::cout << "failed to call clang_CompilationDatabase_fromDirectory" << std::endl;
        return EXIT_FAILURE;
    }
    {
        // -MYGUI_API
        ApiName = &ArgV[2][1];


        std::vector<std::string> Headers;
        for (size_t i = 3; i < ArgC; i++)
        {
            Headers.push_back(ArgV[i]);
        }
        std::cout << "CompilationDatabaseDir: " << ArgV[1] << std::endl;
        std::cout << "ApiName: " << ApiName << std::endl;
        for (size_t i = 0; i < Headers.size(); i++)
        {
            std::cout << "Header" << i << ": " << Headers[i] << std::endl;

            CXCompileCommands compile_commands = clang_CompilationDatabase_getCompileCommands(compilation_database, Headers[i].c_str());
            std::vector<std::string> compile_command_args;
            uint32_t compile_commands_size = clang_CompileCommands_getSize(compile_commands);
            for (size_t i = 0; i < compile_commands_size; i++)
            {
                CXCompileCommand compile_command = clang_CompileCommands_getCommand(compile_commands, i);
                uint32_t compile_command_num_args = clang_CompileCommand_getNumArgs(compile_command);
                for (size_t i = 0; i < compile_command_num_args; i++)
                {
                    CXString compile_command_arg = clang_CompileCommand_getArg(compile_command, i);
                    std::string compile_command_arg_string = clang_getCString(compile_command_arg);
                    clang_disposeString(compile_command_arg);
                    std::cout << compile_command_arg_string << std::endl;
                    compile_command_args.push_back(compile_command_arg_string);
                }
            }
            compile_command_args.push_back("-D__RUN_CODE_GENERATOR__");
            std::vector<const char*> command_line_args;
            for (size_t i = 0; i < compile_command_args.size(); i++)
            {
                command_line_args.push_back(compile_command_args[i].c_str());
            }
            CXIndex index = clang_createIndex(0, 0);
            CXTranslationUnit translation_unit = clang_parseTranslationUnit(
                index,
                nullptr,
                command_line_args.data(), command_line_args.size(),
                nullptr, 0,
                CXTranslationUnit_None);
            if (translation_unit == nullptr)
            {
                std::cerr << "Unable to parse translation unit. Quitting." << std::endl;
                exit(-1);
            }
            CXCursor translation_unit_cursor = clang_getTranslationUnitCursor(translation_unit);
            clang_visitChildren(
                translation_unit_cursor,
                [](CXCursor current_cursor, CXCursor parent_cursor, CXClientData client_data)
                {
                    auto source_range = clang_getCursorExtent(current_cursor);
                    CXSourceLocation source_location = clang_getRangeStart(source_range);
                    if (!clang_Location_isFromMainFile(source_location))
                    {
                        return CXChildVisit_Continue;
                    }
                    CXCursorKind current_cursor_kind = clang_getCursorKind(current_cursor);
                    struct VisitData
                    {
                        bool has_meta_attribute = false;
                    } visit_data;
                    clang_visitChildren(current_cursor,
                        [](CXCursor current_cursor, CXCursor parent_cursor, CXClientData client_data) {
                            VisitData* visit_data_ptr = (VisitData*)client_data;
                            if (clang_isAttribute(clang_getCursorKind(current_cursor))) {
                                CXString current_cursor_spelling = clang_getCursorSpelling(current_cursor);
                                std::string attributes_string = clang_getCString(current_cursor_spelling);
                                std::cout << attributes_string << std::endl;
                                if (attributes_string.substr(0, 4) == "meta")
                                {
                                    visit_data_ptr->has_meta_attribute = true;
                                }
                                return CXChildVisit_Break;
                            }
                            return CXChildVisit_Continue;
                        },
                        & visit_data);
                    switch (current_cursor_kind)
                    {
                    case CXCursor_Namespace:
                        return CXChildVisit_Recurse;
                    case CXCursor_StructDecl:
                    case CXCursor_ClassDecl:
                    case CXCursor_EnumDecl:
                        if (!visit_data.has_meta_attribute) return CXChildVisit_Continue;
                        std::cout << "'" << clang_getCursorSpelling(current_cursor) << "' of kind '" << clang_getCursorKindSpelling(clang_getCursorKind(current_cursor)) << "'\n";
                        return CXChildVisit_Recurse;
                    case CXCursor_EnumConstantDecl:
                    default:
                        break;
                    }
                    std::cout << "'" << clang_getCursorSpelling(current_cursor) << "' of kind '" << clang_getCursorKindSpelling(clang_getCursorKind(current_cursor)) << "'\n";
                    return CXChildVisit_Continue;
                },
                nullptr);
            clang_disposeTranslationUnit(translation_unit);
            clang_disposeIndex(index);


        }
    }
    clang_CompilationDatabase_dispose(compilation_database);
    return EXIT_SUCCESS;
}




