#pragma once
#include "../Emulator.hpp"
class Injector{

private:
    char* data_buf;
    casioemu::Emulator *emu;
public:
    Injector(casioemu::Emulator *e);
    void Show();
};