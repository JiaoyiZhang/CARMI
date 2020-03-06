#ifndef GAPPED_ARRAY_H
#define GAPPED_ARRAY_H

#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>
using namespace std;
static const int maxKeyNum = 600;
class node
{
public:
    node()
    {
        theta1 = 4000;
        theta2 = 0.666;
        density = 0.75;
        capacity = 400;
        m_datasetSize = 0;
        vector<pair<double, double>> tmp(maxKeyNum + 1, {-1, -1});
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
        {
            int idx = max(capacity, m_datasetSize);
            while (m_dataset[idx - 1].first == -1)
                idx--;
            return m_dataset[idx - 1].first;
        }
        else
        {
            return children[childNumber - 1]->getMaxData();
        }
    }

    pair<double, double> insert(pair<double, double> data);
    void insertLeaf(pair<double, double> data, int idx);
    void insertData(vector<pair<double, double>> &vec, pair<double, double> data, int idx, int &cnt);

    pair<double, double> find(double key);
    pair<double, double> update(double key, double value);
    pair<double, double> del(double key);
    int search(double key, int p);
  
    void expand(); // only for leaf node
    void split();  // only for leaf node

    void train(int size);

    void print()
    {
        cout << "This node's structure is as follows:" << endl;
        cout << "isLeafNode:" << isLeafNode << "    childNumber:" << childNumber << endl;
        cout << "linear regression params:" << theta1 << "    " << theta2 << endl;
        cout << "This stage's datasetSize is:" << m_datasetSize << "    capacity is:" << capacity << endl;
        if (isLeafNode)
        {
            for (int i = 0; i < max(capacity, m_datasetSize); i++)
            {
                cout << m_dataset[i].first << "  |  ";
                if ((i + 1) % 10 == 0)
                    cout << endl;
            }
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
        if (p < (capacity / 2))
            return {-999, 0};
        else
            return {-999, 1};
    }
    else
    {
        // a later model-based lookup will result in a
        // direct hit, thus we can do a lookup in O (1)
        if (m_dataset[p].first == key)
            return m_dataset[p];
        else
        {
            int res = search(key, p);
            if (res == -1)
                cout << "Find data failed!" << endl;
            return m_dataset[res];
        }
    }
}

pair<double, double> node::update(double key, double value)
{
    int p = int(theta1 * key + theta2);
    int maxIdx = max(capacity, m_datasetSize);
    p = (p > maxIdx - 1 ? maxIdx - 1 : p);
    p = (p < 0 ? 0 : p);
    if (!isLeafNode)
    {
        if (p < (capacity / 2))
            return {-999, 0};
        else
            return {-999, 1};
    }
    else
    {
        if (m_dataset[p].first == key)
        {
            m_dataset[p].second = value;
            return m_dataset[p];
        }
        else
        {
            auto res = search(key, p);
            m_dataset[res].second = value;
            return m_dataset[res];
        }
    }
}

pair<double, double> node::del(double key)
{
    int p = int(theta1 * key + theta2);
    int maxIdx = max(capacity, m_datasetSize);
    p = (p > maxIdx - 1 ? maxIdx - 1 : p);
    p = (p < 0 ? 0 : p);
    if (!isLeafNode)
    {
        if (p < (capacity / 2))
            return {-999, 0};
        else
            return {-999, 1};
    }
    else
    {
        if (m_dataset[p].first == key)
        {
            m_dataset[p] = {-1, -1};
            return {key, -1};
        }
        else
        {
            m_dataset[search(key, p)] = {-1, -1};
            return {key, -1};
        }
    }
}

int node::search(double key, int p)
{
    /*
    0. Use p to get a approximate position of key
    1. Exponential Search at first, and decide the range of binary search
    2. Use Binary Search to search the gap array.
    */

    int start_idx, end_idx;
    int maxIdx = max(capacity, m_datasetSize);
    while (m_dataset[p].first == -1)
    {
        p++;
        if (p >= maxIdx)
            p = 0;
    }
    if (m_dataset[p].first > key)
    {
        int offset = 1;
        int i = p;
        while (i >= 0 && m_dataset[i].first > key)
        {
            i = p - offset;
            offset = offset << 1;
            while (m_dataset[i].first == -1)
                i++;
        }
        start_idx = max(0, i);
        end_idx = p - (offset / 4);
        while (m_dataset[end_idx].first == -1)
            end_idx++;
    }
    else
    {
        int offset = 1;
        int i = p;
        while (i < maxIdx && m_dataset[i].first < key)
        {
            i = p + offset;
            offset = offset << 1;
            while (m_dataset[i].first == -1)
                i++;
        }
        start_idx = p + (offset >> 2);
        end_idx = min(maxIdx, i);
    }
    // cout << "start idx is: " << start_idx << "    end idx is:" << end_idx << endl;

    while (start_idx <= end_idx)
    {
        int mid = (start_idx + end_idx) >> 1;
        while (m_dataset[mid].first == -1 && mid < maxIdx - 1)
            mid++;
        if (mid == (end_idx + 1))
        {
            mid--;
            while ((m_dataset[mid].first == -1) && (mid >= start_idx))
            {
                mid--;
            }
        }

        if (mid == maxIdx - 1)
        {
            mid = (start_idx + end_idx) >> 1;
            for (; m_dataset[mid].first == -1 && mid > start_idx; mid--)
            {
            }
        }
        // cout << "leftIdx: " << start_idx << "    " << m_dataset[start_idx].first << endl;
        // cout << "end_idx is:" << end_idx << "    " << m_dataset[end_idx].first << endl;
        // cout << "mid:" << mid << "    " << m_dataset[mid].first << endl;
        // cout << endl;

        double tmp = m_dataset[mid].first;
        if (key < tmp)
        {
            end_idx = mid - 1;
        }
        else if (key > tmp)
        {
            start_idx = mid + 1;
        }
        else
        {
            return mid;
        }
    }
    return -1;
}

void node::insertLeaf(pair<double, double> data, int p)
{
    // If the insertion position is not a gap, we make
    // a gap at the insertion position by shifting the elements
    // by one position in the direction of the closest gap
    int start_idx, end_idx;
    int maxIdx = max(capacity, m_datasetSize);
    while (m_dataset[p].first == -1)
    {
        p++;
        if (p >= maxIdx)
            p = 0;
    }
    if (m_dataset[p].first > data.first)
    {
        int offset = 1;
        int i = p;
        while (i >= 0 && m_dataset[i].first > data.first)
        {
            i = p - offset;
            offset = offset << 1;
            while (m_dataset[i].first == -1)
                i++;
        }
        start_idx = max(0, i);
        end_idx = p - (offset / 4);
        while (m_dataset[end_idx].first == -1)
            end_idx++;
    }
    else
    {
        int offset = 1;
        int i = p;
        while (i < maxIdx && m_dataset[i].first < data.first)
        {
            i = p + offset;
            offset = offset << 1;
            while (m_dataset[i].first == -1)
                i++;
        }
        start_idx = p + (offset >> 2);
        end_idx = min(maxIdx, i);
    }
    // cout << endl;
    // cout << "start idx is: " << start_idx << "    end idx is:" << end_idx << endl;

    int cnt = 0;
    while (start_idx <= end_idx)
    {
        cnt = 0;
        for (int i = start_idx; i <= end_idx; i++)
        {
            if (m_dataset[i].first != -1)
                cnt++;
        }
        if (cnt <= 2)
            break;

        int mid = (start_idx + end_idx) >> 1;

        while (m_dataset[mid].first == -1 && mid < maxIdx - 1)
            mid++;

        if (mid == maxIdx - 1)
        {
            mid = (start_idx + end_idx) >> 1;
            for (; m_dataset[mid].first == -1 && mid > start_idx; mid--)
            {
            }
        }

        double tmp = m_dataset[mid].first;
        if (data.first < tmp)
        {
            if (end_idx == mid)
            {
                while (m_dataset[start_idx + 1].first == -1)
                    start_idx++;
                while (m_dataset[end_idx - 1].first == -1)
                    end_idx--;
            }
            else
                end_idx = mid;
        }
        else if (data.first > tmp)
        {
            start_idx = mid;
        }
    }
    while (cnt == 2 && m_dataset[end_idx].first == -1)
        end_idx--;
    while (cnt == 2 && m_dataset[start_idx].first == -1)
        start_idx++;

    if (cnt == 2)
    {
        if (m_dataset[end_idx].first < data.first)
            end_idx++;
        else if (m_dataset[start_idx].first > data.first)
            end_idx = start_idx;
    }

    // cout << "leftIdx: " << start_idx << "    " << m_dataset[start_idx].first << endl;
    // cout << "end_idx is:" << end_idx << "    " << m_dataset[end_idx].first << endl;
    if (end_idx > 0 && m_dataset[end_idx - 1].first == -1)
    {
        // cout << "Insert index is: " << end_idx - 1 << endl;
        m_dataset[end_idx - 1] = data;
    }
    else
    {
        int j = end_idx + 1;
        while (j < maxIdx - 1 && m_dataset[j].first != -1)
            j++;
        if (j == maxIdx - 1)
        {
            j = end_idx - 1;
            while (j >= 0 && m_dataset[j].first != -1)
                j--;
            for (int i = j; i < end_idx - 1; i++)
                m_dataset[i] = m_dataset[i + 1];
            // cout << "Insert index is: " << end_idx - 1 << endl;
            m_dataset[end_idx - 1] = data;
        }
        else
        {
            for (int i = j; i > end_idx; i--)
            {
                m_dataset[i] = m_dataset[i - 1];
            }
            // cout << "Insert index is: " << end_idx << endl;
            m_dataset[end_idx] = data;
        }
    }
    m_datasetSize++;
}

void node::insertData(vector<pair<double, double>> &vec, pair<double, double> data, int idx, int &cnt)
{
    cnt++;
    int maxIdx;
    // cout << "In insertData: cnt is:" << cnt << "  data:" << data.first << "  idx:" << idx << endl;
    if (capacity == cnt)
        maxIdx = capacity + 1;
    else
        maxIdx = max(capacity, cnt);
    while (vec[idx].first != -1 && idx < maxIdx)
    {
        idx++;
    }
    if (idx == maxIdx - 1)
    {
        int j = idx - 1;
        while (vec[j].first != -1)
            j--;
        for (; j < idx - 1; j++)
        {
            vec[j] = vec[j + 1];
        }
        idx--;
    }
    vec[idx] = data;
}

pair<double, double> node::insert(pair<double, double> data)
{

    int p = int(theta1 * data.first + theta2);
    int maxIdx = max(capacity, m_datasetSize);
    p = (p > maxIdx - 1 ? maxIdx - 1 : p);
    p = (p < 0 ? 0 : p);
    if (!isLeafNode)
    {
        if (p < (capacity / 2))
            return {-999, 0};
        else
            return {-999, 1};
    }
    else
    {
        // cout << "In insert, datasetSize is:" << m_datasetSize << "  capacity is:" << capacity << endl;
        if (m_datasetSize >= maxKeyNum)
        {
            // cout << "This node need to be split!" << endl;
            split();
            p = min(p, capacity);
            if (p < (capacity / 2))
                return {-999, 0};
            else
                return {-999, 1};
        }
        else if (m_datasetSize * 1.0 / capacity >= density)
        {
            expand();
        }
        insertLeaf(data, p);
        return data;
    }
}

// expand the dataset when m_datasetSize / capacity >= density
void node::expand()
{
    int newSize = capacity * 1.2;
    if (newSize > maxKeyNum)
        return;
    theta1 *= 1.2;
    theta2 *= 1.2;

    // retrain model corresponding to this leaf node
    // The models at the upper levels of the RMI are not retrained in this event.
    train(newSize);
    capacity = newSize;

    // do model-based inserts of all the elements in this node using the retrained RMI
    vector<pair<double, double>> newDataset(maxKeyNum + 1, pair<double, double>{-1, -1});
    int cnt = 0;
    for (int i = 0; i < maxKeyNum; i++)
    {
        if (m_dataset[i].first != -1)
        {
            int p = int(theta1 * m_dataset[i].first + theta2);
            int maxIdx = max(newSize, m_datasetSize);
            p = (p > maxIdx - 1 ? maxIdx - 1 : p);
            p = (p < 0 ? 0 : p);
            insertData(newDataset, m_dataset[i], p, cnt);
        }
    }
    m_dataset = newDataset;
    if (m_datasetSize * 1.0 / capacity >= density)
        expand();
}

// split this model when m_datasetSize >= maxKeyNum
void node::split()
{
    node *tmp0 = new node();
    node *tmp1 = new node();
    int idx = 0;
    for (int i = 0; i < m_datasetSize; i++)
    {
        if (m_dataset[idx].first == -1)
            i--;
        else
        {
            int p = int(theta1 * m_dataset[idx].first + theta2);
            p = (p > capacity - 1 ? capacity - 1 : p);
            p = (p < 0 ? 0 : p);
            if (p < (capacity / 2))
            {
                insertData(tmp0->m_dataset, m_dataset[idx], p * 2, tmp0->m_datasetSize);
            }
            else
            {
                insertData(tmp1->m_dataset, m_dataset[idx], (p - capacity / 2) * 2, tmp1->m_datasetSize);
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
    // cout << "Split finish!" << endl;
}

// use linear regression to train this node
void node::train(int size)
{
    if (m_datasetSize == 0)
    {
        cout << "This stage is empty!" << endl;
        return;
    }
    cout << "Training dataset, datasetsize is " << m_datasetSize << endl;
    double lr = 0.01;
    int maxIdx = max(capacity, m_datasetSize);
    double factor = size * 1.0 / maxIdx;
    for (int i = 0; i < 50000; i++)
    {
        double error1 = 0.0;
        double error2 = 0.0;
        for (int j = 0; j < maxIdx; j++)
        {
            if (m_dataset[j].first != -1)
            {
                double p = theta1 * m_dataset[j].first + theta2;
                p = (p > maxIdx - 1 ? maxIdx - 1 : p);
                p = (p < 0 ? 0 : p);
                double y = j * factor;
                error2 += p - y;
                error1 += (p - y) * m_dataset[j].first;
            }
        }
        theta1 = theta1 - lr * error1 / m_datasetSize;
        theta2 = theta2 - lr * error2 / m_datasetSize;

        double loss = 0.0;
        for (int j = 0; j < maxIdx; j++)
        {
            if (m_dataset[j].first != -1)
            {
                double p = theta1 * m_dataset[j].first + theta2;
                p = (p > maxIdx - 1 ? maxIdx - 1 : p);
                p = (p < 0 ? 0 : p);
                double y = j * factor;
                loss += (p - y) * (p - y);
            }
        }
        loss = loss / (m_datasetSize * 2);
    }
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

    bool update(double key, double value);

    bool del(double key);

    pair<double, double> find(double key);

    void printStructure()
    {
        cout << "The adaptive rmi model's structure is as follows:" << endl;
        cout << "Root:" << endl;
        for (int i = 0; i < root->childNumber; i++)
        {
            cout << "TOTAL CHILD: " << i << endl;
            root->children[i]->print();
            cout << "88888888888888888888888888888888888888" << endl;
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
    for (int i = 0; i < root->childNumber; i++)
    {
        if (data.first < root->children[i]->getMaxData() || i == root->childNumber - 1)
        {
            node *tmp = root->children[i];
            pair<double, double> res = tmp->insert(data);
            while (res.first != data.first)
            {
                tmp = tmp->children[int(res.second)];
                res = tmp->insert(data);
            }
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
            node *tmp = root->children[i];
            pair<double, double> res = tmp->find(key);
            while (res.first != key)
            {
                tmp = tmp->children[int(res.second)];
                res = tmp->find(key);
            }
            // cout << "Out of while, the res is " << res.first << "    " << res.second << endl;
            return res;
        }
    }
}

bool gappedArray::update(double key, double value)
{
    for (int i = 0; i < root->childNumber; i++)
    {
        if (key <= root->children[i]->getMaxData())
        {
            node *tmp = root->children[i];
            pair<double, double> res = tmp->update(key, value);
            while (res.first != key)
            {
                tmp = tmp->children[int(res.second)];
                res = tmp->update(key, value);
            }
            // cout << "Out of while, the res is " << res.first << "    " << res.second << endl;
            return true;
        }
    }
    return false;
}

bool gappedArray::del(double key)
{
    for (int i = 0; i < root->childNumber; i++)
    {
        if (key <= root->children[i]->getMaxData())
        {
            // cout << "find key is:" << key << "    in root->child idx " << i << endl;
            node *tmp = root->children[i];
            pair<double, double> res = tmp->del(key);
            while (res.first != key)
            {
                tmp = tmp->children[int(res.second)];
                res = tmp->del(key);
            }
            // cout << "Out of while, the res is " << res.first << "    " << res.second << endl;
            return true;
        }
    }
    return false;
}

#endif