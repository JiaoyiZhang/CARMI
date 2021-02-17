#ifndef YCSB_H
#define YCSB_H

#include <algorithm>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "../../params.h"
#include <iomanip>
using namespace std;
extern bool kIsYCSB;

class YCSBDataset
{
public:
    YCSBDataset(double initRatio)
    {
        init = initRatio;
        if (initRatio == 0)
        {
            num = 0;
            init = 0.85;
        }
        else if (initRatio == 1)
            num = -1;
        else
            num = round(initRatio / (1 - initRatio));
    }

    void GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset);

private:
    int num;
    float init;
};

void YCSBDataset::GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset)
{
    vector<pair<double, double>>().swap(initDataset);
    vector<pair<double, double>>().swap(insertDataset);

    vector<pair<double, double>> ds;
    ifstream inFile("../src/dataset/newycsbdata.csv", ios::in);
    if (!inFile)
    {
        cout << "打开文件失败！" << endl;
        exit(1);
    }
    string line;
    while (getline(inFile, line))
    {
        if (line.empty())
            continue;
        istringstream sin(line);
        vector<string> fields;
        string field;
        while (getline(sin, field, ','))
            fields.push_back(field);
        string key = fields[0];
        key.erase(0, 4); // delete "user"
        double k = stod(key);
        double v = k / 10;
        ds.push_back({k, v});
        if (ds.size() == round(67108864.0 / init))
            break;
    }

    std::sort(ds.begin(), ds.end());
    if (kIsYCSB)
    {
        for (int i = 0; i < ds.size(); i++)
            initDataset.push_back(ds[i]);
        int end = round(100000 * (1 - init));
        auto maxValue = ds[ds.size() - 1];
        for (int i = 1; i <= end; i++)
            insertDataset.push_back({maxValue.first + i, maxValue.second + i});
    }
    cout << "YCSB Dataset: Read size:" << initDataset.size() << "\tWrite size:" << insertDataset.size() << endl;
}

#endif