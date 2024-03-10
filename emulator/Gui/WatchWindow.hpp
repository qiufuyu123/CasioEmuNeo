#pragma once

#include "../Emulator.hpp"
#include "hex.hpp"

class WatchWindow{
private:
    MemoryEditor mem_editor;
    casioemu::Emulator *emu;
public:
    WatchWindow(casioemu::Emulator *e);

    void Show();
};