#ifndef SEGMENTATION_MODEL_H
#define SEGMENTATION_MODEL_H

#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;
class secondStageWithSplit
{
public:
    secondStageWithSplit()
    {
        m_datasetSize = 0;
        theta1 = 0.233;
        theta2 = 0.666;
        maxNegativeError = 0;
        maxPositiveError = 0;
        isLeafNode = true;
        capacity = 400;
        childNum = 0;
    }

    void train();

    int getNegativeError()
    {
        return maxNegativeError;
    }

    int getPositiveError()
    {
        return maxPositiveError;
    }

    double getTheta1()
    {
        return theta1;
    }

    double getTheta2()
    {
        return theta2;
    }

    void initialInsert(pair<double, double> data)
    {
        m_dataset.push_back(data);
        m_datasetSize++;
        if (m_datasetSize > capacity)
            split();
        if (isLeafNode)
            train();
    }

    void split();

    void insert(pair<double, double> data, int idx)
    {
    }

    pair<double, double> find(double key);
    pair<double, double> update(double key, double newData);
    pair<double, double> del(double key);
    pair<double, double> insert(double key, double data);

    vector<secondStageWithSplit *> children;

private:
    int findIndex(double key);

    int childNum;

    vector<pair<double, double>> m_dataset;
    int m_datasetSize;
    double theta1;
    double theta2;
    int maxNegativeError;
    int maxPositiveError;

    bool isLeafNode;
    int capacity;
};

int secondStageWithSplit::findIndex(double key)
{
    int p = int(theta1 * key + theta2);
    p = (p > m_datasetSize - 1 ? m_datasetSize - 1 : p);
    p = (p < 0 ? 0 : p);
    // exponential search to find start_idx and end_idx
    int start_idx;
    int end_idx;
    if (m_dataset[p].first < key)
    {
        int i = 1;
        while (p + i < m_datasetSize && m_dataset[p + i].first < key)
            i *= 2;
        start_idx = p + i / 2;
        end_idx = min(p + i, m_datasetSize - 1);
    }
    else if (m_dataset[p].first > key)
    {
        int i = 1;
        while (p - i >= 0 && m_dataset[p - i].first >= key)
            i *= 2;
        start_idx = max(0, p - i);
        end_idx = p - i / 2;
    }
    else
        return p;

    // binary search
    int res = -1;
    while (start_idx < end_idx)
    {
        int mid = (start_idx + end_idx) / 2;
        if (m_dataset[mid].first == key)
        {
            res = mid;
            break;
        }
        else if (m_dataset[mid].first > key)
            end_idx = mid;
        else
            start_idx = mid + 1;
    }

    if (res != -1)
        return res;
    else
    {
        cout << "Fail to find the key" << endl;
        return -1;
    }
}

// use model to predict the index
// if this is leaf node, return {key, value}
// else return the index of child(next layer), {-999, index}
pair<double, double> secondStageWithSplit::find(double key)
{
    if (isLeafNode)
    {
        int p = findIndex(key);
        if (p == -1)
            return {-1, -1};
        else
            return m_dataset[p];
    }
    else
    {
        double index = -1;
        for (int i = 0; i < m_datasetSize; i++)
        {
            if (key < m_dataset[i].first)
            {
                index = m_dataset[i].second;
                break;
            }
        }
        if (index == -1)
        {
            cout << "Fail to find the key" << endl;
            return {-1, -1};
        }
        else
            return {-999, index};
    }
}

/*
// insert successfully, return {key, data}
// fail to find the key, return {-1, -1}
// if this is innerNode, return {-999, index}
pair<double, double> secondStageWithSplit::insert(double key, double data)
{
    if (isLeafNode)
    {
        int p = int(theta1 * key + theta2);
        p = (p > m_datasetSize - 1 ? m_datasetSize - 1 : p);
        p = (p < 0 ? 0 : p);
        // exponential search to find start_idx and end_idx
        int start_idx;
        int end_idx;
        if (m_dataset[p].first < key)
        {
            int i = 1;
            while (p + i < m_datasetSize && m_dataset[p + i].first < key)
                i *= 2;
            start_idx = p + i / 2;
            end_idx = min(p + i, m_datasetSize - 1);
        }
        else
        {
            int i = 1;
            while (p - i >= 0 && m_dataset[p - i].first >= key)
                i *= 2;
            start_idx = max(0, p - i);
            end_idx = p - i / 2;
        }

        // binary search
        while (start_idx < end_idx)
        {
            int mid = (start_idx + end_idx) / 2;
            if (m_dataset[mid - 1].first < key && m_dataset[mid].first >= key)
            {
                p = mid;
                break;
            }
            if (m_dataset[mid].first > key)
                end_idx = mid;
            else
                start_idx = mid + 1;
        }
        m_dataset.push_back(m_dataset[m_datasetSize - 1]);
        m_datasetSize++;
        for (int i = m_datasetSize - 2; i > p; i--)
        {
            m_dataset[i] = m_dataset[i - 1];
        }
        m_dataset[p] = {key, data};
        if (m_datasetSize > capacity)
        {
            isLeafNode = false;
            vector<pair<double, double>> newIndex;
            for (int i = 0; i < m_datasetSize / 2; i++)
            {
                children[0]->initialInsert(m_dataset[i]);
            }
            for (int i = m_datasetSize / 2; i < m_datasetSize; i++)
            {
                children[0]->initialInsert(m_dataset[i]);
            }
            newIndex.push_back({m_dataset[m_datasetSize/2 - 1], double(0));
            newIndex.push_back({m_dataset[m_datasetSize - 1], double(1));
            childNum = 2;
            m_dataset.clear();
            m_dataset = newIndex;
            m_datasetSize = childNum;
        }
        return {key, data};
    }
    else
    {
        double index = -1;
        for (int i = 0; i < m_datasetSize; i++)
        {
            if (key < m_dataset[i].first)
            {
                index = m_dataset[i].second;
                break;
            }
        }
        if (index == -1)
        {
            cout << "Fail to find the key" << endl;
            return {-1, -1};
        }
        else
            return {-999, index};
    }
}
*/

// delete successfully, return {key, -1}
// fail to find the key, return {-1, -1}
// if this is innerNode, return {-999, index}
pair<double, double> secondStageWithSplit::del(double key)
{
    if (isLeafNode)
    {
        int p = findIndex(key);
        if (p == -1)
            return {-1, -1};
        else
        {
            for (; p < m_datasetSize - 1; p++)
            {
                m_dataset[p] = m_dataset[p + 1];
            }
            m_dataset.pop_back();
            return {key, -1};
        }
    }
    else
    {
        double index = -1;
        for (int i = 0; i < m_datasetSize; i++)
        {
            if (key < m_dataset[i].first)
            {
                index = m_dataset[i].second;
                break;
            }
        }
        if (index == -1)
        {
            cout << "Fail to find the key" << endl;
            return {-1, -1};
        }
        else
            return {-999, index};
    }
}

// use model to predict the index
// if this is leaf node, then update the data, return new data
// else return the index of child(next layer), {-999, index}
pair<double, double> secondStageWithSplit::update(double key, double data)
{
    if (isLeafNode)
    {
        int p = findIndex(key);
        if (p == -1)
            return {-1, -1};
        else
        {
            m_dataset[p].second = data;
            return m_dataset[p];
        }
    }
    else
    {
        double index = -1;
        for (int i = 0; i < m_datasetSize; i++)
        {
            if (key < m_dataset[i].first)
            {
                index = m_dataset[i].second;
                break;
            }
        }
        if (index == -1)
        {
            cout << "Fail to find the key" << endl;
            return {-1, -1};
        }
        else
            return {-999, index};
    }
}

// train leafNode
void secondStageWithSplit::train()
{
    if (m_datasetSize == 0)
    {
        cout << "This stage is empty!" << endl;
        return;
    }
    for (int i = 0; i < 5000; i++)
    {
        double error1 = 0.0;
        double error2 = 0.0;
        for (int j = 0; j < m_datasetSize; j++)
        {
            double p = theta1 * m_dataset[j].first + theta2;
            p = (p > m_datasetSize - 1 ? m_datasetSize - 1 : p);
            p = (p < 0 ? 0 : p);
            double y = j;
            error2 += p - y;
            error1 += (p - y) * m_dataset[j].first;
        }
        theta1 = theta1 - 0.000001 * error1 / m_datasetSize;
        theta2 = theta2 - 0.000001 * error2 / m_datasetSize;

        double loss = 0.0;
        for (int j = 0; j < m_datasetSize; j++)
        {
            double p = theta1 * m_dataset[j].first + theta2;
            p = (p > m_datasetSize - 1 ? m_datasetSize - 1 : p);
            p = (p < 0 ? 0 : p);
            double y = j;
            loss += (p - y) * (p - y);
        }
        loss = loss / (m_datasetSize * 2);
        // cout << "iteration " << i << "    loss is:" << loss << endl;
    }

    // for (int i = 0; i < m_datasetSize; i++)
    // {
    //     double p = predict(m_dataset[i].first);
    //     p = (p > m_dataset[m_datasetSize].second ? m_dataset[m_datasetSize].second : p);
    //     p = (p < m_dataset[0].second ? m_dataset[0].second : p);
    //     int y = int(m_dataset[i].second);
    //     int error = y - int(p);
    //     if (error > maxPositiveError)
    //         maxPositiveError = error;
    //     if (error < maxNegativeError)
    //         maxNegativeError = error;
    // }
}

// split when m_datasetSize > capacity
void secondStageWithSplit::split()
{
    isLeafNode = false;
    int index = 0;
    vector<pair<double, double>> newIndex;
    int cnt = 0;
    for (int i = 0; i < m_datasetSize; i++)
    {
        children[index]->initialInsert(m_dataset[i]); // insert data into children, {key, value}
        cnt++;
        if (cnt == 100 || i == m_datasetSize - 1)
        {
            newIndex.push_back({m_dataset[i].first, double(index)}); // record the maximum of current node, {maximum, index}
            index++;
            cnt = 0;
        }
    }
    childNum = index;
    m_dataset.clear();
    m_dataset = newIndex;
    m_datasetSize = childNum;
}

class segModelWithSplit
{
public:
    segModelWithSplit(vector<pair<double, double>> &dataset)
    {
        m_dataset = dataset;
        sortData();
    }
    void sortData()
    {
        sort(m_dataset.begin(), m_dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
            return p1.first < p2.first;
        });
    }

    void preProcess()
    {
        for (int i = 0; i < 100; i++)
        {
            secondStageWithSplit *second = new secondStageWithSplit();
            m_model.push_back(second);
        }
        for (int i = 0; i < m_dataset.size(); i++)
        {
            m_model[int(m_dataset[i].first) / 100]->initialInsert({m_dataset[i].first, i});
        }
        m_dataset.clear();
    }

    pair<double, double> find(double key);
    bool update(double key, double data);
    bool insert(double key, double data);
    bool deleteData(double key);

private:
    vector<pair<double, double>> m_dataset;
    vector<secondStageWithSplit *> m_model;
};

pair<double, double> segModelWithSplit::find(double key)
{
    int idx = int(key) / 100;
    pair<double, double> res = m_model[idx]->find(key);
    if (res.first == -999)
    {
        secondStageWithSplit *tmp = m_model[idx];
        while (res.first == -999)
        {
            tmp = tmp->children[int(res.second)];
            res = tmp->find(key);
        }
        return res;
    }
    else
    {
        return res;
    }
}

bool segModelWithSplit::update(double key, double data)
{
    int idx = int(key) / 100;
    pair<double, double> res = m_model[idx]->update(key, data);
    if (res.first == -999)
    {
        secondStageWithSplit *tmp = m_model[idx];
        while (res.first == -999)
        {
            tmp = tmp->children[int(res.second)];
            res = tmp->update(key, data);
        }
        return true;
    }
    else if(res.first == key)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool segModelWithSplit::insert(double key, double data)
{
    int idx = int(key) / 100;
    pair<double, double> res = m_model[idx]->insert(key, data);
    if (res.first == -999)
    {
        secondStageWithSplit *tmp = m_model[idx];
        while (res.first == -999)
        {
            tmp = tmp->children[int(res.second)];
            res = tmp->insert(key, data);
        }
        return true;
    }
    else if(res.first == key)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool segModelWithSplit::deleteData(double key)
{
    int idx = int(key) / 100;
    pair<double, double> res = m_model[idx]->del(key);
    if (res.first == -999)
    {
        secondStageWithSplit *tmp = m_model[idx];
        while (res.first == -999)
        {
            tmp = tmp->children[int(res.second)];
            res = tmp->del(key);
        }
        return true;
    }
    else if(res.first == key)
    {
        return true;
    }
    else
    {
        return false;
    }    
}

#endif