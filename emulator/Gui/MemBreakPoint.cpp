#include "MemBreakPoint.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"
#include "../Chipset/CPU.hpp"
#include "imgui.h"
#include <cstdint>
#include <cstdlib>
#include <stdlib.h>

UI_SINGLE_IMPL(MemBreakPoint)

MemBreakPoint::MemBreakPoint(){
    instance = this;
}


void MemBreakPoint::DrawContent(){
    ImGuiListClipper c;
    static int selected = -1;
    c.Begin(break_point_hash.size());
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    char buf[5]={0};
    while (c.Step()) {
        
        for (int i = c.DisplayStart;i<c.DisplayEnd;i++) {
            MemBPData_t &data = break_point_hash[i];
            snprintf(buf, sizeof(buf), "%04x", data.addr);
            ImGui::PushID(i);
            if(ImGui::Selectable(buf,selected == i)){
                selected = i;
            }
            ImGui::PopID();
            if(ImGui::BeginPopupContextItem()){
                selected = i;
                ImGui::Text("请选择断点模式：");
                if(ImGui::Button("查找是什么访问了这个地址")){
                    target_addr = i;
                    data.enableWrite=0;
                    data.records.clear();
                    ImGui::CloseCurrentPopup();
                }
                if(ImGui::Button("查找是什么写入了这个地址")){
                    data.enableWrite = true;
                    target_addr = i;
                    data.records.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::Separator();
                if(ImGui::Button("删除此地址")){
                    data.records.clear();
                    if(target_addr == i){
                        target_addr = -1;
                    }
                    break_point_hash.erase(break_point_hash.begin()+i);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

        }
    }
}

void MemBreakPoint::DrawFindContent(){
    if(target_addr == -1){
        ImGui::TextColored(ImVec4(255,255,0,255), "未设置任何断点，请添加地址->右键地址->选择模式");
        return;
    }
    int write = break_point_hash[target_addr].enableWrite;
    static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    ImGui::Text("正在监听地址:%04x",break_point_hash[target_addr].addr);
    ImGui::SameLine();
    if(ImGui::Button("清除记录")){
        break_point_hash[target_addr].records.clear();
    }
    if(ImGui::BeginTable("##outputtable", 2,flags)){
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("PC: ");
        ImGui::TableSetupColumn("模式: ");
        ImGui::TableHeadersRow();
        for(auto kv : break_point_hash[target_addr].records){
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(ImVec4(0,200,0,200), "%01x:%04x",kv.first>>16, kv.first&0x0ffff);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(write?"写":"读");
        }
        ImGui::EndTable();
    }
    
}

void MemBreakPoint::TryTrigBp(uint16_t addr,bool write){
    if(target_addr == -1){
        return;
    }
    MemBPData_t &bp = break_point_hash.at(target_addr);
    if(bp.addr == addr && bp.enableWrite == write){
        bp.records[(casioemu::Emulator::instance->chipset.cpu.reg_csr<<16 ) 
        | casioemu::Emulator::instance->chipset.cpu.reg_pc] = write;
    }
}

void MemBreakPoint::Show(){
    static char buf[5]={0};
    ImGui::Begin("内存断点");
    ImGui::BeginChild("##srcollingmbp",ImVec2(0,ImGui::GetWindowHeight()/3));
    DrawContent();
    ImGui::EndChild();
    ImGui::SetNextItemWidth(ImGui::CalcTextSize("F").x*4);
    ImGui::InputText("地址:", buf, 5,ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::SameLine();
    if(ImGui::Button("添加地址")){
        break_point_hash.push_back({
            .addr = (uint16_t)strtol(buf, nullptr, 16)
        });
    }
    ImGui::BeginChild("##findoutput");
    DrawFindContent();
    ImGui::EndChild();
    ImGui::End();
}