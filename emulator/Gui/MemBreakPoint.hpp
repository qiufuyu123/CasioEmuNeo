#pragma once
#include "UiBase.hpp"
#include <cstdint>
#include <unordered_map>
#include <vector>

typedef struct{
    bool Write;
    uint16_t pc;
}MemRecord_t;

typedef struct {
    bool enableWrite = false;
    uint16_t addr;
    std::unordered_map<uint32_t, int> records;
}MemBPData_t;

class MemBreakPoint : public UiBase
{

private:
    std::vector<MemBPData_t> break_point_hash;

    int target_addr = -1;

    void DrawFindContent();

    void DrawContent();
public:

    UI_SINGLE_HEAD(MemBreakPoint)

    MemBreakPoint();

    void TryTrigBp(uint16_t addr_edit,bool write);

    void Show();
};