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

    std::stack<CursorNode*> _CursorNodeStack;
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

bool IsCursorEqual(CXCursor cursor1,CXCursor cursor2)
{
    return 
        cursor1.data[0] == cursor2.data[0] && 
        cursor1.data[1] == cursor2.data[1] && 
        cursor1.data[2] == cursor2.data[2] && 
        cursor2.xdata == cursor2.xdata;
}

bool HasMetaAttribute(CXCursor in_cursor)
{
    struct VisitData
    {
        std::string _MetaAttributeString;
        bool _HasMetaAttribute = false;
    } visit_data;
    clang_visitChildren(
        in_cursor,
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
        &visit_data);
    return visit_data._HasMetaAttribute;
}

bool IsForwardDeclaration(CXCursor cursor)
{
    auto definition = clang_getCursorDefinition(cursor);
    if (clang_equalCursors(definition, clang_getNullCursor()))
        return true;
    return !clang_equalCursors(cursor, definition);
}

CursorNode* CreateCursorNode(TranslationUnitClientData* translation_unit_client_data_ptr, CXCursor cursor, std::string& MetaAttributeString)
{
    CursorNode* cursor_node = nullptr;
    translation_unit_client_data_ptr->_CursorNodeStack.top()->_ChildrenCursorNode.push_back(std::make_unique<CursorNode>());
    cursor_node = translation_unit_client_data_ptr->_CursorNodeStack.top()->_ChildrenCursorNode.back().get();
    cursor_node->_ParentCursorNode = translation_unit_client_data_ptr->_CursorNodeStack.top();
    cursor_node->_Cursor = cursor;
    cursor_node->_Attributes = ParseMetaAttributeString(MetaAttributeString);
    translation_unit_client_data_ptr->_CurrentCursorNode = cursor_node;
    return cursor_node;
}

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
        // -MYCPP_API
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
            compile_command_args.insert(compile_command_args.end(), "-D__RUN_CODE_GENERATOR__");
            //compile_command_args[0] = "clang++.exe";
            //compile_command_args.insert(compile_command_args.begin() + 8, "-std=c++11");
            //compile_command_args.insert(compile_command_args.begin() + 8, "-fms-compatibility-version=19.10");
            //compile_command_args.insert(compile_command_args.begin() + 8, "c++-header");
            //compile_command_args.insert(compile_command_args.begin() + 8, "-x");
            //compile_command_args.insert(compile_command_args.end() - 1, "-fms-extensions");
            //compile_command_args.insert(compile_command_args.end() - 1, "-fms-compatibility");
            //compile_command_args.insert(compile_command_args.end() - 1, "-fdelayed-template-parsing");

            std::vector<const char*> command_line_args;
            for (size_t j = 0; j < compile_command_args.size(); j++)
            {
                auto& compile_command_arg = compile_command_args[j];
                if (compile_command_arg.size() > 0)
                {
                    static std::unordered_set<std::string> UnrecognizedCompileFlagSet = {
                        "/TP",
                        "/TC",
                        //"/FdTARGET_COMPILE_PDB",
                        //"/FS",
                        //"/Ob0",
                        //"/Od",
                        "/MP",
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
                CXTranslationUnit_Incomplete |
                CXTranslationUnit_KeepGoing);
            if (translation_unit == nullptr)
            {
                std::cerr << "Unable to parse translation unit. Quitting." << std::endl;
                exit(-1);
            }
            uint32_t diagnostics_num = clang_getNumDiagnostics(translation_unit);
            for (size_t i = 0; i < diagnostics_num; i++)
            {
                CXDiagnostic diagnostic = clang_getDiagnostic(translation_unit, i);
                CXDiagnosticSeverity diagnostic_severity = clang_getDiagnosticSeverity(diagnostic);
                switch (diagnostic_severity)
                {
                case CXDiagnostic_Ignored:
                case CXDiagnostic_Note:
                case CXDiagnostic_Warning:
                    break;
                case CXDiagnostic_Error:
                case CXDiagnostic_Fatal:
                {
                    unsigned options =
                        CXDiagnostic_DisplaySourceLocation |
                        CXDiagnostic_DisplayColumn |
                        CXDiagnostic_DisplayOption |
                        CXDiagnostic_DisplaySourceRanges |
                        CXDiagnostic_DisplayCategoryId |
                        CXDiagnostic_DisplayCategoryName;
                    CXString formatted_diagnostic = clang_formatDiagnostic(diagnostic, options);
                    std::cout << ToString(formatted_diagnostic) << std::endl;
                }
                default:
                    break;
                }
            }
            CXCursor translation_unit_cursor = clang_getTranslationUnitCursor(translation_unit);
            translation_unit_client_data._RootCursorNode._Cursor = translation_unit_cursor;
            translation_unit_client_data._CursorNodeStack.push(&translation_unit_client_data._RootCursorNode);
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
                    if (!IsCursorEqual(translation_unit_client_data_ptr->_CursorNodeStack.top()->_Cursor, parent_cursor))
                    {
                        translation_unit_client_data_ptr->_CursorNodeStack.pop();
                    }
                    std::cout << ToString(clang_getCursorKindSpelling(clang_getCursorKind(current_cursor))) << std::endl;
                    CXCursorKind current_cursor_kind = clang_getCursorKind(current_cursor);
                    struct VisitData
                    {
                        std::string _MetaAttributeString;
                        bool _HasMetaAttribute = false;
                    } visit_data;
                    clang_visitChildren(
                        current_cursor,
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
                        &visit_data);
                    CursorNode* current_cursor_node = nullptr;
                    switch (current_cursor_kind)
                    {
                    case CXCursor_Namespace:
                        current_cursor_node = CreateCursorNode(translation_unit_client_data_ptr, current_cursor, visit_data._MetaAttributeString);
                        translation_unit_client_data_ptr->_CursorNodeStack.push(current_cursor_node);
                        return CXChildVisit_Recurse;
                    case CXCursor_StructDecl:
                    case CXCursor_ClassDecl:
                    case CXCursor_EnumDecl:
                        if (IsForwardDeclaration(current_cursor))
                        {
                            return CXChildVisit_Continue;
                        }
                        if (visit_data._HasMetaAttribute)
                        {
                            current_cursor_node = CreateCursorNode(translation_unit_client_data_ptr, current_cursor, visit_data._MetaAttributeString);
                            translation_unit_client_data_ptr->_CursorNodeStack.push(current_cursor_node);
                            return CXChildVisit_Recurse;
                        }
                        return CXChildVisit_Continue;
                    case CXCursor_CXXBaseSpecifier:
                        current_cursor_node = CreateCursorNode(translation_unit_client_data_ptr, current_cursor, visit_data._MetaAttributeString);
                        translation_unit_client_data_ptr->_CursorNodeStack.push(current_cursor_node);
                        return CXChildVisit_Recurse;
                    case CXCursor_EnumConstantDecl:
                        current_cursor_node = CreateCursorNode(translation_unit_client_data_ptr, current_cursor, visit_data._MetaAttributeString);
                        return CXChildVisit_Continue;
                    case CXCursor_TypeRef:
                        if (clang_getCursorKind(parent_cursor) == CXCursor_CXXBaseSpecifier)
                        {
                            current_cursor_node = CreateCursorNode(translation_unit_client_data_ptr, current_cursor, visit_data._MetaAttributeString);
                        }
                        return CXChildVisit_Continue;
                    case CXCursor_FieldDecl:
                        if (visit_data._HasMetaAttribute)
                        {
                            current_cursor_node = CreateCursorNode(translation_unit_client_data_ptr, current_cursor, visit_data._MetaAttributeString);
                        }
                        return CXChildVisit_Continue;
                    case CXCursor_FunctionDecl:
                        if (visit_data._HasMetaAttribute)
                        {
                            current_cursor_node = CreateCursorNode(translation_unit_client_data_ptr, current_cursor, visit_data._MetaAttributeString);
                        }
                        return CXChildVisit_Continue;
                    case CXCursor_CXXMethod:
                        if (visit_data._HasMetaAttribute)
                        {
                            current_cursor_node = CreateCursorNode(translation_unit_client_data_ptr, current_cursor, visit_data._MetaAttributeString);
                            translation_unit_client_data_ptr->_CursorNodeStack.push(current_cursor_node);
                            return CXChildVisit_Recurse;
                        }
                        return CXChildVisit_Continue;
                    case CXCursor_ParmDecl:
                        current_cursor_node = CreateCursorNode(translation_unit_client_data_ptr, current_cursor, visit_data._MetaAttributeString);
                        return CXChildVisit_Continue;
                    default:
                        return CXChildVisit_Continue;
                        break;
                    }
                    return CXChildVisit_Continue;
                },
                &translation_unit_client_data);

            translation_unit_client_data._GeneratedSourceCode = "#include \"" + translation_unit_client_data._OriginalHeader + "\"\n";
            translation_unit_client_data._GeneratedSourceCode = "#include \"Core/Reflect.h\"\n";
            translation_unit_client_data._GeneratedHeaderCode = "#pragma once\n";
            CursorNodeLoop(translation_unit_client_data, &translation_unit_client_data._RootCursorNode);
            std::cout << " --- GeneratedHeaderCode ---" << std::endl;
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
            std::cout << " --- GeneratedSourceCode ---" << std::endl;
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
            children_cursor_node->_Data["parent_classes"] = nlohmann::json::array();
            children_cursor_node->_Data["fields"] = nlohmann::json::array();
            children_cursor_node->_Data["functions"] = nlohmann::json::array();
            CursorNodeLoop(translation_unit_client_data_ref, children_cursor_node);

            std::string class_source_template =R"(
template<>
struct TDefaultTypeInitializer<{{namespace}}::{{name}}>
{
    void operator()(Core::Type* uninitialized_type)
    {
        using namespace Core;
        Class* uninitialized_class = static_cast<Class*>(uninitialized_type);

## for parent_class in parent_classes
        {% if parent_class.is_reflect_class %}uninitialized_class->AddParentClass({{parent_class.name}}::StaticClass());{% else %}uninitialized_class->AddParentClass(nullptr);{% endif %}
## endfor
{% for key, value in attributes %}
        uninitialized_class->_AttributeMap.insert(std::make_pair("{{key}}", std::any({{value}})));{% endfor %}

## for field in fields
        uninitialized_class->_Fields.push_back(Core::MakeField<{{field.cpp_type}}>("{{field.name}}", offsetof({{name}}, {{field.name}})));{% for key, value in field.attributes %}
        uninitialized_class->_Fields.back()->_AttributeMap.insert(std::make_pair("{{key}}", std::any({{value}})));{% endfor %}
## endfor

## for function in functions
        {
            struct ScriptStruct
            {
## for parm in function.parms
                Core::ConvertToRefWrapper<{{parm.cpp_type}}> {{parm.name}} { (*(std::add_pointer_t<std::remove_cvref_t<{{parm.cpp_type}}>>)nullptr) };
## endfor
                Core::ConvertToRefWrapper<{{function.return_type}}> __return { (*(std::add_pointer_t<std::remove_cvref_t<{{function.return_type}}>>)nullptr) };
            };
            uninitialized_class->_Functions.push_back(std::make_unique<TFunction<ScriptStruct, decltype(&{{namespace}}::{{name}}::{{function.name}})>>("{{function.name}}"));
            TFunction<ScriptStruct, decltype(&{{namespace}}::{{name}}::{{function.name}})>* funtion = static_cast<TFunction<ScriptStruct, decltype(&{{namespace}}::{{name}}::{{function.name}})>*>(uninitialized_class->_Functions.back().get());
## for parm in function.parms
            funtion->_Fields.push_back(Core::MakeFieldByConvertToRefWrapper<{{parm.cpp_type}}>("{{parm.name}}", offsetof(ScriptStruct, {{parm.name}})));
## endfor
            funtion->_Fields.push_back(Core::MakeFieldByConvertToRefWrapper<{{function.return_type}}>("__return", offsetof(ScriptStruct, __return)));
            funtion->_NativeCall = &{{namespace}}::{{name}}::{{function.name}};
            funtion->_VMCall =  [](void* in_object, void* in_script_struct) {
                {{name}}* object = ({{name}}*)in_object;
                ScriptStruct* vm_call_object = (ScriptStruct*)in_script_struct;
                vm_call_object->__return = object->TestAdd({% for parm in function.parms %}vm_call_object->{{parm.name}}{% if not loop.is_last %}, {% endif %}{% endfor %});
            };
        }
## endfor
    }
};
Core::Class* {{namespace}}::{{name}}::StaticClass()
{
    using namespace Core;
    static TClass<{{namespace}}::{{name}}> static_class("{{namespace}}::{{name}}");
    return &static_class;
}
static TTypeAutoInitializer<{{namespace}}::{{name}}> S{{name}}AutoInitializer;
)";
            
            children_cursor_node->_Data["namespace"] = GetNamespaceName(cursor_node);
            translation_unit_client_data_ref._GeneratedSourceCode += inja::render(class_source_template, children_cursor_node->_Data);
            break;
        }
        case CXCursor_FieldDecl:
        {
            CXType type = clang_getCursorType(children_cursor);
            children_cursor_node->_Data["cpp_type"] = ToString(clang_getTypeSpelling(type));
            cursor_node->_Data["fields"].push_back(children_cursor_node->_Data);
            break;
        }
        case CXCursor_CXXMethod:
        {
            children_cursor_node->_Data["parms"] = nlohmann::json::array();
            children_cursor_node->_Data["return_type"] = ToString(clang_getTypeSpelling(clang_getResultType(clang_getCursorType(children_cursor))));
            for (size_t i = 0; i < children_cursor_node->_ChildrenCursorNode.size(); i++)
            {
                children_cursor_node->_Data["parms"].push_back(nlohmann::json::object());
                CursorNode* parm_cursor_node = children_cursor_node->_ChildrenCursorNode[i].get();
                children_cursor_node->_Data["parms"][i]["name"] = ToString(clang_getCursorSpelling(parm_cursor_node->_Cursor));
                //std::string parm_cpp_type = ToString(clang_getTypeSpelling(clang_getCursorType(parm_cursor_node->_Cursor)));
                children_cursor_node->_Data["parms"][i]["cpp_type"] = ToString(clang_getTypeSpelling(clang_getCursorType(parm_cursor_node->_Cursor)));
            }
            cursor_node->_Data["functions"].push_back(children_cursor_node->_Data);
            break;
        }
        case CXCursor_EnumDecl:
        {
            children_cursor_node->_Data["enum_constants"] = nlohmann::json::array();
            CursorNodeLoop(translation_unit_client_data_ref, children_cursor_node);
            children_cursor_node->_Data["export_api_name"] = ApiName;
            children_cursor_node->_Data["namespace"] = GetNamespaceName(cursor_node);
            std::string enum_header_template = R"(
#define STATIC_ENUM_{{name}} {{export_api_name}} template<> Core::Enum* GetStaticEnum<{{namespace}}::{{name}}>();
)";
            translation_unit_client_data_ref._GeneratedHeaderCode += inja::render(enum_header_template, children_cursor_node->_Data) + "\n";

            std::string enum_source_template = R"(
template<>
struct TDefaultTypeInitializer<{{namespace}}::{{name}}>
{
    void operator()(Core::Type* uninitialized_type)
    {
        using namespace Core;
        Enum* uninitialized_enum = static_cast<Enum*>(uninitialized_type);
## for enum_constant in enum_constants
        uninitialized_enum->_EnumValueMap.insert(std::make_pair({{name}}::{{enum_constant.name}}, EnumValue{ {{name}}::{{enum_constant.name}}, "{{enum_constant.name}}", "{{enum_constant.display_name}}" }));
## endfor
    }
};

template<>
Core::Enum* GetStaticEnum<{{namespace}}::{{name}}>()
{
    using namespace Core;
	static TEnum<{{namespace}}::{{name}}> static_enum("{{namespace}}::{{name}}");
	return &static_enum;
}
static TTypeAutoInitializer<{{namespace}}::{{name}}> S{{name}}AutoInitializer;
)";
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
            cursor_node->_Data["enum_constants"].push_back(children_cursor_node->_Data);
            break;
        }
        case CXCursor_TypeRef:
            if (clang_getCursorKind(cursor_node->_Cursor) == CXCursor_CXXBaseSpecifier)
            {
                children_cursor_node->_Data["name"] = ToString(clang_getTypeSpelling(clang_getCursorType(children_cursor_node->_Cursor)));
                CXCursor parent_class_cursor = clang_getTypeDeclaration(clang_getCursorType(children_cursor_node->_Cursor));
                children_cursor_node->_Data["is_reflect_class"] = HasMetaAttribute(parent_class_cursor);
                cursor_node->_ParentCursorNode->_Data["parent_classes"].push_back(children_cursor_node->_Data);
            }
            break;
        default:
            CursorNodeLoop(translation_unit_client_data_ref, children_cursor_node);
            break;
        }
        // std::cout << "'" << clang_getCursorSpelling(current_cursor) << "' of kind '" << clang_getCursorKindSpelling(clang_getCursorKind(current_cursor)) << "'\n";
    }
}

