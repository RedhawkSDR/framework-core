#ifndef REPORTING_H_
#define REPORTING_H_

class Reporting
{
public:
    virtual ~Reporting(){}
    
    virtual void report() = 0;
};

#endif