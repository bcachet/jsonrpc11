#ifndef CALCULATOR_H_
#define CALCULATOR_H_

#include <list>

class Calculator {
private:
    std::list<double> values;
public:
    void Push(double);
    double Add();
    double Divide();
    void Clear();
};

#endif
