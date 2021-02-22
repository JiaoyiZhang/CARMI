#ifndef BS_H
#define BS_H
#include <vector>
using namespace std;

class BSModel
{
public:
    BSModel() = default;
    void SetChildNumber(int c)
    {
        flagNumber = (7 << 24) + min(c, 15);
        for (int i = 0; i < 14; i++)
            index[i] = 0;
    }
    void Train(const vector<pair<double, double>> &dataset);
    int Predict(double key) const;

    int flagNumber;  // 4 Byte (flag + childNumber)
    int childLeft;   // 4 Byte
    float index[14]; // 56 Byte (childNumber = 15)
};

#endif