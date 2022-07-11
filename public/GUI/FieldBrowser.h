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
        void DisplayField(const std::string& field_name, void* type_instance_ptr, Core::Type* type_ptr)
        {
            using namespace Core;
            std::string label = field_name + "##label";
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGuiTreeNodeFlags tree_node_flag = ImGuiTreeNodeFlags_SpanFullWidth;
            uint32_t target_cast_type_flag = type_ptr->GetCastTypeFlag();
            Type* target_type_ptr = type_ptr;
            void* target_type_instance_ptr = type_instance_ptr;
            if (type_ptr->GetCastTypeFlag() & CTFB_AnyBit)
            {
                std::any* any_ptr = static_cast<std::any*>(type_instance_ptr);
                if (any_ptr->has_value())
                {
                    Type* any_type_ptr = GetType(any_ptr->type());
                    target_cast_type_flag = any_type_ptr->GetCastTypeFlag();
                    target_type_ptr = any_type_ptr;
                    target_type_instance_ptr = any_type_ptr->GetAny(*any_ptr);
                }
                else
                {
                    target_cast_type_flag = CTFB_VoidBit;
                }
            }
            if (target_cast_type_flag & (CTFB_VoidBit | CTFB_IntBits | CTFB_FloatBits | CTFB_BoolBit | CTFB_StringBit | CTFB_EnumBit))
            {
                tree_node_flag |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            }

            bool tree_opened = ImGui::TreeNodeEx(label.c_str(), tree_node_flag);
            ImGui::TableNextColumn();
            // display value
            DisplayFieldValue(type_instance_ptr, type_ptr);
            ImGui::TableNextColumn();
            if (type_ptr == GetType<std::any>())
            {
                std::any* any_ptr = static_cast<std::any*>(type_instance_ptr);
                Type* any_type_ptr = GetType(any_ptr->type());
                Type* new_selected_any_type_ptr = any_type_ptr;
                TypeSelectorCombox(new_selected_any_type_ptr);
                if (new_selected_any_type_ptr != any_type_ptr)
                {
                    if (new_selected_any_type_ptr)
                    {
                        new_selected_any_type_ptr->SetAny(*any_ptr, new_selected_any_type_ptr->GetDefault());
                    }
                    else
                    {
                        any_ptr->reset();
                    }
                }
            }
            else
            {
                ImGui::TextUnformatted(type_ptr->GetName().c_str());
            }
            if (tree_opened && !(tree_node_flag & ImGuiTreeNodeFlags_NoTreePushOnOpen))
            {
                if (type_ptr->GetCastTypeFlag() & CTFB_AnyBit)
                {
                    std::any* any_ptr = static_cast<std::any*>(type_instance_ptr);
                    if (any_ptr->has_value())
                    {
                        Type* any_type_ptr = GetType(any_ptr->type());
                        target_cast_type_flag = any_type_ptr->GetCastTypeFlag();
                        target_type_ptr = any_type_ptr;
                        target_type_instance_ptr = any_type_ptr->GetAny(*any_ptr);
                    }
                    else
                    {
                        target_cast_type_flag = CTFB_VoidBit;
                    }
                }
                switch (target_cast_type_flag)
                {
                case CTFB_ClassBit:
                {
                    Class* instance_class_ptr = static_cast<Class*>(target_type_ptr);
                    auto& instance_fields = instance_class_ptr->GetFields();
                    for (size_t i = 0; i < instance_fields.size(); i++)
                    {
                        ImGui::PushID(i);
                        Field* instance_field_ptr = instance_fields[i].get();
                        DisplayField(instance_field_ptr->GetName(), instance_field_ptr->GetFieldDataPtr(target_type_instance_ptr), instance_field_ptr->GetType());
                        ImGui::PopID();
                    }
                }
                    break;
                case CTFB_VectorBit:
                {
                    auto& any_vector = *static_cast<std::vector<std::any>*>(target_type_instance_ptr);
                    for (size_t i = 0; i < any_vector.size(); i++)
                    {
                        ImGui::PushID(i);
                        DisplayField(std::to_string(i), &any_vector[i], GetType<std::any>());
                        ImGui::PopID();
                    }
                }
                    break;

                default:
                    break;
                }
                ImGui::TreePop();
            }
        }

        void DisplayFieldValue(void* type_instance_ptr, Core::Type* type_ptr)
        {
            using namespace Core;
            ImGui::PushItemWidth(-1);
            switch (type_ptr->GetCastTypeFlag())
            {
            case Core::CTFB_BoolBit:
                if (ImGui::Checkbox("field_value", (bool*)type_instance_ptr))
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
                if (ImGui::InputScalar("field_value", MapToImGuiDataType(ECastTypeFlagBits(type_ptr->GetCastTypeFlag())), type_instance_ptr))
                {
                    spdlog::info("ImGui::InputInt");
                }
                break;
            case Core::CTFB_StringBit:
                if (ImGui::InputText("field_value", (std::string*)type_instance_ptr))
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
                    if (ImGui::Combo("field_value", &item_current_idx, &Funcs::ItemGetter, enum_value_vector.data(), enum_value_vector.size()))
                    {
                        field_enum_ptr->SetEnumValue(type_instance_ptr, enum_value_vector[item_current_idx]->_Value);
                        spdlog::info("ImGui::Combo {:d}", item_current_idx);
                    }
            }
            break;
            case Core::CTFB_ClassBit:
                ImGui::Text("%s[0x%x]", type_ptr->GetName().c_str(), type_instance_ptr);
                break;
            case  Core::CTFB_AnyBit:
            {
                std::any* any_ptr = static_cast<std::any*>(type_instance_ptr);
                Type* any_type_ptr = GetType(any_ptr->type());
                if (any_ptr->has_value())
                {
                    DisplayFieldValue(any_type_ptr->GetAny(*any_ptr), any_type_ptr);
                }
                else
                {
                    ImGui::Text("std::any[empty]");
                }
            }
                break;
            case Core::CTFB_VectorBit:
            {
                auto vector_ref = static_cast<std::vector<std::any>*>(type_instance_ptr);
                int vector_size = vector_ref->size();
                if (ImGui::InputInt("field_value", &vector_size))
                {
                    vector_ref->resize(vector_size);
                }
                break;
            }
            default:
                break;
            }
            ImGui::PopItemWidth();
        }

        void TypeSelectorCombox(Core::Type*& selected_type)
        {
            using namespace Core;
            ImGui::PushItemWidth(-1);
            bool select_changed = ImGui::BeginCombo("class_selector", selected_type ? selected_type->GetName().c_str() : "");
            if (select_changed)
            {
                static std::vector<Type*> cpp_base_types = { GetType<bool>(), GetType<int>() ,GetType<float>() };
                static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
                for (size_t i = 0; i < cpp_base_types.size(); i++)
                {
                    ImGuiTreeNodeFlags node_flag = ImGuiTreeNodeFlags_FramePadding;
                    node_flag |= ImGuiTreeNodeFlags_Bullet;
                    bool is_expanded = ImGui::TreeNodeExV((void*)cpp_base_types[i], node_flag, "", nullptr);
                    ImGui::SameLine();
                    const bool is_selected = (cpp_base_types[i] == selected_type);
                    if (ImGui::Selectable(cpp_base_types[i]->GetName().c_str(), is_selected))
                    {
                        selected_type = cpp_base_types[i];
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                    if (is_expanded)
                        ImGui::TreePop();
                }
                for (auto enum_name_map_iterator : global_enum_name_map_ref)
                {
                    ImGuiTreeNodeFlags node_flag = ImGuiTreeNodeFlags_FramePadding;
                    node_flag |= ImGuiTreeNodeFlags_Bullet;
                    bool is_expanded = ImGui::TreeNodeExV((void*)enum_name_map_iterator.second, node_flag, "", nullptr);
                    ImGui::SameLine();
                    const bool is_selected = (enum_name_map_iterator.second == selected_type);
                    if (ImGui::Selectable(enum_name_map_iterator.second->GetName().c_str(), is_selected))
                    {
                        selected_type = enum_name_map_iterator.second;
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                    if (is_expanded)
                        ImGui::TreePop();
                }
                struct ClassTree {
                    static void MakeNode(Class* in_class, Core::Type*& selected_type)
                    {
                        if (!in_class->IsCopyAssignable()) return;
                        ImGuiTreeNodeFlags node_flag = ImGuiTreeNodeFlags_FramePadding;
                        bool exist_child_classes = !in_class->GetChildClasses().empty();
                        if (!exist_child_classes)
                        {
                            node_flag |= ImGuiTreeNodeFlags_Bullet;
                        }
                        bool is_expanded = ImGui::TreeNodeExV((void*)in_class, node_flag, "", nullptr);
                        ImGui::SameLine();
                        const bool is_selected = (in_class == selected_type);
                        if (ImGui::Selectable(in_class->GetName().c_str(), is_selected))
                        {
                            selected_type = in_class;
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                        if (is_expanded) {
                            if (!in_class->GetChildClasses().empty())
                            {
                                auto& child_classes = in_class->GetChildClasses();
                                for (size_t i = 0; i < child_classes.size(); i++)
                                {
                                    Class* child_class = child_classes[i];
                                    MakeNode(child_class, selected_type);
                                }
                            }
                            ImGui::TreePop();
                        }
                    }
                };
                for (auto class_ptr : GetRootClasses())
                {
                    ClassTree::MakeNode(class_ptr, selected_type);
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
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
                ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 9.0f);
                ImGui::TableHeadersRow();
                if (_Object)
                {
                    Class* object_class_ptr = _Object->GetClass();
                    auto& object_fields = object_class_ptr->GetFields();
                    for (size_t i = 0; i < object_fields.size(); i++)
                    {
                        ImGui::PushID(i);
                        Field* object_field_ptr = object_fields[i].get();
                        DisplayField(object_field_ptr->GetName(), object_field_ptr->GetFieldDataPtr(_Object), object_field_ptr->GetType());
                        //ShowFieldTableRow(_Object, object_field_ptr);
                        ImGui::PopID();
                    }
                }
                ImGui::EndTable();
            }
            ImGui::End();
        }

        Core::Object* _Object = nullptr;
    };
}











