#ifndef GAPPED_ARRAY_H
#define GAPPED_ARRAY_H

#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>
using namespace std;
class node
{
public:
    node()
    {
        theta1 = 4000;
        theta2 = 0.666;
    }
    void getData(const vector<pair<double, double>> &dataset, int maxNum);
    int getDatasetSize() { return m_datasetSize; }
    pair<double, double> find(double key);
    void insert(pair<double, double> data);
    int predict(double key);
    void expand();
    void train(int size);

    void print()
    {
        cout << "This node's structure is as follows:" << endl;
        cout << "isLeafNode:" << isLeafNode << "    childNumber:" << childNumber << "    maxKeyNum:" << maxKeyNum << endl;
        cout << "linear regression params:" << theta1 << "    " << theta2 << endl;
        cout << "This stage's datasetSize is:" << m_datasetSize << endl;
        for (int i = 0; i < m_datasetSize; i++)
        {
            cout << m_dataset[i].first << " " << m_dataset[i].second << "  |  ";
            if ((i + 1) % 5 == 0)
                cout << endl;
        }
        for (int i = 0; i < children.size(); i++)
        {
            cout << "child: " << i << endl;
            children[i].print();
        }
        cout << "_________________________________" << endl;
        cout << endl;
    }

private:
    vector<pair<double, double>> m_dataset;
    int m_datasetSize;
    int maxKeyNum;
    int childNumber;
    bool isLeafNode;

    double theta1;
    double theta2;

    vector<node> children;
};

void node::getData(const vector<pair<double, double>> &dataset, int maxNum)
{
    m_dataset = dataset;
    maxKeyNum = maxNum;
    m_datasetSize = m_dataset.size();
    // childNumber = ceil((1.0 * m_datasetSize) / (1 * maxKeyNum));
    if (m_datasetSize > maxKeyNum)
        isLeafNode = false;
    else
        isLeafNode = true;
}

pair<double, double> node::find(double key)
{
}

void node::insert(pair<double, double> data)
{
}

int node::predict(double key)
{
    int p = int(theta1 * key + theta2);
    p = (p > m_datasetSize - 1 ? m_datasetSize - 1 : p);
    p = (p < 0 ? 0 : p);
    return p;
}

void node::expand()
{
    int newSize = int(m_datasetSize * 1.5);
    theta1 *= 1.5;
    theta2 *= 1.5;

    // retrain model corresponding to this leaf node
    // The models at the upper levels of the RMI are not retrained in this event.
    train(newSize);

    // do model-based inserts of all the elements in this node using the retrained RMI
    for (int i = 0; i < m_datasetSize; i++)
    {
        insert(m_dataset[i]);
    }
}

void node::train(int size)
{

    if (m_datasetSize == 0)
    {
        cout << "This stage is empty!" << endl;
        return;
    }
    cout << "Training dataset, datasetsize is " << size << endl;
    double factor = size * 1.0 / m_datasetSize;
    double lr = 0.01;
    for (int i = 0; i < 5000; i++)
    {
        double error1 = 0.0;
        double error2 = 0.0;
        for (int j = 0; j < m_datasetSize; j++)
        {
            double p = theta1 * m_dataset[j].first + theta2;
            p = (p > size - 1 ? size - 1 : p);
            p = (p < 0 ? 0 : p);
            double y = j * factor;
            error2 += p - y;
            error1 += (p - y) * m_dataset[j].first;
        }
        theta1 = theta1 - lr * error1 / m_datasetSize;
        theta2 = theta2 - lr * error2 / m_datasetSize;

        double loss = 0.0;
        for (int j = 0; j < m_datasetSize; j++)
        {
            double p = theta1 * m_dataset[j].first + theta2;
            p = (p > size - 1 ? size - 1 : p);
            p = (p < 0 ? 0 : p);
            double y = j * factor;
            loss += (p - y) * (p - y);
        }
        loss = loss / (m_datasetSize * 2);
        // if ((i + 1) % 100 == 0)
        //     cout << "iteration: " << i << "    loss is " << loss << endl;
    }
    cout << "Train this node is over, params are:" << theta1 << ",    " << theta2 << endl;
    childNumber = 0;
    if (!isLeafNode)
    {
        vector<pair<double, double>> nextStageDataset;
        int nowNum = maxKeyNum;
        for (int i = 0; i < m_datasetSize; i++)
        {
            double p = theta1 * m_dataset[i].first + theta2;
            p = (p > size - 1 ? size - 1 : p);
            p = (p < 0 ? 0 : p);
            nextStageDataset.push_back(m_dataset[i]);
            // cout << "key is:" << m_dataset[i].first << "    p is:" << p << endl;
            // if ((int(p) >= nowNum) || (i == (m_datasetSize - 1)))
            if (((i + 1) >= nowNum) || (i == (m_datasetSize - 1)))
            {
                node tmp;
                tmp.getData(nextStageDataset, maxKeyNum);
                tmp.train(nextStageDataset.size());
                children.push_back(tmp);
                nextStageDataset.clear();
                nowNum += maxKeyNum;
            }
        }
        childNumber = children.size();
        m_dataset.clear();
        m_datasetSize = 0;
    }
}

class gappedArray
{
public:
    gappedArray(const vector<pair<double, double>> &dataset, int maxNum)
    {
        m_dataset = dataset;
        m_datasetSize = m_dataset.size();
        sortData();
        vector<pair<double, double>> subDataset;
        int nowNum = 400;
        for (int i = 0; i < m_dataset.size(); i++)
        {
            subDataset.push_back(m_dataset[i]);
            if ((i + 1) >= nowNum || i == (m_datasetSize - 1))
            {
                node tmp;
                tmp.getData(subDataset, maxNum);
                root.push_back(tmp);
                nowNum += 400;
                subDataset.clear();
            }
        }
        train();
    }
    bool insert(pair<double, double> data);

    pair<double, double> find(double key);

    void sortData()
    {
        sort(m_dataset.begin(), m_dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
            return p1.first < p2.first;
        });
    }
    void printStructure()
    {
        cout << "The adaptive rmi model's structure is as follows:" << endl;
        cout << "Root:" << endl;
        for (int i = 0; i < root.size(); i++)
        {
            root[i].print();
        }
    }

private:
    void train();

    vector<pair<double, double>> m_dataset;
    int m_datasetSize;

    vector<node> root;
};

bool gappedArray::insert(pair<double, double> data)
{
    // // If an additional element will push the gapped
    // // array over its density bound d , then the gapped array expands.
    // if (m_datasetSize * 1.0 / maxKeyNum >= 2.0 / 3.0)
    //     expand();
    // // use the RMI to predict the insertion position
    // int preIdx = predict(data.first);
    // if (!isLeafNode)
    // {
    //     gappedArray tmp = children[preIdx];
    //     while (!tmp.isLeafNode)
    //     {
    //         preIdx = tmp.predict(data.first);
    //         tmp = tmp.children[preIdx];
    //     }
    //     preIdx = tmp.predict(data.first);
    //     // If the insertion position is not a gap, we make
    //     // a gap at the insertion position by shifting the elements
    //     // by one position in the direction of the closest gap
    //     if (tmp.m_dataset[preIdx].first != -1)
    //     {
    //         int j = preIdx;
    //         while (tmp.m_dataset[j].first != -1)
    //         {
    //             j++;
    //         }
    //         for (int i = j; i > preIdx; i--)
    //         {
    //             tmp.m_dataset[i] = tmp.m_dataset[i - 1];
    //         }
    //     }
    //     // insert the element into the gap
    //     tmp.m_dataset[preIdx] = data;
    // }
    // else
    // {
    //     m_dataset[preIdx] = data;
    // }
}

void gappedArray::train()
{
    cout << "Start to train total model" << endl;
    for (int i = 0; i < root.size(); i++)
    {
        root[i].train(root[i].getDatasetSize());
    }
    cout << "End train" << endl;
}

pair<double, double> gappedArray::find(double key)
{
    // int preIdx = predict(key);
    // cout << "Now is finding: " << key << "    preIdx is: " << preIdx << endl;
    // if (!isLeafNode)
    // {
    //     gappedArray tmp = children[preIdx];
    //     while (!tmp.isLeafNode)
    //     {
    //         preIdx = tmp.predict(key);
    //         tmp = tmp.children[preIdx];
    //     }
    //     preIdx = tmp.predict(key);
    //     cout << "Not leaf node, final preIdx is" << preIdx << endl;
    //     // a later model-based lookup will result in a
    //     // direct hit, thus we can do a lookup in O (1)
    //     if (tmp.m_dataset[preIdx].first == key)
    //         return tmp.m_dataset[preIdx];
    //     else
    //     {
    //         int start, end;
    //         //do exponential search to find the actual insertion position
    //         if (tmp.m_dataset[preIdx].first > key)
    //         {
    //             int i = preIdx - 1;
    //             while (i >= 0 && tmp.m_dataset[i].first >= key)
    //             {
    //                 i -= preIdx - i;
    //             }
    //             start = max(0, i);
    //             end = preIdx - (preIdx - i) / 2;
    //         }
    //         else
    //         {
    //             int i = preIdx + 1;
    //             while (i < tmp.m_datasetSize && tmp.m_dataset[i].first <= key)
    //                 i += i - preIdx;

    //             start = preIdx + (i - preIdx) / 2;
    //             end = min(i, tmp.m_datasetSize);
    //         }
    //         // do binary search for the bound range
    //         int res = -1;
    //         while (start < end)
    //         {
    //             int mid = (start + end) / 2;
    //             if (tmp.m_dataset[mid].first == key)
    //             {
    //                 res = mid;
    //                 break;
    //             }
    //             else if (tmp.m_dataset[mid].first > key)
    //                 end = mid;
    //             else
    //                 start = mid + 1;
    //         }

    //         if (res != -1)
    //             return {key, tmp.m_dataset[res].second};
    //         else
    //             return {};
    //     }
    // }
    // else
    // {
    //     // a later model-based lookup will result in a
    //     // direct hit, thus we can do a lookup in O (1)
    //     if (m_dataset[preIdx].first == key)
    //         return m_dataset[preIdx];
    //     else
    //     {
    //         int start, end;
    //         //do exponential search to find the actual insertion position
    //         if (m_dataset[preIdx].first > key)
    //         {
    //             int i = preIdx - 1;
    //             while (i >= 0 && m_dataset[i].first >= key)
    //             {
    //                 i -= preIdx - i;
    //             }
    //             start = max(0, i);
    //             end = preIdx - (preIdx - i) / 2;
    //         }
    //         else
    //         {
    //             int i = preIdx + 1;
    //             while (i < m_datasetSize && m_dataset[i].first <= key)
    //                 i += i - preIdx;

    //             start = preIdx + (i - preIdx) / 2;
    //             end = min(i, m_datasetSize);
    //         }
    //         // do binary search for the bound range
    //         int res = -1;
    //         while (start < end)
    //         {
    //             int mid = (start + end) / 2;
    //             if (m_dataset[mid].first == key)
    //             {
    //                 res = mid;
    //                 break;
    //             }
    //             else if (m_dataset[mid].first > key)
    //                 end = mid;
    //             else
    //                 start = mid + 1;
    //         }

    //         if (res != -1)
    //             return {key, m_dataset[res].second};
    //         else
    //             return {};
    //     }
    // }
}

#endif