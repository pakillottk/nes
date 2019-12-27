#include "nes_types.h"
#include "nes.h"
#include "win32_nes.h"

#include <Windows.h>
#include "../vendor/imgui/imgui.h"

#define internal static
#define local_persist static

enum MENU_REQUEST
{
    kNone,
    kLoadRom,
    kQuit
};

internal MENU_REQUEST
RenderMainMenu()
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

internal const char*
AddrModeToStr(ADDR_MODE mode)
{
    local_persist const char *addr_mode_str[13] = 
    {
        "ACCUMULATOR",
        "ABS",
        "ABS_X",
        "ABS_Y",
        "IMMEDIATE",
        "IMPL", 
        "IND", 
        "X_IND",
        "IND_Y"
        "REL", 
        "ZPG", 
        "ZPG_X",
        "ZPG_Y"
    };

    return addr_mode_str[mode];
}

internal void
DisplayInstruction(Instruction *ins)
{
    ImGui::Text("0x%x", ins->opcode);
    ImGui::NextColumn();
    ImGui::Text("%s", ins->label);
    ImGui::NextColumn();
    ImGui::Text("%s", AddrModeToStr(ins->addr_mode));
    ImGui::NextColumn();
    ImGui::Text("%d", ins->cycles);
    ImGui::NextColumn();
}

internal void
RenderState(NESContext *nesContext)
{
    ImGui::Begin("Debugger");
        ImGui::Text("Total cycles: %d", nesContext->totalCycles);        
        ImGui::SameLine();
        ImGui::Text("Frame cycles: %d", nesContext->deltaCycles);

        ImGui::Separator();

        if(ImGui::Button("Run"))
        {
            nesContext->runMode = kRun;
        }
        ImGui::SameLine();
        if(ImGui::Button("Pause"))
        {
            nesContext->runMode = kPause;
        }
        ImGui::SameLine();
        if(ImGui::Button("Step"))
        {
            nesContext->runMode = kStep;
        }

        ImGui::Separator();

        ImGui::Text("PC: 0x%x", nesContext->nes.cpu.PC);        
        ImGui::Text("SP: 0x%x", nesContext->nes.cpu.SP);        

        ImGui::Columns(3, "cpu_regs_state");
        ImGui::Text("A: 0x%x", nesContext->nes.cpu.regs.A);
        ImGui::NextColumn();
        ImGui::Text("X: 0x%x", nesContext->nes.cpu.regs.X);
        ImGui::NextColumn();
        ImGui::Text("Y: 0x%x", nesContext->nes.cpu.regs.Y);
        ImGui::NextColumn();
        
        ImGui::Columns(1);

        ImGui::Columns(6);
        ImGui::Text("C: %d", nesContext->nes.cpu.P.C);
        ImGui::NextColumn();
        ImGui::Text("Z: %d", nesContext->nes.cpu.P.Z);
        ImGui::NextColumn();
        ImGui::Text("I: %d", nesContext->nes.cpu.P.I);
        ImGui::NextColumn();
        ImGui::Text("D: %d", nesContext->nes.cpu.P.D);
        ImGui::NextColumn();
        ImGui::Text("V: %d", nesContext->nes.cpu.P.V);
        ImGui::NextColumn();
        ImGui::Text("N: %d", nesContext->nes.cpu.P.N);
        ImGui::NextColumn();

        ImGui::Columns(1);

        ImGui::Separator();

        ImGui::Columns(4, "ins_table_header");

        ImGui::Text("Op Code");
        ImGui::NextColumn();

        ImGui::Text("Instruction");
        ImGui::NextColumn();

        ImGui::Text("Address Mode");
        ImGui::NextColumn();

        ImGui::Text("Cycles");
        ImGui::NextColumn();

        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::BeginChild("Instructions", ImVec2(0,0), false);
            ImGui::Columns(4, "ins_table_rows", true);

            if( nesContext->instructionQueueOverflow )
            {
                for( i32 i = 0; i < INSTRUCTION_BUFFER_SIZE; ++i )
                {                
                    DisplayInstruction(&nesContext->processedInstructions[ (nesContext->lastInstructionCursor - i - 1) % INSTRUCTION_BUFFER_SIZE ]);
                }
            }
            else
            {
                for( i32 i = nesContext->lastInstructionCursor - 1; i >= 0; --i )
                {                
                    DisplayInstruction(&nesContext->processedInstructions[i]);
                }
            }

            ImGui::Columns(1);
        ImGui::EndChild();
    ImGui::End();
}