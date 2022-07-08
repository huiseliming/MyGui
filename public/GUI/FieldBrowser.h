#pragma once
#include "VulkanGuiApplication.h"

namespace GUI
{

    ImGuiDataType MapToImGuiDataType(Core::ECastTypeFlagBits CastTypeFlagBits)
    {
        switch (CastTypeFlagBits)
        {
        case Core::ECastTypeFlagBits::CTFB_FloatBit:
            return ImGuiDataType_Float;
        case Core::ECastTypeFlagBits::CTFB_DoubleBit:
            return ImGuiDataType_Double;
        case Core::ECastTypeFlagBits::CTFB_SInt8Bit:
            return ImGuiDataType_S8;
        case Core::ECastTypeFlagBits::CTFB_SInt16Bit:
            return ImGuiDataType_S16;
        case Core::ECastTypeFlagBits::CTFB_SInt32Bit:
            return ImGuiDataType_S32;
        case Core::ECastTypeFlagBits::CTFB_SInt64Bit:
            return ImGuiDataType_S64;
        case Core::ECastTypeFlagBits::CTFB_UInt8Bit:
            return ImGuiDataType_U8;
        case Core::ECastTypeFlagBits::CTFB_UInt16Bit:
            return ImGuiDataType_U16;
        case Core::ECastTypeFlagBits::CTFB_UInt32Bit:
            return ImGuiDataType_U32;
        case Core::ECastTypeFlagBits::CTFB_UInt64Bit:
            return ImGuiDataType_U64;
        default:
            break;
        }
        return ImGuiDataType_COUNT;
    }

    class FieldBrowser
    {
    public:
        void DisplayFieldModifier(void* type_instance_ptr, Core::Type* field_type_ptr, const std::string& field_name)
        {
            using namespace Core;
            std::string label_id = "##" + field_name;
            ImGui::PushItemWidth(-1);
            if (field_type_ptr->GetCastTypeFlag() & CTFB_BoolBit)
            {
                if (ImGui::Checkbox(label_id.c_str(), (bool*)type_instance_ptr))
                {
                    spdlog::info("ImGui::Checkbox");
                }
            }
            else if (field_type_ptr->GetCastTypeFlag() & (CTFB_IntBits | CTFB_FloatBits))
            {
                if (ImGui::InputScalar(label_id.c_str(), MapToImGuiDataType(ECastTypeFlagBits(field_type_ptr->GetCastTypeFlag())), type_instance_ptr))
                {
                    spdlog::info("ImGui::InputInt");
                }
            }
            else if (field_type_ptr->GetCastTypeFlag() & CTFB_StringBit)
            {
                if (ImGui::InputText(label_id.c_str(), (std::string*)type_instance_ptr))
                {
                    spdlog::info("ImGui::InputText");
                }
            }
            else if (field_type_ptr->GetCastTypeFlag() & CTFB_EnumBit)
            {
                Core::Enum* field_enum_ptr = static_cast<Enum*>(field_type_ptr);
                static std::vector<const Core::EnumValue*> enum_value_vector = ([field_enum_ptr]() {
                    auto& enum_value_map = field_enum_ptr->GetEnumValueMap();
                    std::vector<const Core::EnumValue*> enum_value_vector;
                    for (auto enum_value_map_iterator = enum_value_map.begin(); enum_value_map_iterator != enum_value_map.end(); enum_value_map_iterator++)
                    {
                        enum_value_vector.push_back(&(enum_value_map_iterator->second));
                    }
                    return enum_value_vector;
                    })();
                    static int item_current_idx = 0; // Here we store our selection data as an index.
                    struct Funcs {
                        static bool ItemGetter(void* data, int n, const char** out_str) {
                            const Core::EnumValue** enum_values = (const Core::EnumValue**)data;
                            *out_str = enum_values[n]->_DisplayName.c_str();
                            return true;
                        }
                    };
                    if (ImGui::Combo(label_id.c_str(), &item_current_idx, &Funcs::ItemGetter, enum_value_vector.data(), enum_value_vector.size()))
                    {
                        field_enum_ptr->SetEnumValue(type_instance_ptr, enum_value_vector[item_current_idx]->_Value);
                        spdlog::info("ImGui::Combo {:d}", item_current_idx);
                    }
            }
            ImGui::PopItemWidth();
        }

        void DisplaySimpleTypeRow(const std::string& row_name, void* type_instance_ptr, Core::Type* type_ptr)
        {
            using namespace Core;
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            std::string label_id = "##" + row_name;
            ImGui::TreeNodeEx(row_name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
            ImGui::TableNextColumn();
            ImGui::PushItemWidth(-1);
            switch (type_ptr->GetCastTypeFlag())
            {
            case Core::CTFB_BoolBit:
                if (ImGui::Checkbox(label_id.c_str(), (bool*)type_instance_ptr))
                {
                    spdlog::info("ImGui::Checkbox");
                }
                break;
            case Core::CTFB_FloatBit:
            case Core::CTFB_DoubleBit:
            case Core::CTFB_SInt8Bit:
            case Core::CTFB_SInt16Bit:
            case Core::CTFB_SInt32Bit:
            case Core::CTFB_SInt64Bit:
            case Core::CTFB_UInt8Bit:
            case Core::CTFB_UInt16Bit:
            case Core::CTFB_UInt32Bit:
            case Core::CTFB_UInt64Bit:
                if (ImGui::InputScalar(label_id.c_str(), MapToImGuiDataType(ECastTypeFlagBits(type_ptr->GetCastTypeFlag())), type_instance_ptr))
                {
                    spdlog::info("ImGui::InputInt");
                }
                break;
            case Core::CTFB_StringBit:
                if (ImGui::InputText(label_id.c_str(), (std::string*)type_instance_ptr))
                {
                    spdlog::info("ImGui::InputText");
                }
                break;
            case Core::CTFB_EnumBit:
            {
                Core::Enum* field_enum_ptr = static_cast<Enum*>(type_ptr);
                static std::vector<const Core::EnumValue*> enum_value_vector = ([field_enum_ptr]() {
                    auto& enum_value_map = field_enum_ptr->GetEnumValueMap();
                    std::vector<const Core::EnumValue*> enum_value_vector;
                    for (auto enum_value_map_iterator = enum_value_map.begin(); enum_value_map_iterator != enum_value_map.end(); enum_value_map_iterator++)
                    {
                        enum_value_vector.push_back(&(enum_value_map_iterator->second));
                    }
                    return enum_value_vector;
                    })();
                static int item_current_idx = 0; // Here we store our selection data as an index.
                struct Funcs {
                    static bool ItemGetter(void* data, int n, const char** out_str) {
                        const Core::EnumValue** enum_values = (const Core::EnumValue**)data;
                        *out_str = enum_values[n]->_DisplayName.c_str();
                        return true;
                    }
                };
                if (ImGui::Combo(label_id.c_str(), &item_current_idx, &Funcs::ItemGetter, enum_value_vector.data(), enum_value_vector.size()))
                {
                    field_enum_ptr->SetEnumValue(type_instance_ptr, enum_value_vector[item_current_idx]->_Value);
                    spdlog::info("ImGui::Combo {:d}", item_current_idx);
                }
            }
            break;
            default:
                break;
            }
            ImGui::PopItemWidth();
            ImGui::TableNextColumn();
        }

        void DisplayClassInstanceRow(const std::string& row_name, void* type_instance_ptr, Core::Type* type_ptr)
        {
            using namespace Core;
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            bool open = ImGui::TreeNodeEx(row_name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth);
            ImGui::TableNextColumn();
            ImGui::Text("%s[0x%x]", type_ptr->GetName().c_str(), type_instance_ptr);
            ImGui::TableNextColumn();
            if (ImGui::Button("r"))
            {

            }
            if (open)
            {
                Class* instance_class_ptr = static_cast<Class*>(type_ptr);
                auto& instance_fields = instance_class_ptr->GetFields();
                for (size_t i = 0; i < instance_fields.size(); i++)
                {
                    Field* instance_field_ptr = instance_fields[i].get();
                    ShowFieldTableRow(type_instance_ptr, instance_field_ptr);
                }
                ImGui::TreePop();
            }
        }

        void DisplayVectorRow(const std::string& row_name, void* type_instance_ptr, Core::Type* type_ptr)
        {
            using namespace Core;
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            auto& any_vector = *static_cast<std::vector<std::any>*>(type_instance_ptr);
            bool open = ImGui::TreeNodeEx(row_name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth);
            ImGui::TableNextColumn();
            //auto& any_vector = *field_ptr->GetFieldDataPtrAs<std::vector<std::any>>(instance_ptr);
            {
                static int selected_index = 0;
                static Type* selected_type = nullptr;
                static std::vector<Type*> cpp_base_types = { GetType<bool>(), GetType<int>() ,GetType<float>() };
                if (ImGui::BeginCombo(row_name.c_str(), selected_type ? selected_type->GetName().c_str() : ""))
                {
                    int n = 0;
                    for (size_t i = 0; i < cpp_base_types.size(); i++)
                    {
                        const bool is_selected = (selected_index == n);
                        if (ImGui::Selectable(cpp_base_types[i]->GetName().c_str(), is_selected))
                        {
                            selected_index = n;
                        }

                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                        n++;
                    }
                    ImGui::EndCombo();
                }
            }
            ImGui::TableNextColumn();
            if (ImGui::Button("r"))
            {
                any_vector.push_back(1);
            }
            if (open)
            {
                for (size_t i = 0; i < any_vector.size(); i++)
                {
                    ImGui::PushID(i);
                    DisplayAnyRow(std::to_string(i), any_vector[i]);
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
        }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
        void DisplayAnyRow(const std::string& row_name, std::any& any_ref)
        {
            using namespace Core;
            if (any_ref.has_value())
            {
                Type* type_ptr = GetType(any_ref.type());
                if (type_ptr)
                {
                    void* data_ptr = type_ptr->GetAny(any_ref);
                    DisplayRow(row_name, data_ptr, type_ptr);
                }
                else
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TreeNodeEx(row_name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Nontype");
                    ImGui::TableNextColumn();
                }
            }
            else
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TreeNodeEx(row_name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("None");
                ImGui::TableNextColumn();
            }
        }

        void DisplayRow(const std::string& row_name, void* type_instance_ptr, Core::Type* type_ptr)
        {
            using namespace Core;
            ECastTypeFlagBits cast_type_flag = ECastTypeFlagBits(type_ptr->GetCastTypeFlag());
            switch (cast_type_flag)
            {
            case Core::CTFB_NoFlag:
                break;
            case Core::CTFB_VoidBit:
                break;
            case Core::CTFB_BoolBit:
            case Core::CTFB_FloatBit:
            case Core::CTFB_DoubleBit:
            case Core::CTFB_SInt8Bit:
            case Core::CTFB_SInt16Bit:
            case Core::CTFB_SInt32Bit:
            case Core::CTFB_SInt64Bit:
            case Core::CTFB_UInt8Bit:
            case Core::CTFB_UInt16Bit:
            case Core::CTFB_UInt32Bit:
            case Core::CTFB_UInt64Bit:
            case Core::CTFB_StringBit:
            case Core::CTFB_EnumBit:
                DisplaySimpleTypeRow(row_name, type_instance_ptr, type_ptr);
                break;
            case Core::CTFB_ClassBit:
                DisplayClassInstanceRow(row_name, type_instance_ptr, type_ptr);
                break;
            case Core::CTFB_VectorBit:
                DisplayVectorRow(row_name, type_instance_ptr, type_ptr);
                break;
            case Core::CTFB_PtrMapBit:
                break;
            case Core::CTFB_Int64MapBit:
                break;
            case Core::CTFB_StringMapBit:
                break;
            case Core::CTFB_PtrUnorderedMapBit:
                break;
            case Core::CTFB_Int64UnorderedMapBit:
                break;
            case Core::CTFB_StringUnorderedMapBit:
                break;
            case Core::CTFB_ReferenceWrapperBit:
                break;
            case Core::CTFB_PointerBit:
                break;
            default:
                break;
            }
        }

        void ShowFieldTableRow(void* instance_ptr, Core::Field* field_ptr)
        {
            using namespace Core;
            std::string field_name = field_ptr->GetName();
            void* field_data_ptr = field_ptr->GetFieldDataPtr(instance_ptr);
            ECastTypeFlagBits cast_type_flag = ECastTypeFlagBits(field_ptr->GetCastTypeFlag());
            DisplayRow(field_name, field_data_ptr, field_ptr->GetType());
            return;
            switch (cast_type_flag)
            {
            case Core::CTFB_NoFlag:
                break;
            case Core::CTFB_VoidBit:
                break;
            case Core::CTFB_BoolBit:
            case Core::CTFB_FloatBit:
            case Core::CTFB_DoubleBit:
            case Core::CTFB_SInt8Bit:
            case Core::CTFB_SInt16Bit:
            case Core::CTFB_SInt32Bit:
            case Core::CTFB_SInt64Bit:
            case Core::CTFB_UInt8Bit:
            case Core::CTFB_UInt16Bit:
            case Core::CTFB_UInt32Bit:
            case Core::CTFB_UInt64Bit:
            case Core::CTFB_StringBit:
            case Core::CTFB_EnumBit:
                DisplaySimpleTypeRow(field_name, field_data_ptr, field_ptr->GetType());
                break;
            case Core::CTFB_ClassBit:
                DisplayClassInstanceRow(field_name, field_data_ptr, field_ptr->GetType());
            break;
            case Core::CTFB_VectorBit:
                DisplayVectorRow(field_name, field_data_ptr, field_ptr->GetType());
                break;
            case Core::CTFB_PtrMapBit:
                break;
            case Core::CTFB_Int64MapBit:
                break;
            case Core::CTFB_StringMapBit:
                break;
            case Core::CTFB_PtrUnorderedMapBit:
                break;
            case Core::CTFB_Int64UnorderedMapBit:
                break;
            case Core::CTFB_StringUnorderedMapBit:
                break;
            case Core::CTFB_ReferenceWrapperBit:
                break;
            case Core::CTFB_PointerBit:
                break;
            default:
                break;
            }
        }

        void DrawWidget()
        {
            using namespace Core;
            ImGui::Begin("ObjectBrowser");
            static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
            if (ImGui::BeginTable("field_browser", 3, flags))
            {
                const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
                // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
                ImGui::TableSetupColumn("field", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
                ImGui::TableSetupColumn("modifier", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("action", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 9.0f);
                ImGui::TableHeadersRow();
                if (_Object)
                {
                    Class* object_class_ptr = _Object->GetClass();
                    auto& object_fields = object_class_ptr->GetFields();
                    for (size_t i = 0; i < object_fields.size(); i++)
                    {
                        Field* object_field_ptr = object_fields[i].get(); 
                        ShowFieldTableRow(_Object, object_field_ptr);
                    }
                }
                ImGui::EndTable();
            }
            ImGui::End();
        }

        Core::Object* _Object = nullptr;
    };
}











