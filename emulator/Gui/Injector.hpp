#pragma once
#include "../Emulator.hpp"
#include "UiBase.hpp"
class Injector : public UiBase{
private:
    char* data_buf;
public:
    UI_SINGLE_HEAD(Injector)
    Injector();
    void Show();
};