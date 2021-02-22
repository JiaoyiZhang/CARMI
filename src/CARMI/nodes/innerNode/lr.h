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
    void Train(const vector<pair<double, double>> &dataset);
    int Predict(double key) const;

    int flagNumber; // 4 Byte (flag + childNumber)

    int childLeft; // 4 Byte
    float divisor; // 4 Byte
    float minValue;              // 4 Byte
    pair<float, float> theta[6]; // 48 Byte
};

#endif