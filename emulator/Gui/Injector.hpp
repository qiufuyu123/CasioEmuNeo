#pragma once
#include "../Emulator.hpp"
#include "UiBase.hpp"
class Injector : public UiBase{
private:
    char* data_buf;
public:

    Injector();
    void Show();
};