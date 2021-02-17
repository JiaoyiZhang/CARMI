#ifndef NN_H
#define NN_H

#include <iostream>
#include <vector>
using namespace std;

class PLRModel
{
public:
    PLRModel() = default;
    void SetChildNumber(int c)
    {
        flagNumber = (5 << 24) + c;
    }
    void Initialize(const vector<pair<double, double>> &dataset);
    void Train(const vector<pair<double, double>> &dataset);
    void Train(const int left, const int size);
    int Predict(double key);

    int flagNumber; // 4 Byte (flag + childNumber)

    int childLeft;             // 4 Byte
    pair<float, int> point[6]; // 48 Byte
    float maxX;
    float tmp;
};

#endif
