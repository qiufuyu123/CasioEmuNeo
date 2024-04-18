#pragma once

class UiBase{

public:
    
    UiBase(){};

    virtual void BeforeShow();

    virtual void Show();

};

#define UI_SINGLE_HEAD(T) static T* instance;
#define UI_SINGLE_IMPL(T) T* T::instance = nullptr;