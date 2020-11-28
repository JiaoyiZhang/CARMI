#ifndef GREEDY_H
#define GREEDY_H

#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"
#include "../function.h"
#include <vector>
using namespace std;

extern vector<LRType> LRVector;
extern vector<NNType> NNVector;
extern vector<HisType> HisVector;
extern vector<BSType> BSVector;

pair<int, int> ChooseRoot(const vector<pair<double, double>> &dataset)
{
    double OptimalValue = DBL_MAX;
    int space;
    int c;
    int optimalChildNumber, optimalType;
    for (int c = 1024; c <= dataset.size();)
    {
        if (c <= 4096)
            c *= 2;
        else if (c <= 40960)
            c += 8192;
        else if (c <= 1000000)
            c += 65536;
        else
            c *= 2;
        for (int type = 0; type < 4; type++)
        {
            double time;
            switch (type)
            {
            case 0:
            {
                space = (4 * c + sizeof(LRType) + sizeof(ArrayType) * c);
                LRVector.push_back(LRType(c));
                LRVector[0].Initialize(dataset);
                time = 8.1624 * 1e-9 * dataset.size();
                break;
            }
            case 1:
            {
                space = (4 * c + 192 + sizeof(NNType) + sizeof(ArrayType) * c);
                NNVector.push_back(NNType(c));
                NNVector[0].Initialize(dataset);
                time = 20.26894 * 1e-9 * dataset.size();
                break;
            }
            case 2:
            {
                space = (5 * c + sizeof(HisType) + sizeof(ArrayType) * c);
                HisVector.push_back(HisType(c));
                HisVector[0].Initialize(dataset);
                time = 19.6543 * 1e-9 * dataset.size();
                break;
            }
            case 3:
            {
                space = (12 * c + sizeof(BSType) + sizeof(ArrayType) * c);
                BSVector.push_back(BSType(c));
                BSVector[0].Initialize(dataset);
                time = 4 * log(c) / log(2) * 1e-9 * dataset.size();
                break;
            }
            }

            auto entropy = GetEntropy(dataset.size());
            // entropy /= (log(c) / log(2));
            double ratio = time / entropy;
            if (ratio <= OptimalValue)
            {
                optimalChildNumber = c;
                optimalType = type;
                OptimalValue = ratio;
            }
            vector<LRType>().swap(LRVector);
            vector<NNType>().swap(NNVector);
            vector<HisType>().swap(HisVector);
            vector<BSType>().swap(BSVector);
            vector<ArrayType>().swap(ArrayVector);
            vector<GappedArrayType>().swap(GAVector);
            initEntireData(entireDataSize);
        }
    }
    cout << "Best type is: " << optimalType << "\tbest childNumber: " << optimalChildNumber << "\tOptimal Value: " << OptimalValue << endl;
    return {optimalType, optimalChildNumber};
}

#endif // !GREEDY_H