//
//  imguiStyling.c
//  
//
//  Created by Fedor Artemenkov on 27.06.2024.
//

#include "imguiStyling.h"
#include <imgui.h>

void setImGuiStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.Colors[ImGuiCol_MenuBarBg]                = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

    style.Colors[ImGuiCol_WindowBg]                 = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_PopupBg]                  = ImVec4(0.15f, 0.15f, 0.15f, 0.95f);
    style.Colors[ImGuiCol_Text]                     = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);

    style.Colors[ImGuiCol_Header]                   = ImVec4(0.20f, 0.21f, 0.21f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered]            = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive]             = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

    style.Colors[ImGuiCol_Button]                   = ImVec4(0.20f, 0.21f, 0.21f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered]            = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive]             = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        
    style.Colors[ImGuiCol_FrameBg]                  = ImVec4(0.20f, 0.21f, 0.21f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]           = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive]            = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        
    style.Colors[ImGuiCol_TitleBg]                  = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]         = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive]            = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    
//    style.Colors[ImGuiCol_DockingPreview]           = ImVec4(0.84f, 0.65f, 0.19f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]                = ImVec4(0.84f, 0.65f, 0.19f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]               = ImVec4(0.84f, 0.65f, 0.19f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive]         = ImVec4(0.84f, 0.65f, 0.19f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]               = ImVec4(0.84f, 0.65f, 0.19f, 1.00f);
    
    // Rounding
    style.WindowRounding    = 4.0;
//    style.ChildRounding     = 4.0;
    style.FrameRounding     = 4.0;
//    style.TabRounding       = 4.0;
//    style.PopupRounding     = 4.0;
    style.GrabRounding      = 3.0;
    
    // Padding
    style.FramePadding = ImVec2(6, 3);
    
    // Size
    style.GrabMinSize = 11.0;
    
    // Show/Hide
//    style.WindowMenuButtonPosition = ImGuiDir_None;
}
