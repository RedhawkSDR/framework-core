#ifndef STATE_H_
#define STATE_H_

class State
{
public:
    virtual ~State(){}
    
    virtual void update_state() = 0;
};

#endif