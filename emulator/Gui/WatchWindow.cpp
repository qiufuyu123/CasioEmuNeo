#include "WatchWindow.hpp"
#include "../Chipset//Chipset.hpp"
#include "../Chipset/CPU.hpp"
#include "imgui.h"
#include "../Peripheral/BatteryBackedRAM.hpp"
#include <cstdint>

#include "../Config/Config.hpp"

WatchWindow::WatchWindow(casioemu::Emulator *e){
    emu = e;
    
}

void WatchWindow::Show(){
    ImGui::Begin(EmuGloConfig[UI_REPORT_WINDOW]);
    ImGui::BeginChild("##stack_trace",ImVec2(0,ImGui::GetWindowHeight()/4));
    casioemu::Chipset& chipset = emu->chipset;
    std::string s=chipset.cpu.GetBacktrace();
    ImGui::InputTextMultiline("##as",(char*)s.c_str(),s.size(),ImVec2(ImGui::GetWindowWidth(),0),ImGuiInputTextFlags_ReadOnly);
    ImGui::EndChild();
    ImGui::BeginChild("##reg_trace",ImVec2(0,ImGui::GetWindowHeight()/4));
    ImGui::BeginTable("table", 2);
        ImGuiListClipper clipper;
        clipper.Begin(16+10+2);
        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                if(row>=0 && row<16){
                    ImGui::Text("R%d:",row);
                }else if(row>=16 && row<24){
                    ImGui::Text("ER%d:",row == 16?1:(row-16)*2);
                }else if(row == 24){
                    ImGui::Text("SP:");
                }else if(row == 25){
                    ImGui::Text("LR");
                }else if(row == 26){
                    ImGui::Text("EA");
                }
                ImGui::TableSetColumnIndex(1);
                if(row>=0 && row<16){
                    ImGui::Text("0x%02x",chipset.cpu.reg_r[row]&0x0ff);
                }else if(row>=16 && row<24){
                    int x = (row -16)*2;
                    ImGui::Text("0x%04x",chipset.cpu.reg_r[x+1]<<8|chipset.cpu.reg_r[x]);
                }else if(row == 24){
                    ImGui::Text("0x%04x",chipset.cpu.reg_sp&0xffff);
                }else if(row == 25){
                    ImGui::Text("0x%04x",chipset.cpu.reg_lr&0xffff);
                }else if(row==26){
                    ImGui::Text("0x%04x",chipset.cpu.reg_ea&0xffff);
                }
            }
        }
        ImGui::EndTable();
    
    ImGui::EndChild();
    static int range=64;
    ImGui::BeginChild("##stack_view");
    ImGui::Text(EmuGloConfig[UI_REPORT_RANGE]);
    ImGui::SameLine();
    ImGui::SliderInt(EmuGloConfig[UI_REPORT_RANGE_SLIDER], &range, 64, 2048);
    uint16_t offset = chipset.cpu.reg_sp&0xffff;
    mem_editor.DrawContents(casioemu::BatteryBackedRAM::rom_addr+ offset-0xd000, range,offset);
    ImGui::EndChild();
    ImGui::End();
    
}