#include "Config.hpp"
#include <fstream>
#include <imgui.h>
#include <string>

void EmuConfig::initTranslate(){
    translate[UI_TITLE]="CasioEmuNeo";
    translate[UI_DISAS]="Disassembler";
    translate[UI_DISAS_WAITING]="Please wait ...";
    translate[UI_DISAS_GOTO_ADDR]="go to addr: ";
    translate[UI_DISAS_STEP]="step";
    translate[UI_DISAS_TRACE]="trace";
    translate[UI_REPORT_WINDOW]="Watch Window";
    translate[UI_REPORT_RANGE]="set stack range";
    translate[UI_REPORT_RANGE_SLIDER]="range";
    translate[UI_STACK]="Stack Window";
    translate[UI_INJECTOR]="Injector";
    translate[UI_CHANGE_SCALE]="Change Scale";
    translate[UI_CHANGE_SCALE_SLIDER]="scale";
    translate[UI_ROP_INPUT]="Input your rop!";
    translate[UI_ROP_SETINPUTRANGE]="Set rop size";
    translate[UI_ROP_ANOFFSET]="set 'an' offset bytes";
    translate[UI_ROP_ENTERAN]="Enter 'an'";
    translate[UI_ROP_LOAD]="Load ROP";
    translate[UI_ROP_LOADFROMSTR]="Load ROP from string";
    translate[UI_INFO1]="Calculator is already set to Math I/O mode!";
    translate[UI_INFO2]="'an' is entered!\nPlease make sure you're in Math I/O mode\n Back to emulator and press [->][=] to finish!";
    translate[UI_INFO3]="ROP is entered!\nPlease press [->][=] to finish!";
    translate[UI_MEMEDIT]="Memory Viewer";
}

void EmuConfig::update(){
    file->write(root);
}

void EmuConfig::loadTranslate(std::string path){
    std::ifstream fin(path);
    if(fin.fail())
        return;
    char buf[200] = {0};
    int ss = 0;
    while (!fin.eof()) {
        fin.getline(buf,199);
        for (int i = 0; i < 100 && buf[i]; i++) {
            if(buf[i]=='%')
                buf[i]='\n';
        }
        fbuilder.AddText(buf);
        translate[ss] = std::string(buf);
        
        ss++;
    }
    fin.close();
}

EmuConfig::EmuConfig(const char *f){
    initTranslate();
    path = f;
    file = new mINI::INIFile(path);
    if(!file->read(root))
        return;
    format_succ = true;
    if(root.has("lang")){
        auto& lang = root["lang"];
        if(lang.has("lang")){
            std::string prefix = lang["lang"];
            loadTranslate("lang/"+prefix+".ini");
        }
    }

}

std::string EmuConfig::GetFontPath(){
    if(root.has("settings")){
        if(root["settings"].has("font")){
            return root["settings"]["font"];
        }
    }
    return "unifont.otf";
}

float EmuConfig::GetScale(){
    if(!root.has("settings"))
        return 1.0;
    if(!root["settings"].has("scale"))
        return 1.0;
    return std::stof(root["settings"]["scale"]);
}

void EmuConfig::SetScale(float num){
    root["settings"]["scale"] = std::to_string(num);
    update();
}

char* EmuConfig::operator[](int idx){
    return translate[idx].data();
}

ImFontGlyphRangesBuilder& EmuConfig::GetAtlas(){
    return fbuilder;
}

EmuConfig EmuGloConfig("config.ini");