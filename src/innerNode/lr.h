#ifndef LR_H
#define LR_H

#include <iostream>
#include <vector>
using namespace std;

class LRModel
{
public:
    LRModel() = default;
    void SetChildNumber(int c)
    {
        flagNumber = (4 << 24) + c;
    }
    void Initialize(const vector<pair<double, double>> &dataset);
    void Train(const vector<pair<double, double>> &dataset);
    int Predict(double key);

    int flagNumber; // 4 Byte (flag + childNumber)

    int childLeft; // 4 Byte
    float divisor; // 4 Byte
    // float bound;                 // 4 Byte bound = c / 5;
    float minValue;              // 4 Byte
    pair<float, float> theta[6]; // 48 Byte
};

#endif