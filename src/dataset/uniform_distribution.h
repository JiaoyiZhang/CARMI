#ifndef UNIFORM_DISTRIBUTION_H
#define UNIFORM_DISTRIBUTION_H

#include <algorithm>
#include <random>
#include <iostream>
#include <vector>
using namespace std;

class UniformDataset
{
public:
    UniformDataset(int total, double initRatio)
    {
        totalSize = total;
        if(initRatio == 0)
        {// several leaf nodes are inserted
            insertSize = 0;
            initSize = 0;
        }
        else if(initRatio == 1)
            num = -1;
        else
        {
            initSize = total * initRatio;
            insertSize = totalSize - initSize;
            num = initRatio / (1 - initRatio);
        }
    }

    void GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset);

private:
    int totalSize;
    int insertSize;
    int initSize;

    int num;
};

void UniformDataset::GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset)
{
    initDataset.clear();
    insertDataset.clear();
    int cnt = 0;
    if(initSize == 0)
    {
        int i = 0;
        for(; i < 0.6 * totalSize; i++)
            initDataset.push_back({double(i), double(i) * 10});
        for(; i < 0.9 * totalSize; i+=2)
        {
            initDataset.push_back({double(i), double(i) * 10});
            insertDataset.push_back({double(i+1), double(i+1) * 10});
        }
        for(; i < totalSize; i++)
            initDataset.push_back({double(i), double(i) * 10});        
    }
	else if(num == -1)
	{
        for (int i = 0; i < totalSize; i++)
            initDataset.push_back({double(i), double(i) * 10});
	}
    else
    {
        for (int i = 0; i < totalSize; i++)
        {
            cnt++;
            if (cnt <= num)
            {
                initDataset.push_back({double(i), double(i) * 10});
            }
            else
            {
                insertDataset.push_back({double(i), double(i) * 10});
                cnt = 0;
            }
        }
    }
    cout<<"uniform: Read size:"<<initDataset.size()<<"\tWrite size:"<<insertDataset.size()<<endl;
}

#endif