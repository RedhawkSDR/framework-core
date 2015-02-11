#ifndef STATISTICS_H_
#define STATISTICS_H_

class Statistics
{
public:
    virtual ~Statistics(){}
    
    virtual void compute_statistics() = 0;
};

#endif