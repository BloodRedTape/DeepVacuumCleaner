#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include <vector>
#include <string>

namespace ImGui {

inline bool SimpleCombo(const char *name, std::size_t *select, const std::vector<std::string> &options){
    if(!select || *select >= options.size()){
        return false;
    }
    
    bool changed = false;
    if (ImGui::BeginCombo(name, options[*select].c_str())) {
        
        for(int i = 0; i<options.size(); i++){
            bool is_selected = i == *select;
            if(ImGui::Selectable(options[i].c_str(), &is_selected)) {
                changed = true;
                *select = i;
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    return changed;
}

inline bool SimpleCombo(const char* name, bool* select, const char *true_option, const char *false_option) {
    bool changed = false;
    if (ImGui::BeginCombo(name, *select ? true_option : false_option)) {
        bool is_false = !*select;
        if(ImGui::Selectable(false_option, &is_false)){
            changed = true;
            *select = false;
        }

        if(ImGui::Selectable(true_option, select)){
            changed = true;
            *select = true;
        }
        ImGui::EndCombo();
    }
    return changed;
}

inline bool InputText(const char* label, std::string& buffer, ImGuiInputTextFlags flags = 0) {
    return ImGui::InputText(label, buffer.data(), buffer.size() + 1, flags | ImGuiInputTextFlags_CallbackResize, [](ImGuiInputTextCallbackData *data)->int {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
        {
            std::string* str = (std::string*)data->UserData;
            str->resize(data->BufTextLen);
            data->Buf = str->data();
        }
        return 0;
    }, &buffer);
}

}