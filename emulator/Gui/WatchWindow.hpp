#pragma once

#include "../Emulator.hpp"
#include "UiBase.hpp"
#include "hex.hpp"

class WatchWindow:public UiBase{
private:
    MemoryEditor mem_editor;
public:

    UI_SINGLE_HEAD(WatchWindow)

    WatchWindow();

    void Show();
};