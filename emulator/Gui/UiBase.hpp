#pragma once

class UiBase{

public:
    
    static UiBase *instance;
    
    template<class T>
    UiBase(T *child_this){
        this->instance = child_this;
    }

    virtual void BeforeShow();

    virtual void Show();
    
    template<class T>
    static T *GetInst(){
        return (T*)(UiBase::instance);
    }
};

