#pragma once
#include "VulkanGuiApplication.h"
#include "Core/Class.h"
#include "Core/Enum.h"


class MyApplication : public VulkanGuiApplication
{
    using Super = VulkanGuiApplication;
public:
    virtual int main(const std::vector<std::string>& args) override
    {
        class D : public GuiDrawObject
        {
            virtual void Draw()
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
            }

        };
        _Drawables.push_back(std::make_shared<D>());
        return Super::main(args);
    }
private:
};

POCO_APP_MAIN(MyApplication);