#ifndef GAPPED_ARRAY_H
#define GAPPED_ARRAY_H

#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>
using namespace std;
static const int maxKeyNum = 400;
class node
{
public:
    node()
    {
        theta1 = 4000;
        theta2 = 0.666;
        density = 3 / 4;
        capacity = 300;
        m_datasetSize = 0;
        vector<pair<double, double>> tmp(maxKeyNum, {-1, -1});
        m_dataset = tmp;
        isLeafNode = true;
        childNumber = 0;
    }
    void initRoot();
    void receiveData(const vector<pair<double, double>> &dataset);

    int getDatasetSize() { return m_datasetSize; }
    double getMaxData()
    {
        if (isLeafNode)
            return m_dataset[m_datasetSize - 1].first;
        else
        {
            return children[childNumber - 1]->getMaxData();
        }
    }

    void insert(pair<double, double> data);
    void insertLeaf(pair<double, double> data, int idx);
    void insertData(vector<pair<double, double>> &vec, pair<double, double> data, int idx);

    int predict(double key);

    pair<double, double> find(double key);
    int search(const vector<pair<double, double>> &data_, double key, int p);
    int gapArrayBinarySearch(const vector<pair<double, double>> &data, double key, int start_pos, int end_pos);

    void expand(); // only for leaf node
    void split();  // only for leaf node

    void train(int size);

    void print()
    {
        cout << "This node's structure is as follows:" << endl;
        cout << "isLeafNode:" << isLeafNode << "    childNumber:" << childNumber << endl;
        cout << "linear regression params:" << theta1 << "    " << theta2 << endl;
        cout << "This stage's datasetSize is:" << m_datasetSize << "    capacity is:" << capacity << endl;
        for (int i = 0; i < m_datasetSize; i++)
        {
            cout << m_dataset[i].first << "  |  ";
            if ((i + 1) % 5 == 0)
                cout << endl;
        }
        for (int i = 0; i < children.size(); i++)
        {
            cout << "child: " << i << endl;
            children[i]->print();
        }
        cout << "_________________________________" << endl;
        cout << endl;
    }

    vector<node *> children;
    int childNumber;
    bool isLeafNode;

private:
    double theta1;
    double theta2;

    int capacity;
    double density;

    vector<pair<double, double>> m_dataset;
    int m_datasetSize;
};

// split the total dataset into several partitions
void node::initRoot()
{
    isLeafNode = false;
    vector<pair<double, double>> subDataset;
    int nowNum = maxKeyNum;
    for (int i = 0; i < m_dataset.size(); i++)
    {
        subDataset.push_back(m_dataset[i]);
        if ((i + 1) >= nowNum || i == (m_datasetSize - 1))
        {
            node *tmp = new node();
            tmp->receiveData(subDataset);
            children.push_back(tmp);
            nowNum += maxKeyNum;
            subDataset.clear();
        }
    }
    childNumber = children.size();
}

void node::receiveData(const vector<pair<double, double>> &dataset)
{
    m_dataset = dataset;
    m_datasetSize = m_dataset.size();
    isLeafNode = true;
}

int node::predict(double key)
{
    int p = int(theta1 * key + theta2);
    int maxIdx = max(capacity, m_datasetSize);
    p = (p > maxIdx - 1 ? maxIdx - 1 : p);
    p = (p < 0 ? 0 : p);
    return p;
}

// use model to predict the index
// if this is leaf node, return {key, value}
// else return the index of child(next layer), {-999, index}
pair<double, double> node::find(double key)
{
    int p = int(theta1 * key + theta2);
    int maxIdx = max(capacity, m_datasetSize);
    p = (p > maxIdx - 1 ? maxIdx - 1 : p);
    p = (p < 0 ? 0 : p);
    if (!isLeafNode)
    {
        return {-999, p};
    }
    else
    {
        // a later model-based lookup will result in a
        // direct hit, thus we can do a lookup in O (1)
        if (m_dataset[p].first == key)
            return m_dataset[p];
        else
        {
            return m_dataset[search(m_dataset, key, p)];
        }
    }
}

int node::search(const vector<pair<double, double>> &data_, double key, int p)
{
    /*
    0. Use p to get a approximate position of key
    1. Exponential Search at first, and decide the range of binary search
    2. Use Binary Search to search the gap array.
    */

    int start_idx, end_idx;
    if (data_[p].first == -1)
    {
        p++;
        if (p >= data_.size())
            p -= 2;
    }
    if (data_[p].first > key)
    {
        int offset = 1;
        int i = p;
        while (i >= 0 && data_[i].first >= key)
        {
            i = p - offset;
            offset = offset << 1;
            if (data_[i].first == -1)
                i++;
        }
        start_idx = max(0, i);
        end_idx = p - (offset >> 1);
        if (data_[end_idx].first == -1)
            end_idx++;
    }
    else
    {
        int offset = 1;
        int i = p;
        while (i < data_.size() && data_[i].first <= key)
        {
            i = p + offset;
            offset = offset << 1;
            if (data_[i].first == -1)
                i++;
        }
        start_idx = p + (offset >> 1);
        end_idx = min(int(data_.size()), i);
    }
    return this->gapArrayBinarySearch(data_, key, start_idx, end_idx);
}

int node::gapArrayBinarySearch(const vector<pair<double, double>> &data_, double key, int start_pos, int end_pos)
{
    while (start_pos <= end_pos)
    {
        int mid = (start_pos + end_pos) >> 1;
        if (data_[mid].first == -1)
            mid++;

        if (mid == data_.size() - 1)
        {
            mid = (start_pos + end_pos) >> 1;
            for (; data_[mid].first == -1 && mid > start_pos; mid--)
            {
            }
        }

        double tmp = data_[mid].first;
        if (key < tmp)
        {
            end_pos = mid - 1;
        }
        else if (key > tmp)
        {
            start_pos = mid + 1;
        }
        else
        {
            return mid;
        }
    }
    return -1;
}

void node::insertLeaf(pair<double, double> data, int idx)
{
    // If the insertion position is not a gap, we make
    // a gap at the insertion position by shifting the elements
    // by one position in the direction of the closest gap
    if (m_dataset[idx].first != -1)
    {
        int j = idx;
        while (m_dataset[j].first != -1)
        {
            j++;
        }
        for (int i = j; i > idx; i--)
        {
            m_dataset[i] = m_dataset[i - 1];
        }
        // cout << "In insertLeaf: j is:" << j << endl;
    }
    // cout << "In insertLeaf: idx is:" << idx << endl;
    // insert the element into the gap
    m_dataset[idx] = data;
    m_datasetSize++;
}

void node::insertData(vector<pair<double, double>> &vec, pair<double, double> data, int idx)
{
    if (vec[idx].first != -1)
    {
        int j = idx;
        while (vec[j].first != -1)
            j++;
        for (int i = j; j < idx; i--)
            vec[i] = vec[i - 1];
    }
    vec[idx] = data;
}

void node::insert(pair<double, double> data)
{
    // cout << "In insert, datasetSize is:" << m_datasetSize << "  capacity is:" << capacity << endl;
    if (m_datasetSize >= maxKeyNum)
    {
        // cout << "This node need to be split!" << endl;
        int p = int(theta1 * data.first + theta2);
        int maxIdx = max(capacity, m_datasetSize);
        p = (p > maxIdx - 1 ? maxIdx - 1 : p);
        p = (p < 0 ? 0 : p);
        insertLeaf(data, p);
        split();
        return;
    }
    else if (m_datasetSize / capacity >= density)
    {
        // cout << "This node need to be expanded!" << endl;
        expand();
    }
    int p = int(theta1 * data.first + theta2);
    int maxIdx = max(capacity, m_datasetSize);
    p = (p > maxIdx - 1 ? maxIdx - 1 : p);
    p = (p < 0 ? 0 : p);
    insertLeaf(data, p);
}

// expand the dataset when m_datasetSize / capacity >= density
void node::expand()
{
    // capacity *= 1.5;
    int newSize = capacity * 1.5;
    theta1 *= 1.5;
    theta2 *= 1.5;

    // retrain model corresponding to this leaf node
    // The models at the upper levels of the RMI are not retrained in this event.
    train(newSize);

    // do model-based inserts of all the elements in this node using the retrained RMI
    vector<pair<double, double>> newDataset(newSize, pair<double, double>{-1, -1});
    capacity = newSize;
    for (int i = 0; i < capacity; i++)
    {
        if (m_dataset[i].first != -1)
        {
            int p = int(theta1 * m_dataset[i].first + theta2);
            int maxIdx = max(capacity, m_datasetSize);
            p = (p > maxIdx - 1 ? maxIdx - 1 : p);
            p = (p < 0 ? 0 : p);
            insertData(newDataset, m_dataset[i], p);
        }
    }
    m_dataset = newDataset;
}

// split this model when m_datasetSize >= maxKeyNum
void node::split()
{
    // cout << "Now is in split()" << endl;
    node *tmp0 = new node();
    node *tmp1 = new node();
    int idx = 0;
    for (int i = 0; i < m_datasetSize; i++)
    {
        // if it is a gap
        // cout << "deal with m_dataset[idx]: " << idx << endl;
        if (m_dataset[idx].first == -1)
            i--;
        else
        {
            int p = int(theta1 * m_dataset[idx].first + theta2);
            int maxIdx = max(capacity, m_datasetSize);
            p = (p > maxIdx - 1 ? maxIdx - 1 : p);
            p = (p < 0 ? 0 : p);
            // cout << "In split, p is: " << p << endl;
            if (p < (maxKeyNum / 2))
            {
                // cout << "to tmp0, new index is: " << p * 2 << endl;
                tmp0->insertLeaf(m_dataset[idx], p * 2);
                // cout << "finish insert" << endl;
            }
            else
            {
                // cout << "to tmp1, new index is: " << (p - maxKeyNum / 2) * 2 << endl;
                tmp1->insertLeaf(m_dataset[idx], (p - maxKeyNum / 2) * 2);
                // cout << "finish insert" << endl;
            }
        }
        idx++;
    }
    tmp0->train(tmp0->m_datasetSize);
    tmp1->train(tmp1->m_datasetSize);
    children.push_back(tmp0);
    children.push_back(tmp1);
    childNumber = 2;
    isLeafNode = false;
    m_dataset.clear();
    m_datasetSize = 0;
    // cout << "After split, tmp0's datasetSize is: " << tmp0->m_datasetSize << "    tmp1: " << tmp1->m_datasetSize << endl;
}

// use linear regression to train this node
void node::train(int size)
{
    if (m_datasetSize == 0)
    {
        cout << "This stage is empty!" << endl;
        return;
    }
    cout << "Training dataset, datasetsize is " << size << endl;
    double factor = size * 1.0 / capacity;
    double lr = 0.01;
    for (int i = 0; i < 10000; i++)
    {
        double error1 = 0.0;
        double error2 = 0.0;
        for (int j = 0; j < capacity; j++)
        {
            if (m_dataset[j].first != -1)
            {
                double p = theta1 * m_dataset[j].first + theta2;
                p = (p > size - 1 ? size - 1 : p);
                p = (p < 0 ? 0 : p);
                double y = j * factor;
                error2 += p - y;
                error1 += (p - y) * m_dataset[j].first;
            }
        }
        theta1 = theta1 - lr * error1 / m_datasetSize;
        theta2 = theta2 - lr * error2 / m_datasetSize;

        double loss = 0.0;
        for (int j = 0; j < m_datasetSize; j++)
        {
            if (m_dataset[j].first != -1)
            {
                double p = theta1 * m_dataset[j].first + theta2;
                p = (p > size - 1 ? size - 1 : p);
                p = (p < 0 ? 0 : p);
                double y = j * factor;
                loss += (p - y) * (p - y);
            }
        }
        loss = loss / (m_datasetSize * 2);
        // if ((i + 1) % 1000 == 0)
        //     cout << "iteration: " << i << "    loss is " << loss << endl;
    }
    cout << "Train this node is over, params are:" << theta1 << ",    " << theta2 << endl;
}

class gappedArray
{
public:
    gappedArray(const vector<pair<double, double>> &dataset)
    {
        m_dataset = dataset;
        m_datasetSize = m_dataset.size();
        sortData();
        root = new node();
        root->receiveData(m_dataset);
        root->initRoot();
        train();
    }

    void insert(pair<double, double> data);

    pair<double, double> find(double key);

    void printStructure()
    {
        cout << "The adaptive rmi model's structure is as follows:" << endl;
        cout << "Root:" << endl;
        for (int i = 0; i < root->childNumber; i++)
        {
            root->children[i]->print();
        }
    }

private:
    void sortData()
    {
        sort(m_dataset.begin(), m_dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
            return p1.first < p2.first;
        });
    }

    void train();

    vector<pair<double, double>> m_dataset;
    int m_datasetSize;

    node *root;
};

void gappedArray::insert(pair<double, double> data)
{
    // cout << endl;
    for (int i = 0; i < root->childNumber; i++)
    {
        if (data.first < root->children[i]->getMaxData())
        {
            // cout << "Insert data is:" << data.first << "    root->children:" << i << endl;
            node *tmp = root->children[i];
            int res = tmp->predict(data.first);
            while (tmp->isLeafNode == false)
            {
                tmp = tmp->children[res];
                res = tmp->predict(data.first);
            }
            tmp->insert(data);
            return;
        }
    }
}

// train the structure of  adaptive rmi
void gappedArray::train()
{
    cout << "Start to train total model" << endl;
    for (int i = 0; i < root->childNumber; i++)
    {
        root->children[i]->train(root->children[i]->getDatasetSize());
    }
    cout << "End train" << endl;
}

pair<double, double> gappedArray::find(double key)
{
    for (int i = 0; i < root->childNumber; i++)
    {
        if (key <= root->children[i]->getMaxData())
        {
            // cout << "find key is:" << key << "    in root->child idx " << i << endl;
            node *tmp = root->children[i];
            pair<double, double> res = tmp->find(key);
            while (res.first != key)
            {
                tmp = tmp->children[int(res.second)];
                res = tmp->find(key);
                // cout << "In while, the res is " << res.first << "    " << res.second << endl;
            }
            // cout << "Out of while, the res is " << res.first << "    " << res.second << endl;
            return res;
        }
    }
}

#endif