#include "Injector.hpp"
#include "imgui.h"
#include "hex.hpp"
#include "../Peripheral/BatteryBackedRAM.hpp"

#include <cstdlib>
#include <cstring>
#include <string>

#include "../Config/Config.hpp"
Injector::Injector(casioemu::Emulator*e){
    emu = e;
    data_buf = new char[1024];
    memset(data_buf, 0,1024);
}

void Injector::Show(){
    ImGui::Begin(EmuGloConfig[UI_INJECTOR]);
    
    static float scale = -1.0f;
    static int range = 64;
    static char strbuf[1024]={0};
    static char buf[10]={0};
    static MemoryEditor editor;
    static char* info_msg;
    ImGui::BeginChild("##ropinput",ImVec2(0,ImGui::GetWindowHeight()*0.6));
    ImGui::Text(EmuGloConfig[UI_CHANGE_SCALE]);
    ImGui::SameLine();
    if(scale == -1.0f){
        scale = EmuGloConfig.GetScale();
    }
    if(ImGui::SliderFloat(EmuGloConfig[UI_CHANGE_SCALE_SLIDER], &scale, 0.01f, 2.0f)){
        EmuGloConfig.SetScale(scale);
    }
    ImGui::NewLine();
    ImGui::Text(EmuGloConfig[UI_ROP_INPUT]);
    ImGuiIO& io = ImGui::GetIO();
    float ddpi, hdpi, vdpi;
    if (SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi) != 0) {
        fprintf(stderr, "Failed to obtain DPI information for display 0: %s\n", SDL_GetError());
        exit(1);
    }
    float dpi_scaling = ddpi / 72.f;
    io.FontGlobalScale = scale*dpi_scaling;

    editor.DrawContents(data_buf, range);
    ImGui::EndChild();
    ImGui::SliderInt(EmuGloConfig[UI_ROP_SETINPUTRANGE], &range, 64, 1024);
    ImGui::Text(EmuGloConfig[UI_ROP_ANOFFSET]);
    ImGui::SameLine();
    ImGui::InputText("offset", buf, 9);
    char *base_addr = casioemu::BatteryBackedRAM::rom_addr;
    if(ImGui::Button("Math I/O")){
        *(base_addr+0xd112 - 0xd000)= 0xc4;
        *(base_addr+0xd11e - 0xd000)= 0x00;
        info_msg = EmuGloConfig[UI_INFO1];
        ImGui::OpenPopup("info");
    }
    if(ImGui::Button(EmuGloConfig[UI_ROP_ENTERAN])){
        int off = atoi(buf);
        if(off>100){
            memset(base_addr+0xd180-0xd000, 0x31, 100);
            memset(base_addr+0xd180-0xd000+100, 0xa6, 1);
            memset(base_addr+0xd180-0xd000+101, 0x31, off-100);
        }else {
            memset(base_addr+0xd180-0xd000, 0x31, off);
        }
        *(base_addr+0xd180-0xd000+off)= 0xfd;
        *(base_addr+0xd180-0xd000+off+1)= 0x20;
        info_msg = EmuGloConfig[UI_INFO2];
        ImGui::OpenPopup("info");
    }
    if(ImGui::Button(EmuGloConfig[UI_ROP_LOAD])){
        memcpy(base_addr+0xd180-0xd000,data_buf,range);
        info_msg = EmuGloConfig[UI_INFO3];
        ImGui::OpenPopup("info");
    }
    if(ImGui::Button(EmuGloConfig[UI_ROP_LOADFROMSTR])){
        
        int j = 0;
        int i = 0;
        info_msg = EmuGloConfig[UI_INFO3];
        while (strbuf[i] != '\0') {
            if(strbuf[i] == ' ' || strbuf[i] == '\n'){
                j++;
                while(strbuf[i]==' ' || strbuf[i] == '\n'){
                    i++;
                }
            }else{
                char c1 = strbuf[i];
                char c2 = strbuf[i+1];
                if(c1>='0' && c1<='9'){
                    c1-='0';
                }else if(c1>='a' && c1<='f'){
                    c1=c1-'a'+10;
                }
                if(c2>='0' && c2<='9'){
                    c2-='0';
                }else if(c2>='a' && c2<='f'){
                    c2= c2 - 'a'+10;
                }
                *(base_addr+0xd180-0xd000+j) = (c1<<4)|(c2&0x0f);
                *(data_buf+j)=*(base_addr+0xd180-0xd000+j);
                i+=2;
            }            
        }
        ImGui::OpenPopup("info");

    }
    ImGui::SameLine();
    ImGui::InputTextMultiline("Input hex string. use ' ' to split", strbuf, 1023);
    
    if(ImGui::BeginPopupModal("info",0,ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::Text(info_msg);
        if(ImGui::Button("OK")){
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    
    ImGui::End();
}