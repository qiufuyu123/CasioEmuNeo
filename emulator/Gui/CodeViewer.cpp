#include "CodeViewer.hpp"
#include "../Config.hpp"
#include "../Logger.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"
#include "../Chipset/CPU.hpp"
#include "imgui.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ios>
#include <iostream>
#include <ostream>
#include <string>
#include <fstream>
#include <thread>

#include "../Config/Config.hpp"

std::unordered_map<int, uint8_t> DebugBreakPoints;

int get_real_pc(uint8_t seg,uint16_t off){
    return (seg<<16)|off;
}

CodeViewer::CodeViewer(std::string path,casioemu::Emulator *e){
    src_path = path;
    emu = e;
    std::thread t1([this](){
        std::ifstream f(src_path,std::ios::in);
    if(!f.is_open()){
        PANIC("\nFail to open disassembly code src: %s\n",src_path.c_str());
    }
    casioemu::logger::Info("Start to read code src ...\n");
    char buf[200]{0};
    char adr[6]{0};
    while(!f.eof()){
        f.getline(buf,200);
        // 1sf, extract segment number
        uint8_t seg = buf[1] - '0';
        uint8_t len = strlen(buf);
        if(!len)
            break;
        if(len>max_col)
            max_col = len;
        memcpy(adr, buf+2, 4);
        //casioemu::logger::Info("[%s %d %d]\n",adr,seg,len);
        uint16_t offset = std::stoi( adr,0,16);
        CodeElem e;
        e.offset =offset;
        e.segment =seg;
        memset(e.srcbuf, 0, 40);
        memcpy(e.srcbuf, buf+28, len-28);
        codes.push_back(e);
        memset(buf, 0, 200);
        memset(adr, 0, 6);
    }
    f.close();
    casioemu::logger::Info("Read src codes over!\n");
    max_row = codes.size();
    is_loaded=true;
    });
    t1.detach();
}
bool elem_cmp(const CodeElem& a, const CodeElem& b) {

    return get_real_pc(a.segment, a.offset)<get_real_pc(b.segment, b.offset);
}

CodeViewer::~CodeViewer(){

}

CodeElem CodeViewer::LookUp(uint8_t seg,uint16_t offset,int *idx){
    // binary search
    //
    CodeElem target;
    target.offset = offset;
    target.segment = seg;
    auto it= std::lower_bound(codes.begin(),codes.end(),target,elem_cmp);
    if(it==codes.end()){
        it = codes.begin();
        //casioemu::logger::Info("fFound element ! at: %d seg, %x off %s\n",it->segment,it->offset,it->srcbuf);
    }
    if(idx)
        *idx = it-codes.begin();
    return {.segment=it->segment,.offset=it->offset};
}

bool CodeViewer::TryTrigBP(uint8_t seg,uint16_t offset,bool bp_mode){
    int realpc = get_real_pc(seg, offset);
    auto it = DebugBreakPoints.find(realpc);
    if(it != DebugBreakPoints.end() && it->second==1){
        //TODO: We ignore a second trigger
        (*it).second = 2;
        int idx=0;
        LookUp(seg, offset,&idx);
        cur_col = idx;
        need_roll=true;
        cur_break_real_pc = realpc;
        return true;
    }
    if( !bp_mode &&( debug_flags & DEBUG_STEP || debug_flags & DEBUG_RET_TRACE)){
        int idx=0;
        LookUp(seg, offset,&idx);
        DebugBreakPoints[realpc] = 2;
        cur_col=idx;
        need_roll=true;
        cur_break_real_pc = realpc;
        return true;
    }
    return false;
}

void CodeViewer::DrawContent(){
    
    ImGuiListClipper c;
    c.Begin(max_row,ImGui::GetTextLineHeight());
    ImDrawList *draw_list =ImGui::GetWindowDrawList();
    // if(cur_break_real_pc!=-1){
    //     c.IncludeItemByIndex(cur_col);
    // }
    while(c.Step())
    {
        for (int line_i=c.DisplayStart; line_i<c.DisplayEnd; line_i++) {
            CodeElem e=codes[line_i];
            int realpc = get_real_pc(e.segment, e.offset);
            auto it = DebugBreakPoints.find(realpc);
            if(it!= DebugBreakPoints.end()&&it->second == 1){
                ImGui::TextColored(ImVec4(1.0,0.0,0.0,1.0), "[ x ]");
                if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)){
                    DebugBreakPoints.erase(realpc);
                }
            }else if(realpc != cur_break_real_pc){
                ImGui::Text("[ o ]");
                if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)){
                    DebugBreakPoints[realpc] = 1;
                }
            }
            if(realpc == cur_break_real_pc){
                
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImGui::GetWindowDrawList()
                    ->AddRectFilled(pos, ImVec2(pos.x + ImGui::GetWindowWidth(),pos.y +ImGui::GetTextLineHeight()),IM_COL32(255,255,0,50));
                ImGui::TextColored(ImVec4(0.0,1.0,0.0,1.0),"[ > ]");
                if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)){
                    DebugBreakPoints.erase(realpc);
                }
            }
        
            ImGui::SameLine();
            
            ImGui::TextColored(ImVec4(1.0,1.0,0.0,1.0), "%d:%04x",e.segment,e.offset);
            ImGui::SameLine();
            ImGui::Text("%s",e.srcbuf);

        }
    }
    c.End();
    if(need_roll){
        float v=(float)cur_col/max_row*ImGui::GetScrollMaxY();
        ImGui::SetScrollY(v);
        need_roll = false;
        selected_addr = codes[cur_col].segment*0x10000+codes[cur_col].offset;
    }
    
}

void CodeViewer::DrawMonitor(){
    
}

static bool step_debug=false,trace_debug=false;

void CodeViewer::DrawWindow(){

    int h = ImGui::GetTextLineHeight()+4;
    int w = ImGui::CalcTextSize("F").x;
    if(!is_loaded){
        ImGui::SetNextWindowSize(ImVec2(w*50,h*10));
        ImGui::SetNextWindowContentSize(ImVec2(w*50,h*10));
        ImGui::Begin(EmuGloConfig[UI_DISAS]);
        ImGui::SetCursorPos(ImVec2(w*2,h*5));
        ImGui::Text(EmuGloConfig[UI_DISAS_WAITING]);
        ImGui::End();
        return;
    }
    ImVec2 sz;
    h*=10;
    w*=max_col;
    sz.x = w;
    sz.y = h;
    //ImGui::SetNextWindowSize(sz);
    //ImGui::SetNextWindowContentSize(sz);
    ImGui::Begin(EmuGloConfig[UI_DISAS],0);
    ImGui::BeginChild("##scrolling",ImVec2(0,-ImGui::GetWindowHeight()/3));
    DrawContent();
    ImGui::EndChild();
    ImGui::Separator();
    ImGui::Text(EmuGloConfig[UI_DISAS_GOTO_ADDR]);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::CalcTextSize("000000").x);
    ImGui::InputText("##input", adrbuf, 8);
    if(adrbuf[0]!='\0' && ImGui::IsItemFocused()){
        uint32_t addr = std::stoi(adrbuf,0,16);
        JumpTo(addr>>16, addr&0x0ffff);
    }
    ImGui::SameLine();
    ImGui::Checkbox(EmuGloConfig[UI_DISAS_STEP], &step_debug);
    ImGui::SameLine();
    ImGui::Checkbox(EmuGloConfig[UI_DISAS_TRACE], &trace_debug);
    if(cur_break_real_pc != -1){
        ImGui::SameLine();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 sz = ImGui::CalcTextSize("[next]  [step]");
        //debug tool
        ImGui::GetWindowDrawList()
        ->AddRectFilled(pos, ImVec2(pos.x+sz.x,pos.y+sz.y),IM_COL32(255, 255, 0, 50));
        
        ImGui::Text("[next]");
        if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)){
            if(DebugBreakPoints.find(cur_break_real_pc) != DebugBreakPoints.end()){
                DebugBreakPoints[cur_break_real_pc]=1;
            }
            cur_break_real_pc = -1;
            emu->SetPaused(false);
        }
        ImGui::SameLine();

        ImGui::Text("  ");
        ImGui::SameLine();

        ImGui::Text("[step]");
        if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)){
            if(DebugBreakPoints.find(cur_break_real_pc) != DebugBreakPoints.end()){
                DebugBreakPoints[cur_break_real_pc]=1;
            }
            cur_break_real_pc = -1;
            emu->SetPaused(false);
            step_debug = true;
        }
    }
    //ImGui::BeginChild("##scrolling");
    DrawMonitor();
    //ImGui::EndChild();
    ImGui::End();
    debug_flags = DEBUG_BREAKPOINT | (step_debug?DEBUG_STEP:0) | (trace_debug?DEBUG_RET_TRACE:0);

}

void CodeViewer::JumpTo(uint8_t seg,uint16_t offset){
    int idx=0;
    //printf("jumpto:seg%d\n",seg);
    LookUp(seg, offset,&idx);
    cur_col=idx;
    need_roll=true;
}