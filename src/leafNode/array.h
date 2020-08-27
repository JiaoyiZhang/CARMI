#ifndef ARRAY_H
#define ARRAY_H

#include <math.h>
#include "leaf_node.h"
using namespace std;

class ArrayNode : public BasicLeafNode
{
public:
    ArrayNode(int maxNumber) : BasicLeafNode()
    {
        m_maxNumber = maxNumber;
        writeTimes = 0;
    }

    void SetDataset(const vector<pair<double, double>> &dataset);
    double CalculateError();
    double UpdateError(const vector<pair<double, double>> &findDataset, const vector<pair<double, double>> &insertDataset);

    pair<double, double> Find(double key);
    bool Insert(pair<double, double> data);
    bool Delete(double key);
    bool Update(pair<double, double> data);

    static long double GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, const vector<pair<double, double>> &dataset);

    int BinarySearch(double key, int preIdx, int start, int end);
    int ExponentialSearch(double key, int preIdx, int start, int end);

    // designed for gtest
    int GetPredictIndex(double key)
    {
        double p = model->Predict(key);
        int preIdx = static_cast<int>(p * (m_datasetSize - 1));
        return preIdx;
    }

private:
    int m_maxNumber; // the maximum number of Inserts
    int writeTimes;
};

void ArrayNode::SetDataset(const vector<pair<double, double>> &dataset)
{
    m_dataset = dataset;
    m_datasetSize = m_dataset.size();
    if (m_datasetSize == 0)
        return;

    model->Train(m_dataset);
    for (int i = 0; i < m_datasetSize; i++)
    {
        double p = model->Predict(m_dataset[i].first);
        int preIdx = static_cast<int>(p * (m_datasetSize - 1));
        int e = abs(i - preIdx);
        if (e > error)
            error = e;
    }
    error++;
    writeTimes = 0;
}

double ArrayNode::CalculateError()
{
    double error = 0;    
    for (int i = 0; i < m_datasetSize; i++)
    {
        double p = model->Predict(m_dataset[i].first);
        int preIdx = static_cast<int>(p * (m_datasetSize - 1));
        error += abs(i - preIdx);
    }
    return error;
}

pair<double, double> ArrayNode::Find(double key)
{
    if (m_datasetSize == 0)
        return {};
    double p = model->Predict(key);
    int preIdx = static_cast<int>(p * (m_datasetSize - 1));
    if (m_dataset[preIdx].first == key)
    {
        return m_dataset[preIdx];
    }
    else
    {
        int start = max(0, preIdx - error);
        int end = min(m_datasetSize - 1, preIdx + error);
        start = min(start, end);
        int res = SEARCH_METHOD(key, preIdx, start, end);
        if (res <= start)
            res = SEARCH_METHOD(key, preIdx, 0, start);
        else if (res >= end)
        {
            res = SEARCH_METHOD(key, preIdx, res, m_datasetSize - 1);
            if (res >= m_datasetSize)
                return {};
        }
        if (m_dataset[res].first == key)
            return m_dataset[res];
        return {};
    }
}

bool ArrayNode::Insert(pair<double, double> data)
{
    if (m_datasetSize == 0)
    {
        m_dataset.push_back(data);
        m_datasetSize++;
        writeTimes++;
        SetDataset(m_dataset);
        return true;
    }
    double p = model->Predict(data.first);
    int preIdx = static_cast<int>(p * (m_datasetSize - 1));
    int start = max(0, preIdx - error);
    int end = min(m_datasetSize - 1, preIdx + error);
    start = min(start, end);

    preIdx = SEARCH_METHOD(data.first, preIdx, start, end);
    if (preIdx <= start)
        preIdx = SEARCH_METHOD(data.first, preIdx, 0, start);
    else if (preIdx >= end)
        preIdx = SEARCH_METHOD(data.first, preIdx, preIdx, m_datasetSize - 1);

    // Insert data
    if (preIdx == m_datasetSize - 1 && m_dataset[preIdx].first < data.first)
    {
        m_dataset.push_back(data);
        m_datasetSize++;
        writeTimes++;
        return true;
    }
    m_dataset.push_back(m_dataset[m_datasetSize - 1]);
    m_datasetSize++;
    for (int i = m_datasetSize - 2; i > preIdx; i--)
        m_dataset[i] = m_dataset[i - 1];
    m_dataset[preIdx] = data;

    writeTimes++;

    // If the current number is greater than the maximum,
    // the child node needs to be retrained
    if (writeTimes >= m_datasetSize || writeTimes > m_maxNumber)
        SetDataset(m_dataset);
    return true;
}

bool ArrayNode::Delete(double key)
{
    double p = model->Predict(key);
    int preIdx = static_cast<int>(p * (m_datasetSize - 1));
    if (m_dataset[preIdx].first != key)
    {
        int start = max(0, preIdx - error);
        int end = min(m_datasetSize - 1, preIdx + error);
        start = min(start, end);
        int res = SEARCH_METHOD(key, preIdx, start, end);
        if (m_dataset[res].first == key)
            preIdx = res;
        else
        {
            if (res <= start)
                res = SEARCH_METHOD(key, preIdx, 0, start);
            else if (res >= end)
            {
                res = SEARCH_METHOD(key, preIdx, res, m_datasetSize - 1);
                if (res >= m_datasetSize)
                    return false;
            }
            if (m_dataset[res].first == key)
                preIdx = res;
            else
                return false;
        }
    }
    for (int i = preIdx; i < m_datasetSize - 1; i++)
        m_dataset[i] = m_dataset[i + 1];
    m_datasetSize--;
    m_dataset.pop_back();
    writeTimes++;
    return true;
}

bool ArrayNode::Update(pair<double, double> data)
{
    double p = model->Predict(data.first);
    int preIdx = static_cast<int>(p * (m_datasetSize - 1));
    if (m_dataset[preIdx].first != data.first)
    {
        int start = max(0, preIdx - error);
        int end = min(m_datasetSize - 1, preIdx + error);
        start = min(start, end);
        int res = SEARCH_METHOD(data.first, preIdx, start, end);
        if (m_dataset[res].first == data.first)
            preIdx = res;
        else
        {
            if (res <= start)
                res = SEARCH_METHOD(data.first, preIdx, 0, start);
            else if (res >= end)
            {
                res = SEARCH_METHOD(data.first, preIdx, res, m_datasetSize - 1);
                if (res >= m_datasetSize)
                    return false;
            }
            if (m_dataset[res].first == data.first)
                preIdx = res;
            else
                return false;
        }
    }
    m_dataset[preIdx].second = data.second;
    return true;
}

int ArrayNode::BinarySearch(double key, int preIdx, int start, int end)
{
    while (start < end)
    {
        int mid = (start + end) / 2;
        if (m_dataset[mid].first < key)
            start = mid + 1;
        else
            end = mid;
    }
    return start;
}

int ArrayNode::ExponentialSearch(double key, int preIdx, int start, int end)
{
    // exponential search
    int start_idx;
    int end_idx;
    if (m_dataset[preIdx].first < key)
    {
        int i = 1;
        while (preIdx + i < m_datasetSize && m_dataset[preIdx + i].first < key)
            i *= 2;
        start_idx = preIdx + i / 2;
        end_idx = min(preIdx + i, m_datasetSize - 1);
    }
    else
    {
        int i = 1;
        while (preIdx - i >= 0 && m_dataset[preIdx - i].first >= key)
            i *= 2;
        start_idx = max(0, preIdx - i);
        end_idx = preIdx - i / 2;
    }
    end_idx = min(end_idx, m_datasetSize - 1);
    return BinarySearch(key, preIdx, start_idx, end_idx);
}

long double ArrayNode::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, const vector<pair<double, double>> &dataset)
{
    int datasetSize = dataset.size();
    if (datasetSize == 0)
        return 0;
        
    int leftError = 0;
    int rightError = 0;
    BasicModel *tmpModel = new LinearRegression();
    tmpModel->Train(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
        {
            double p = tmpModel->Predict(dataset[i].first);
            int preIdx = static_cast<int>(p * (datasetSize - 1));
            int error = i - preIdx;
            if (error > rightError)
                rightError = error;
            if (error < leftError)
                leftError = error;
        }
    }
    rightError++;
    leftError--;

    // calculate cost
    long double totalCost = 16.0 * kRate;
    double READCOST = log(rightError-leftError+1) / log(2);
    double WRITECOST = READCOST + (datasetSize + 1) / 2;
    for (int i = 0; i < datasetSize; i++)
    {
        pair<int, int> tmp = (cntTree.find(dataset[i].first))->second;
        double tmpRead = tmp.first * READCOST * (1 - kRate);
        double tmpWrite = tmp.second * WRITECOST * (1 - kRate);
        totalCost += tmpRead + tmpWrite;
    }
    return totalCost;
}

double ArrayNode::UpdateError(const vector<pair<double, double>> &findDataset, const vector<pair<double, double>> &insertDataset)
{
    if(findDataset.size() == 0 && insertDataset.size() == 0)
        return 0;
        
    if(error > 1000)
        return (log(error) / log(2)) * m_datasetSize;
    vector<pair<double, double>> dataset;
    for(int i=0;i<m_dataset.size();i++)
        dataset.push_back(m_dataset[i]);
    int newError = error;
    
    LARGE_INTEGER s, e, c;
    double time = 999999999;
    for(int i = error; i >= 0; i--)
    {
        error = i;
        QueryPerformanceFrequency(&c);  
        QueryPerformanceCounter(&s);
        for (int i = 0; i < findDataset.size(); i++)
            Find(findDataset[i].first);
        for (int i = 0; i < insertDataset.size(); i++)
            Insert(insertDataset[i]);
        QueryPerformanceCounter(&e);
        double tmpTime = (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
        if(tmpTime < time)
        {
            time = tmpTime;
            newError = error;
        }
        m_dataset.clear();
        for(int i=0;i<dataset.size();i++)
            m_dataset.push_back(dataset[i]);
        m_datasetSize=dataset.size();
    }
    error = newError;
    if(error == 0)
        return 0;
    // cout<<"After update errornegative error is: "<<maxNegativeError<<"\tpositive error is: "<<maxPositiveError<<endl;
    // // cout<<"The shortest time is: "<<time<<"\taverage: "<<time / float(insertDataset.size()+findDataset.size())<<endl;
    // cout<<"newP - newN :"<<newP - newN<<endl;
    // cout<<"log(newp-newn): "<<log(newP - newN)<<endl;
    // cout<<"final error is: " << error<<endl;
    return (log(error) / log(2)) * m_datasetSize;
}

#endif