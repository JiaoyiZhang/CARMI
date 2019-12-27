#ifndef GAPPED_ARRAY_H
#define GAPPED_ARRAY_H

#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>
using namespace std;

class gappedArray
{
public:
    gappedArray(const vector<pair<double, double>> &dataset, int maxNum)
    {
        maxKeyNum = 0;
        m_dataset = dataset;
        maxKeyNum = maxNum;
        m_datasetSize = m_dataset.size();
        childNumber = ceil((1.0 * m_datasetSize) / (1 * maxKeyNum));
        if (m_datasetSize > maxKeyNum)
            isLeafNode = false;
        else
            isLeafNode = true;

        train(m_datasetSize);
    }
    bool insert(pair<double, double> data);

    pair<double, double> find(double key);

private:
    void expand()
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

    int predict(double key)
    {
        int p = int(theta1 * key + theta2);
        p = (p > m_datasetSize - 1 ? m_datasetSize - 1 : p);
        p = (p < 0 ? 0 : p);
        return p;
    }

    void train(int size);

    vector<pair<double, double>> m_dataset;
    int m_datasetSize;
    int maxKeyNum;
    int childNumber;
    bool isLeafNode;

    double theta1;
    double theta2;

    vector<gappedArray> children;
};

bool gappedArray::insert(pair<double, double> data)
{
    // If an additional element will push the gapped
    // array over its density bound d , then the gapped array expands.
    if (m_datasetSize * 1.0 / maxKeyNum >= 2.0 / 3.0)
        expand();
    // use the RMI to predict the insertion position
    int preIdx = predict(data.first);
    if (!isLeafNode)
    {
        gappedArray tmp = children[preIdx];
        while (!tmp.isLeafNode)
        {
            preIdx = tmp.predict(data.first);
            tmp = tmp.children[preIdx];
        }
        preIdx = tmp.predict(data.first);
        // If the insertion position is not a gap, we make
        // a gap at the insertion position by shifting the elements
        // by one position in the direction of the closest gap
        if (tmp.m_dataset[preIdx].first != -1)
        {
            int j = preIdx;
            while (tmp.m_dataset[j].first != -1)
            {
                j++;
            }
            for (int i = j; i > preIdx; i--)
            {
                tmp.m_dataset[i] = tmp.m_dataset[i - 1];
            }
        }
        // insert the element into the gap
        tmp.m_dataset[preIdx] = data;
    }
    else
    {
        m_dataset[preIdx] = data;
    }
}

void gappedArray::train(int size)
{
    if (m_datasetSize == 0)
    {
        cout << "This stage is empty!" << endl;
        return;
    }
    double factor = size * 1.0 / m_datasetSize;
    for (int i = 0; i < 5000; i++)
    {
        double error1 = 0.0;
        double error2 = 0.0;
        for (int j = 0; j < m_datasetSize; j++)
        {
            double p = theta1 * m_dataset[j].first + theta2;
            p = (p > size - 1 ? size - 1 : p);
            p = (p < 0 ? 0 : p);
            double y = m_dataset[j].second * factor;
            error2 += p - y;
            error1 += (p - y) * m_dataset[j].first;
        }
        theta1 = theta1 - 0.000001 * error1 / m_datasetSize;
        theta2 = theta2 - 0.000001 * error2 / m_datasetSize;

        double loss = 0.0;
        for (int j = 0; j < m_datasetSize; j++)
        {
            double p = theta1 * m_dataset[j].first + theta2;
            p = (p > size - 1 ? size - 1 : p);
            p = (p < 0 ? 0 : p);
            double y = m_dataset[j].second * factor;
            loss += (p - y) * (p - y);
        }
        loss = loss / (m_datasetSize * 2);
    }
    if (!isLeafNode)
    {
        vector<pair<double, double>> nextStageDataset;
        int nowNum = 0;
        for (int i = 0; i < m_datasetSize; i++)
        {
            double p = theta1 * m_dataset[i].first + theta2;
            p = (p > size - 1 ? size - 1 : p);
            p = (p < 0 ? 0 : p);
            nextStageDataset.push_back(m_dataset[i]);
            if (int(p / maxKeyNum) > nowNum)
            {
                gappedArray ga = gappedArray(nextStageDataset, maxKeyNum);
                children.push_back(ga);
                nextStageDataset.clear();
                nowNum++;
            }
        }
        m_dataset.clear();
        m_datasetSize = 0;
    }
}

pair<double, double> gappedArray::find(double key)
{
    int preIdx = predict(key);
    if (!isLeafNode)
    {
        gappedArray tmp = children[preIdx];
        while (!tmp.isLeafNode)
        {
            preIdx = tmp.predict(key);
            tmp = tmp.children[preIdx];
        }
        preIdx = tmp.predict(key);
        // a later model-based lookup will result in a
        // direct hit, thus we can do a lookup in O (1)
        if (tmp.m_dataset[preIdx].first == key)
            return tmp.m_dataset[preIdx];
        else
        {
            int start, end;
            //do exponential search to find the actual insertion position
            if (tmp.m_dataset[preIdx].first > key)
            {
                int i = preIdx - 1;
                while (i >= 0 && tmp.m_dataset[i].first >= key)
                {
                    i -= preIdx - i;
                }
                start = max(0, i);
                end = preIdx - (preIdx - i) / 2;
            }
            else
            {
                int i = preIdx + 1;
                while (i < tmp.m_datasetSize && tmp.m_dataset[i].first <= key)
                    i += i - preIdx;

                start = preIdx + (i - preIdx) / 2;
                end = min(i, tmp.m_datasetSize);
            }
            // do binary search for the bound range
            int res = -1;
            while (start < end)
            {
                int mid = (start + end) / 2;
                if (tmp.m_dataset[mid].first == key)
                {
                    res = mid;
                    break;
                }
                else if (tmp.m_dataset[mid].first > key)
                    end = mid;
                else
                    start = mid + 1;
            }

            if (res != -1)
                return {key, tmp.m_dataset[res].second};
            else
                return {};
        }
    }
    else
    {
        // a later model-based lookup will result in a
        // direct hit, thus we can do a lookup in O (1)
        if (m_dataset[preIdx].first == key)
            return m_dataset[preIdx];
        else
        {
            int start, end;
            //do exponential search to find the actual insertion position
            if (m_dataset[preIdx].first > key)
            {
                int i = preIdx - 1;
                while (i >= 0 && m_dataset[i].first >= key)
                {
                    i -= preIdx - i;
                }
                start = max(0, i);
                end = preIdx - (preIdx - i) / 2;
            }
            else
            {
                int i = preIdx + 1;
                while (i < m_datasetSize && m_dataset[i].first <= key)
                    i += i - preIdx;

                start = preIdx + (i - preIdx) / 2;
                end = min(i, m_datasetSize);
            }
            // do binary search for the bound range
            int res = -1;
            while (start < end)
            {
                int mid = (start + end) / 2;
                if (m_dataset[mid].first == key)
                {
                    res = mid;
                    break;
                }
                else if (m_dataset[mid].first > key)
                    end = mid;
                else
                    start = mid + 1;
            }

            if (res != -1)
                return {key, m_dataset[res].second};
            else
                return {};
        }
    }
}

#endif