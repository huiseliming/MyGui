#pragma once
#include "VulkanGuiApplication.h"
#include "Core/Class.h"
#include "Core/Enum.h"

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

    class ObjectBrowserWidget
    {
    public:

        void DrawWidget()
        {
            using namespace Core;
            ImGui::Begin("ObjectBrowser");
            if (_Object)
            {
                ImGui::Columns(2, "tree", true);

                Class* object_class_ptr = _Object->GetClass();
                auto& object_fields = object_class_ptr->GetFields();
                for (size_t i = 0; i < object_fields.size(); i++)
                {
                    Field* object_field_ptr = object_fields[i].get();
                    std::string label_name = object_field_ptr->GetName();
                    ImGui::TextUnformatted(label_name.c_str());
                    ImGui::NextColumn();
                    std::string label_id = "##" + label_name;
                    ImGui::PushItemWidth(-1);
                    if (object_field_ptr->GetCastTypeFlag() & CTFB_BoolBit)
                    {
                        if (ImGui::Checkbox(label_id.c_str(), object_field_ptr->GetFieldDataPtrAs<bool>(_Object)))
                        {
                            spdlog::info("ImGui::Checkbox");
                        }
                        ImGui::NextColumn();
                    }
                    else if (object_field_ptr->GetCastTypeFlag() & (CTFB_IntBits | CTFB_FloatBits))
                    {
                        if (ImGui::InputScalar(label_id.c_str(), MapToImGuiDataType(ECastTypeFlagBits(object_field_ptr->GetCastTypeFlag())), object_field_ptr->GetFieldDataPtr(_Object)))
                        {
                            spdlog::info("ImGui::InputInt");
                        }
                        ImGui::NextColumn();
                    }
                    else if (object_field_ptr->GetCastTypeFlag() & CTFB_StringBit)
                    {
                        if (ImGui::InputText(label_id.c_str(), object_field_ptr->GetFieldDataPtrAs<std::string>(_Object)))
                        {
                            spdlog::info("ImGui::InputText");
                        }
                        ImGui::NextColumn();
                    }
                    else if (object_field_ptr->GetCastTypeFlag() & CTFB_EnumBit)
                    {
                        Core::Enum* field_enum_ptr = static_cast<Enum*>(object_field_ptr->GetType());
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
                            object_field_ptr->SetUInt(_Object, enum_value_vector[item_current_idx]->_Value);
                            spdlog::info("ImGui::Combo {:d}", item_current_idx);
                        }
                        ImGui::NextColumn();
                    }
                    else if (object_field_ptr->GetCastTypeFlag() & Core::ECastTypeFlagBits::CTFB_VectorBit)
                    {
                        //auto& vector = *object_field_ptr->GetFieldDataPtrAs<std::vector<std::any>>(&_Object);
                        //for (size_t i = 0; i < vector.size(); i++)
                        //{
                        //    if (Core::Type* type = Core::GetType(vector[i].type()))
                        //    {
                        //        ImGui::Text(type->GetName().c_str());
                        //    }
                        //    else
                        //    {
                        //        ImGui::Text("Empty");
                        //    }
                        //}
                        //static int vector_size = 0;
                        //if (ImGui::InputInt("vector_size", &vector_size))
                        //{
                        //    test_struct._Vector.resize(vector_size, 1);
                        //}
                    }
                    ImGui::PopItemWidth();
                }
                ImGui::Columns(1);
            }
            ImGui::End();
        }

        Core::Object* _Object = nullptr;
    };
}

class MyApplication : public VulkanGuiApplication
{
    using Super = VulkanGuiApplication;
public:
    virtual int main(const std::vector<std::string>& args) override
    {
        static Core::TestObject test_object;
        GUI::ObjectBrowserWidget object_browser_widget;
        object_browser_widget._Object = &test_object;
        _ImGuiDrawFunctions.insert_or_assign("ObjectBrowserWidget", std::bind(&GUI::ObjectBrowserWidget::DrawWidget, object_browser_widget));
        _ImGuiDrawFunctions.insert_or_assign(
            "draw",
            []()
            {
                static bool show_demo_window = true;
                static bool show_another_window = false;

                ImGui::Begin("TestStruct");
                static Core::TestStruct test_struct;
                Core::Class* reflect_class = Core::TestStruct::StaticClass();
                {
                    Core::Function* function = reflect_class->GetFunctions()[0].get();
                    void* script_struct = function->New();
                    int value_a = 5;
                    int* value_a_ptr = &value_a;
                    {
                        Core::Field* field = function->GetFields()[0].get();
                        std::reference_wrapper<int*>* data_ptr = field->GetFieldDataPtrAs<std::reference_wrapper<int*>>(script_struct);
                        *data_ptr = value_a_ptr;
                    }
                    {
                        Core::Field* field = function->GetFields()[1].get();
                        std::reference_wrapper<int >* data_ptr = field->GetFieldDataPtrAs<std::reference_wrapper<int>>(script_struct);
                        *data_ptr = value_a;
                    }
                    function->GetVMCallFuncPtr()(&test_struct, script_struct);
                    std::reference_wrapper<int >* data_ptr = function->GetFields().back()->GetFieldDataPtrAs<std::reference_wrapper<int>>(script_struct);
                    int c = data_ptr->get();
                    ImGui::Text("%d", c);
                    auto obejct_native_call = *function->GetNativeCallFuncPtrAs<decltype(&Core::TestStruct::TestAdd)>();
                    int d = (test_struct.*obejct_native_call)(value_a_ptr, c);
                    ImGui::Text("%d", d);
                }
                auto& reflect_class_fields = reflect_class->GetFields();
                for (size_t i = 0; i < reflect_class_fields.size(); i++)
                {
                    Core::Field* field = reflect_class_fields[i].get();
                    std::string label_name = "TestStruct::" + field->GetName();

                    if (field->GetCastTypeFlag() & Core::ECastTypeFlagBits::CTFB_BoolBit)
                    {
                        if (ImGui::Checkbox(label_name.c_str(), field->GetFieldDataPtrAs<bool>(&test_struct)))
                        {
                            spdlog::info("ImGui::Checkbox");
                        }
                    }
                    else if (field->GetCastTypeFlag() & Core::ECastTypeFlagBits::CTFB_SInt32Bit)
                    {
                        if (ImGui::InputInt(label_name.c_str(), field->GetFieldDataPtrAs<int>(&test_struct)))
                        {
                            spdlog::info("ImGui::InputInt");
                        }
                    }
                    else if (field->GetCastTypeFlag() & Core::ECastTypeFlagBits::CTFB_VectorBit)
                    {
                        auto& vector = *field->GetFieldDataPtrAs<std::vector<std::any>>(&test_struct);
                        for (size_t i = 0; i < vector.size(); i++)
                        {
                            if (Core::Type* type = Core::GetType(vector[i].type()))
                            {
                                ImGui::Text(type->GetName().c_str());
                            }
                            else
                            {
                                ImGui::Text("Empty");
                            }
                        }
                        static int vector_size = 0;
                        if (ImGui::InputInt("vector_size", &vector_size))
                        {
                            test_struct._Vector.resize(vector_size, 1);
                        }
                    }
                }
                static Core::ETestEnum test_enum;
                Core::Enum* reflect_enum = Core::GetStaticEnum<Core::ETestEnum>();
                auto& enum_value_map = reflect_enum->GetEnumValueMap();
                std::vector<const Core::EnumValue*> enum_value_vector;
                for (auto enum_value_map_iterator = enum_value_map.begin(); enum_value_map_iterator != enum_value_map.end(); enum_value_map_iterator++)
                {
                    enum_value_vector.push_back(&(enum_value_map_iterator->second));
                }
                static int item_current_idx = 0; // Here we store our selection data as an index.
                struct Funcs {
                    static bool ItemGetter(void* data, int n, const char** out_str) {
                        const Core::EnumValue** enum_values = (const Core::EnumValue**)data;
                        *out_str = enum_values[n]->_CppName.c_str();
                        return true;
                    }
                };
                if (ImGui::Combo("combo 4 (function)", &item_current_idx, &Funcs::ItemGetter, enum_value_vector.data(), enum_value_vector.size()))
                {
                    spdlog::info("ImGui::Combo {:d}", item_current_idx);
                }
                ImGui::End();


                if (show_demo_window)
                    ImGui::ShowDemoWindow(&show_demo_window);
                // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
                {
                    static float f = 0.0f;
                    static int counter = 0;
                    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
                    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Another Window", &show_another_window);

                    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                    if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                        counter++;
                    ImGui::SameLine();
                    ImGui::Text("counter = %d", counter);

                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                    ImGui::End();
                }

                // 3. Show another simple window.
                if (show_another_window)
                {
                    ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                    ImGui::Text("Hello from another window!");
                    if (ImGui::Button("Close Me"))
                        show_another_window = false;
                    ImGui::End();
                }
            });
        return Super::main(args);
    }
private:
};

POCO_APP_MAIN(MyApplication);