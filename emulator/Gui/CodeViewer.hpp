#pragma once
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <unordered_map>


#include "../Emulator.hpp"
typedef struct{
    uint8_t segment;
    uint16_t offset;
    char srcbuf[40];
}CodeElem;

enum EmuDebugFlag{
    DEBUG_BREAKPOINT=1,
    DEBUG_STEP=2,
    DEBUG_RET_TRACE=4
};

extern std::unordered_map<int, uint8_t> DebugBreakPoints;

class CodeViewer
{ 
    private:
        casioemu::Emulator *emu;
        std::vector<CodeElem> codes;
        size_t rows;
        std::string src_path;
        char adrbuf[9]{0};
        int max_row = 0;
        int max_col = 0;
        int cur_col = 0;
        bool is_loaded = false;
        bool edit_active = false;
        bool need_roll = false;
        int cur_break_real_pc = -1;
        uint32_t selected_addr = -1;

    public:
        uint8_t debug_flags = DEBUG_BREAKPOINT;
        CodeViewer(std::string path,casioemu::Emulator* e);
        ~CodeViewer();
        bool TryTrigBP(uint8_t seg,uint16_t offset,bool bp_mode=true);
        CodeElem LookUp(uint8_t seg,uint16_t offset,int *idx=0);
        void DrawWindow();
        void DrawContent();
        void DrawMonitor();
        void JumpTo(uint8_t seg,uint16_t offset);
};