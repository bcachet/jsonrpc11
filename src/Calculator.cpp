#include <limits>
#include <Calculator.h>

void Calculator::Push(double n) {
    values.push_back(n);
}

double Calculator::Add() {
    double result = 0;
    for(std::list<double>::const_iterator i = values.begin(); i != values.end(); ++i) {
        result += *i;
    }
    return result;
}

double Calculator::Divide() {
    double result = std::numeric_limits<double>::quiet_NaN();
    for(std::list<double>::const_iterator i = values.begin(); i != values.end(); ++i) {
        if (i == values.begin()) {
            result = *i;
        } else {
            result /= *i;
        }
    }
    return result;
}

void Calculator::Clear() {
  values.clear();
}
