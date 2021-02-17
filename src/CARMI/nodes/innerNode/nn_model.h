
#ifndef NN_MODEL_H
#define NN_MODEL_H

#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include "../dataManager/child_array.h"
#include "../baseNode.h"
#include <vector>
using namespace std;

extern vector<BaseNode> entireChild;
extern vector<pair<double, double>> findActualDataset;

inline void NNModel::Initialize(const vector<pair<double, double>> &dataset)
{
    int childNumber = flagNumber & 0x00FFFFFF;
    childLeft = allocateChildMemory(childNumber);
    if (dataset.size() == 0)
        return;

    Train(dataset);

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        int p = Predict(dataset[i].first);
        perSubDataset[p].push_back(dataset[i]);
    }

    switch (kLeafNodeID)
    {
    case 0:
        for (int i = 0; i < childNumber; i++)
        {
            ArrayType tmp(kThreshold);
            tmp.SetDataset(perSubDataset[i], kMaxKeyNum);
            entireChild[childLeft + i].array = tmp;
        }
        break;
    case 1:
        for (int i = 0; i < childNumber; i++)
        {
            GappedArrayType tmp(kThreshold);
            tmp.SetDataset(perSubDataset[i], kMaxKeyNum);
            entireChild[childLeft + i].ga = tmp;
        }
        break;
    }
}

inline void NNModel::Train(const vector<pair<double, double>> &dataset)
{
    int childNumber = flagNumber & 0x00FFFFFF;
    int actualSize = 0;
    vector<double> xx(7, 0.0);
    vector<double> x(7, 0.0);
    vector<double> px(7, 0.0);
    vector<double> pp(7, 0.0);
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != DBL_MIN)
            actualSize++;
    }
    if (actualSize == 0)
        return;
    for (int i = dataset.size() - 1; i >= 0; i--)
    {
        if (dataset[i].first != DBL_MIN)
        {
            maxX = dataset[i].first;
            break;
        }
    }
    int width = actualSize / 7;
    for (int i = 0; i < 7; i++)
    {
        point[i] = {dataset[(i + 1) * width].first, childNumber * (i + 1) / 7};
    }
    if (childNumber <= 32)
        return;
    for (int k = 0; k < 7; k++)
    {
        int start = k * width;
        int end = (k + 1) * width;
        for (int i = start; i < end; i++)
        {
            if (dataset[i].first != DBL_MIN)
            {
                xx[k] += dataset[i].first * dataset[i].first;
                x[k] += dataset[i].first;
                px[k] += float(i) / dataset.size() * dataset[i].first;
                pp[k] += float(i * i) / dataset.size() / dataset.size();
            }
        }
        px[k] *= childNumber;
        pp[k] *= childNumber * childNumber;
    }
    double opt = DBL_MAX;
    vector<pair<double, double>> theta(7, {1, 1});
    int bound = childNumber / 7;
    int a, b;
    for (int t1 = -2; t1 < 2; t1++)
    {
        theta[0] = {float(bound + t1) / point[0].first, 0};
        for (int t2 = -2; t2 < 2; t2++)
        {
            a = float(bound + t2 - t1) / (point[1].first - point[0].first);
            b = bound * 2 + t2 - a * point[1].first;
            theta[1] = {a, b};
            for (int t3 = -2; t3 < 2; t3++)
            {
                a = float(bound + t3 - t2) / (point[2].first - point[1].first);
                b = bound * 3 + t3 - a * point[2].first;
                theta[2] = {a, b};
                for (int t4 = -2; t4 < 2; t4++)
                {
                    a = float(bound + t4 - t3) / (point[3].first - point[2].first);
                    b = bound * 4 + t4 - a * point[3].first;
                    theta[3] = {a, b};
                    for (int t5 = -2; t5 < 2; t5++)
                    {
                        a = float(bound + t5 - t4) / (point[4].first - point[3].first);
                        b = bound * 5 + t5 - a * point[4].first;
                        theta[4] = {a, b};
                        for (int t6 = -2; t6 < 2; t6++)
                        {
                            a = float(bound + t6 - t5) / (point[5].first - point[4].first);
                            b = bound * 6 + t6 - a * point[5].first;
                            theta[5] = {a, b};
                            a = float(childNumber - 6 * bound - t6) / (maxX - point[5].first);
                            b = childNumber - a * maxX;
                            theta[6] = {a, b};

                            double value = 0.0;
                            for (int i = 0; i < 7; i++)
                            {
                                value += theta[i].first * theta[i].first * xx[i];
                                value += 2 * theta[i].first * theta[i].second * x[i];
                                value -= 2 * (theta[i].first + theta[i].second) * px[i];
                                value += theta[i].second * theta[i].second;
                                value += pp[i];
                            }
                            if (value < opt)
                            {
                                opt = value;
                                point[0].second = bound + t1;
                                point[1].second = 2 * bound + t2;
                                point[2].second = 3 * bound + t3;
                                point[3].second = 4 * bound + t4;
                                point[4].second = 5 * bound + t5;
                                point[5].second = 6 * bound + t6;
                            }
                        }
                    }
                }
            }
        }
    }
}

inline int NNModel::Predict(double key)
{
    int s = 0;
    int e = 5;
    int mid;
    while (s < e)
    {
        mid = (s + e) / 2;
        if (point[mid].first < key)
            s = mid + 1;
        else
            e = mid;
    }

    int p;
    if (s == 0)
    {
        p = float(point[0].second) / point[0].first * key;
    }
    else if (s == 5)
    {
        int childNumber = flagNumber & 0x00FFFFFF;
        int a = float(childNumber - point[5].second) / (maxX - point[5].first);
        int b = childNumber - a * maxX;
        p = a * key + b;
    }
    else
    {
        int a = float(point[s].second - point[s - 1].second) / (point[s].second - point[s - 1].first);
        int b = point[s].second - a * point[s].first;
        p = a * key + b;
    }

    return p;
}

inline void NNModel::Train(const int left, const int size)
{
    int childNumber = flagNumber & 0x00FFFFFF;
    int actualSize = 0;
    vector<double> xx(7, 0.0);
    vector<double> x(7, 0.0);
    vector<double> px(7, 0.0);
    vector<double> pp(7, 0.0);
    if (size == 0)
        return;
    int width = size / 7;
    for (int i = 0; i < 7; i++)
    {
        point[i] = {findActualDataset[(i + 1) * width].first, childNumber * (i + 1) / 7};
    }
    if (childNumber <= 32)
        return;
    for (int k = 0; k < 7; k++)
    {
        int start = k * width;
        int end = (k + 1) * width;
        for (int i = start; i < end; i++)
        {
            xx[k] += findActualDataset[i].first * findActualDataset[i].first;
            x[k] += findActualDataset[i].first;
            px[k] += float(i) / size * findActualDataset[i].first;
            pp[k] += float(i * i) / size / size;
        }
        px[k] *= childNumber;
        pp[k] *= childNumber * childNumber;
    }
    double opt = DBL_MAX;
    vector<pair<double, double>> theta(6, {1, 1});
    int bound = childNumber / 7;
    int a, b;
    for (int t1 = -2; t1 < 2; t1++)
    {
        theta[0] = {float(bound + t1) / point[0].first, 0};
        for (int t2 = -2; t2 < 2; t2++)
        {
            a = float(bound + t2 - t1) / (point[1].first - point[0].first);
            b = bound * 2 + t2 - a * point[1].first;
            theta[1] = {a, b};
            for (int t3 = -2; t3 < 2; t3++)
            {
                a = float(bound + t3 - t2) / (point[2].first - point[1].first);
                b = bound * 3 + t3 - a * point[2].first;
                theta[2] = {a, b};
                for (int t4 = -2; t4 < 2; t4++)
                {
                    a = float(bound + t4 - t3) / (point[3].first - point[2].first);
                    b = bound * 4 + t4 - a * point[3].first;
                    theta[3] = {a, b};
                    for (int t5 = -2; t5 < 2; t5++)
                    {
                        a = float(bound + t5 - t4) / (point[4].first - point[3].first);
                        b = bound * 5 + t5 - a * point[4].first;
                        theta[4] = {a, b};
                        for (int t6 = -2; t6 < 2; t6++)
                        {
                            a = float(bound + t6 - t5) / (point[5].first - point[4].first);
                            b = bound * 6 + t6 - a * point[5].first;
                            theta[5] = {a, b};
                            a = float(childNumber - 6 * bound - t6) / (maxX - point[5].first);
                            b = childNumber - a * maxX;
                            theta[6] = {a, b};

                            double value = 0.0;
                            for (int i = 0; i < 7; i++)
                            {
                                value += theta[i].first * theta[i].first * xx[i];
                                value += 2 * theta[i].first * theta[i].second * x[i];
                                value -= 2 * (theta[i].first + theta[i].second) * px[i];
                                value += theta[i].second * theta[i].second;
                                value += pp[i];
                            }
                            if (value < opt)
                            {
                                opt = value;
                                point[0].second = bound + t1;
                                point[1].second = 2 * bound + t2;
                                point[2].second = 3 * bound + t3;
                                point[3].second = 4 * bound + t4;
                                point[4].second = 5 * bound + t5;
                                point[5].second = 6 * bound + t6;
                            }
                        }
                    }
                }
            }
        }
    }
}

#endif