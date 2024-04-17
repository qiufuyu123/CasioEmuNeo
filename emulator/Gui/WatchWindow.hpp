#pragma once

#include "../Emulator.hpp"
#include "UiBase.hpp"
#include "hex.hpp"

class WatchWindow:public UiBase{
private:
    MemoryEditor mem_editor;
public:
    WatchWindow();

    void Show();
};