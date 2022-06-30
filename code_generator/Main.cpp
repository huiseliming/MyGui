#include "clang-c/Index.h"
#include "clang-c/CXCompilationDatabase.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <inja/inja.hpp>
#include <filesystem>
#include "MetaAttributeStringParser.h"


struct CursorNode
{
    nlohmann::json _Data;
    std::map<std::string, std::string> _Attributes;
    CXCursor _Cursor;
    CursorNode* _ParentCursorNode{ nullptr };
    std::vector<std::unique_ptr<CursorNode>> _ChildrenCursorNode;
};

std::ostream& operator<<(std::ostream& stream, const CXString& str)
{
    stream << clang_getCString(str);
    clang_disposeString(str);
    return stream;
}

std::string ApiName;
std::string GeneratedOuputDir;

struct TranslationUnitClientData
{
    CursorNode* _ParentCursorNode{ nullptr };
    CursorNode* _CurrentCursorNode{ nullptr };
    CursorNode _RootCursorNode;
    std::string _OriginalHeader;
    std::string _GeneratedSourceFile;
    std::string _GeneratedHeaderFile;
    std::string _GeneratedSourceCode;
    std::string _GeneratedHeaderCode;
};

std::string ToString(CXString cx_string)
{
    std::string ret_val = clang_getCString(cx_string);
    clang_disposeString(cx_string);
    return ret_val;
}
void CursorNodeLoop(TranslationUnitClientData& translation_unit_client_data_ref, CursorNode* cursor_node);

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
        ApiName = &ArgV[3][1];
        GeneratedOuputDir = ArgV[2];

        std::vector<std::string> Headers;
        for (size_t i = 4; i < ArgC; i++)
        {
            Headers.push_back(ArgV[i]);
        }
        std::cout << "CompilationDatabaseDir: " << ArgV[1] << std::endl;
        std::cout << "ApiName               : " << ApiName << std::endl;
        std::cout << "GeneratedOuputDir     : " << GeneratedOuputDir << std::endl;
        for (size_t i = 0; i < Headers.size(); i++)
        {
            TranslationUnitClientData translation_unit_client_data;
            //std::cout << "Header" << i << ": " << Headers[i] << std::endl;
            std::string original_header = Headers[i];
            std::size_t dot_pos = original_header.rfind(".");
            std::size_t last_separator_pos = original_header.find_last_of("\\/");
            translation_unit_client_data._OriginalHeader = original_header;

            translation_unit_client_data._GeneratedHeaderFile = GeneratedOuputDir + original_header.substr(last_separator_pos, dot_pos - last_separator_pos) + ".gen" + original_header.substr(dot_pos);
            translation_unit_client_data._GeneratedSourceFile = GeneratedOuputDir + original_header.substr(last_separator_pos, dot_pos - last_separator_pos) + ".gen.cpp";
            {
                std::error_code ErrorCode;
                std::filesystem::file_time_type OriginalHeaderLastWriteTime = std::filesystem::last_write_time(translation_unit_client_data._OriginalHeader, ErrorCode);
                std::filesystem::file_time_type GeneratedHeaderLastWriteTime = std::filesystem::last_write_time(translation_unit_client_data._GeneratedHeaderFile, ErrorCode);
                if (OriginalHeaderLastWriteTime == GeneratedHeaderLastWriteTime)
                {
                    continue;
                }
            }
            CXCompileCommands compile_commands = clang_CompilationDatabase_getCompileCommands(compilation_database, original_header.c_str());
            std::vector<std::string> compile_command_args;
            uint32_t compile_commands_size = clang_CompileCommands_getSize(compile_commands);
            for (size_t j = 0; j < compile_commands_size; j++)
            {
                CXCompileCommand compile_command = clang_CompileCommands_getCommand(compile_commands, j);
                uint32_t compile_command_num_args = clang_CompileCommand_getNumArgs(compile_command);
                for (size_t k = 0; k < compile_command_num_args; k++)
                {
                    CXString compile_command_arg = clang_CompileCommand_getArg(compile_command, k);
                    std::string compile_command_arg_string = clang_getCString(compile_command_arg);
                    clang_disposeString(compile_command_arg);
                    //std::cout << compile_command_arg_string << std::endl;
                    compile_command_args.push_back(compile_command_arg_string);
                }
            }
            compile_command_args.push_back("-D__RUN_CODE_GENERATOR__");
            std::vector<const char*> command_line_args;
            for (size_t j = 0; j < compile_command_args.size(); j++)
            {
                auto& compile_command_arg = compile_command_args[j];
                if (compile_command_arg.size() > 0)
                {
                    static std::unordered_set<std::string> UnrecognizedCompileFlagSet = {
                        "/TP",
                        "/TC",
                    };
                    if (!UnrecognizedCompileFlagSet.contains(compile_command_args[j].c_str()))
                    {
                        command_line_args.push_back(compile_command_args[j].c_str());
                    }
                }
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
            translation_unit_client_data._RootCursorNode._Cursor = translation_unit_cursor;
            translation_unit_client_data._ParentCursorNode = &translation_unit_client_data._RootCursorNode;
            translation_unit_client_data._CurrentCursorNode = &translation_unit_client_data._RootCursorNode;
            clang_visitChildren(
                translation_unit_cursor,
                [](CXCursor current_cursor, CXCursor parent_cursor, CXClientData client_data)
                {
                    TranslationUnitClientData* translation_unit_client_data_ptr = (TranslationUnitClientData*)client_data;
                    auto source_range = clang_getCursorExtent(current_cursor);
                    CXSourceLocation source_location = clang_getRangeStart(source_range);
                    if (!clang_Location_isFromMainFile(source_location))
                    {
                        return CXChildVisit_Continue;
                    }
                    CXCursorKind current_cursor_kind = clang_getCursorKind(current_cursor);
                    struct VisitData
                    {
                        std::string _MetaAttributeString;
                        bool _HasMetaAttribute = false;
                    } visit_data;
                    clang_visitChildren(current_cursor,
                        [](CXCursor current_cursor, CXCursor parent_cursor, CXClientData client_data) {
                            VisitData* visit_data_ptr = (VisitData*)client_data;
                            if (clang_isAttribute(clang_getCursorKind(current_cursor))) {
                                CXString current_cursor_spelling = clang_getCursorSpelling(current_cursor);
                                std::string attributes_string = clang_getCString(current_cursor_spelling);
                                if (attributes_string.substr(0, 4) == "meta")
                                {
                                    if (attributes_string.size() > 4)
                                    {
                                        visit_data_ptr->_MetaAttributeString = attributes_string.substr(5);
                                    }
                                    visit_data_ptr->_HasMetaAttribute = true;
                                }
                                return CXChildVisit_Break;
                            }
                            return CXChildVisit_Continue;
                        },
                        & visit_data);
                    CursorNode* current_cursor_node = nullptr;
                    if(current_cursor_kind != CXCursor_EnumConstantDecl && current_cursor_kind != CXCursor_Namespace && !visit_data._HasMetaAttribute)
                    {
                        return CXChildVisit_Continue;
                    }
                    translation_unit_client_data_ptr->_ParentCursorNode->_ChildrenCursorNode.push_back(std::make_unique<CursorNode>());
                    current_cursor_node = translation_unit_client_data_ptr->_ParentCursorNode->_ChildrenCursorNode.back().get();
                    current_cursor_node->_ParentCursorNode = translation_unit_client_data_ptr->_ParentCursorNode;
                    current_cursor_node->_Cursor = current_cursor;
                    current_cursor_node->_Attributes = ParseMetaAttributeString(visit_data._MetaAttributeString);
                    translation_unit_client_data_ptr->_CurrentCursorNode = current_cursor_node;
                    switch (current_cursor_kind)
                    {
                    case CXCursor_Namespace:
                    case CXCursor_StructDecl:
                    case CXCursor_ClassDecl:
                    case CXCursor_EnumDecl:
                        translation_unit_client_data_ptr->_ParentCursorNode = current_cursor_node;
                        return CXChildVisit_Recurse;
                    case CXCursor_EnumConstantDecl:
                        return CXChildVisit_Continue;
                    default:
                        std::cout << "'" << clang_getCursorSpelling(current_cursor) << "' of kind '" << clang_getCursorKindSpelling(clang_getCursorKind(current_cursor)) << "'\n";
                        return CXChildVisit_Continue;
                        break;
                    }
                    return CXChildVisit_Continue;
                },
                &translation_unit_client_data);

            translation_unit_client_data._GeneratedSourceCode = "#include \"" + translation_unit_client_data._OriginalHeader + "\"\n";
            translation_unit_client_data._GeneratedHeaderCode = "#pragma once";
            CursorNodeLoop(translation_unit_client_data, &translation_unit_client_data._RootCursorNode);
            //std::cout << " --- GeneratedHeaderCode ---" << std::endl;
            //std::cout << translation_unit_client_data._GeneratedHeaderCode << std::endl;
            {
                std::fstream OutputGeneratedFileStream;
                OutputGeneratedFileStream.open(translation_unit_client_data._GeneratedHeaderFile, std::ios::out | std::ios::trunc);
                if (OutputGeneratedFileStream.is_open())
                {
                    OutputGeneratedFileStream.write(translation_unit_client_data._GeneratedHeaderCode.data(), translation_unit_client_data._GeneratedHeaderCode.size());
                    OutputGeneratedFileStream.close();
                    std::error_code ErrorCode;
                    std::filesystem::file_time_type DotHLastWriteTime = std::filesystem::last_write_time(translation_unit_client_data._OriginalHeader, ErrorCode);
                    std::filesystem::last_write_time(translation_unit_client_data._GeneratedHeaderFile, DotHLastWriteTime, ErrorCode);
                }
            }
            //std::cout << " --- GeneratedSourceCode ---" << std::endl;
            //std::cout << translation_unit_client_data._GeneratedSourceCode << std::endl;
            {
                std::fstream OutputGeneratedFileStream;
                OutputGeneratedFileStream.open(translation_unit_client_data._GeneratedSourceFile, std::ios::out | std::ios::trunc);
                if (OutputGeneratedFileStream.is_open())
                {
                    OutputGeneratedFileStream.write(translation_unit_client_data._GeneratedSourceCode.data(), translation_unit_client_data._GeneratedSourceCode.size());
                    OutputGeneratedFileStream.close();
                    std::error_code ErrorCode;
                    std::filesystem::file_time_type DotHLastWriteTime = std::filesystem::last_write_time(translation_unit_client_data._OriginalHeader, ErrorCode);
                    std::filesystem::last_write_time(translation_unit_client_data._GeneratedSourceFile, DotHLastWriteTime, ErrorCode);
                }
            }
            clang_disposeTranslationUnit(translation_unit);
            clang_disposeIndex(index);
        }
    }
    clang_CompilationDatabase_dispose(compilation_database);
    return EXIT_SUCCESS;
}

std::string GetNamespaceName(CursorNode* cursor_node) 
{
    std::string namespace_name;
    CursorNode* loopped_cursor_node = cursor_node;
    while (loopped_cursor_node)
    {
        CXCursorKind children_cursor_kind = clang_getCursorKind(loopped_cursor_node->_Cursor);
        switch (children_cursor_kind)
        {
        case CXCursor_Namespace:
            namespace_name = ToString(clang_getCursorSpelling(loopped_cursor_node->_Cursor)) + "::" + namespace_name;
            break;
        default:
            break;
        }
        loopped_cursor_node = loopped_cursor_node->_ParentCursorNode;
    }
    while (namespace_name.back() == ':')
    {
        namespace_name.pop_back();
    }
    return namespace_name;
}

void CursorNodeLoop(TranslationUnitClientData& translation_unit_client_data_ref, CursorNode* cursor_node)
{
    using namespace inja;
    using namespace nlohmann;
    for (size_t i = 0; i < cursor_node->_ChildrenCursorNode.size(); i++)
    {
        CursorNode* children_cursor_node = cursor_node->_ChildrenCursorNode[i].get();
        CXCursor children_cursor = children_cursor_node->_Cursor;
        CXCursorKind children_cursor_kind = clang_getCursorKind(children_cursor);

        children_cursor_node->_Data["decl_type"] = children_cursor_kind;
        children_cursor_node->_Data["name"] = ToString(clang_getCursorSpelling(children_cursor));
        children_cursor_node->_Data["attributes"] = nlohmann::json::object();
        for (const auto& [key, value] : children_cursor_node->_Attributes) {
            children_cursor_node->_Data["attributes"][key] = value;
        }
        switch (children_cursor_kind)
        {
        case CXCursor_StructDecl:
        case CXCursor_ClassDecl:
        {
            children_cursor_node->_Data["children"] = nlohmann::json::array();
            CursorNodeLoop(translation_unit_client_data_ref, children_cursor_node);

            std::string class_source_template =R"(
namespace {{namespace}}{
template<>
struct TDefaultClassInitializer<{{name}}>
{
    void operator()(Class* uninitialized_class)
    {
        AddTypeToNameMap("{{namespace}}::{{name}}", uninitialized_class);
{% for key, value in attributes %}
        uninitialized_class->_AttributeMap.insert(std::make_pair("{{key}}", std::any({{value}})));{% endfor %}

## for child in children
        uninitialized_class->_Fields.push_back(MakeField<{{child.cpp_type}}>("{{child.name}}", offsetof({{name}}, {{child.name}})));{% for key, value in child.attributes %}
        uninitialized_class->_Fields.back()->_AttributeMap.insert(std::make_pair("{{key}}", std::any({{value}})));{% endfor %}

## endfor
    }
};
Class* {{name}}::StaticClass()
{
    static Class static_class("{{namespace}}::{{name}}");
    return &static_class;
}
static TClassAutoInitializer<{{name}}> S{{name}}AutoInitializer;
})";
            children_cursor_node->_Data["namespace"] = GetNamespaceName(cursor_node);
            translation_unit_client_data_ref._GeneratedSourceCode += inja::render(class_source_template, children_cursor_node->_Data);
            break;
        }
        case CXCursor_FieldDecl:
        {
            CXType type = clang_getCursorType(children_cursor);
            children_cursor_node->_Data["cpp_type"] = ToString(clang_getTypeSpelling(type));
            cursor_node->_Data["children"].push_back(children_cursor_node->_Data);
            break;
        }
        case CXCursor_EnumDecl:
        {
            children_cursor_node->_Data["children"] = nlohmann::json::array();
            CursorNodeLoop(translation_unit_client_data_ref, children_cursor_node);

            std::string enum_header_template = R"(
#define STATIC_ENUM_{{name}} {{export_api_name}} template<> Enum* GetStaticEnum<{{name}}>();
)";
            children_cursor_node->_Data["export_api_name"] = ApiName;
            translation_unit_client_data_ref._GeneratedHeaderCode += inja::render(enum_header_template, children_cursor_node->_Data) + "\n";

            std::string enum_source_template = R"(
namespace {{namespace}}{
template<>
struct TDefaultEnumInitializer<{{name}}>
{
    void operator()(Enum* uninitialized_enum)
    {
        AddTypeToNameMap("{{namespace}}::{{name}}", uninitialized_enum);
## for child in children
        uninitialized_enum->_EnumValueMap.insert(std::make_pair({{name}}::{{child.name}}, EnumValue{ {{name}}::{{child.name}}, "{{child.name}}", "{{child.display_name}}" }));
## endfor
    }
};
template<>
Enum* GetStaticEnum<{{name}}>()
{
	static Enum static_enum("{{namespace}}::{{name}}");
	return &static_enum;
}
static TEnumAutoInitializer<{{name}}> S{{name}}AutoInitializer;
})";
            children_cursor_node->_Data["namespace"] = GetNamespaceName(cursor_node);
            translation_unit_client_data_ref._GeneratedSourceCode += inja::render(enum_source_template, children_cursor_node->_Data);
            break;
        }
        case CXCursor_EnumConstantDecl:
        {
            if (children_cursor_node->_Attributes.contains("DisplayName"))
            {
                const std::string attribute_display_name = children_cursor_node->_Attributes["DisplayName"];
                if (!attribute_display_name.empty())
                {
                    if (attribute_display_name.back() == '"' && attribute_display_name[0] == '"')
                    {
                        children_cursor_node->_Data["display_name"] = attribute_display_name.substr(1, attribute_display_name.size() - 2);
                    }
                    else
                    {
                        children_cursor_node->_Data["display_name"] = children_cursor_node->_Attributes["DisplayName"];
                    }
                }
                else
                {
                    children_cursor_node->_Data["display_name"] = children_cursor_node->_Attributes["DisplayName"];
                }
            }
            else
            {
                children_cursor_node->_Data["display_name"] = children_cursor_node->_Data["name"];
            }
            cursor_node->_Data["children"].push_back(children_cursor_node->_Data);
            break;
        }
        default:
            CursorNodeLoop(translation_unit_client_data_ref, children_cursor_node);
            break;
        }
        // std::cout << "'" << clang_getCursorSpelling(current_cursor) << "' of kind '" << clang_getCursorKindSpelling(clang_getCursorKind(current_cursor)) << "'\n";
    }
}

