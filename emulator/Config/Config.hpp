#pragma once


#include <string>
#include <string_view>
#include <unordered_map>
#include "ini.hpp"

#include "imgui.h"

enum TranslateMap{
    UI_TITLE,
    UI_DISAS,
    UI_DISAS_WAITING,
    UI_DISAS_GOTO_ADDR,
    UI_DISAS_STEP,
    UI_DISAS_TRACE,
    UI_DISAS_CONTINUE,
    UI_REPORT_WINDOW,
    UI_REPORT_RANGE,
    UI_REPORT_RANGE_SLIDER,
    UI_STACK,
    UI_INJECTOR,
    UI_CHANGE_SCALE,
    UI_CHANGE_SCALE_SLIDER,
    UI_ROP_INPUT,
    UI_ROP_SETINPUTRANGE,
    UI_ROP_ANOFFSET,
    UI_ROP_ENTERAN,
    UI_ROP_LOAD,
    UI_ROP_LOADFROMSTR,
    UI_INFO1,
    UI_INFO2,
    UI_INFO3,
    UI_MEMEDIT
};

class EmuConfig{

private:
    std::string path;
    mINI::INIStructure root;
    mINI::INIFile *file = nullptr;
    std::unordered_map<int, std::string> translate;
    ImFontGlyphRangesBuilder fbuilder;
    bool format_succ = false;;

    void initTranslate();

    void loadTranslate(std::string path);

    void update();
public:
    EmuConfig(const char* filepath);

    ImFontGlyphRangesBuilder& GetAtlas();

    float GetScale();

    void SetScale(float num);

    char* operator[](int idx);

    std::string GetFontPath();

};

extern EmuConfig EmuGloConfig;