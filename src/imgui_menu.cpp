#include "nes_types.h"
#include "nes.h"
#include "win32_nes.h"

#include <Windows.h>
#include "../vendor/imgui/imgui.h"

#define internal static

enum MENU_REQUEST
{
    kNone,
    kLoadRom,
    kQuit
};

internal MENU_REQUEST
RenderMainMenu(NesCode *nesCode)
{
    MENU_REQUEST Result = kNone;

    if(ImGui::BeginMainMenuBar())
    {
        if( ImGui::BeginMenu("File") )
        {
            if( ImGui::MenuItem("Load ROM...") )
            {
                Result = kLoadRom;
            }
            if( ImGui::MenuItem("Quit", "Alt + F4") )
            {
                Result = kQuit;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    return(Result);
}