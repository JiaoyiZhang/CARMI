#ifndef HistogramModel_H
#define HistogramModel_H

#include "model.h"
#include <iostream>
class HistogramModel : public BasicModel
{
public:
    HistogramModel(){};
    HistogramModel(int childNum)
    {
        childNumber = childNum * 2; // means the max idx of table
        value = 1;
        minValue = 0;
        // for (int i = 0; i <= childNumber; i++)
        //     table.push_back(0);
    }
    void Train(const vector<pair<double, double>> &dataset, int len);
    int Predict(double key)
    {
        // return the idx in children
        int idx = float(key - minValue) / value;
        if (idx < 0)
            idx = 0;
        else if (idx >= childNumber)
            idx = childNumber - 1;

        int base;
        if ((idx / 16) % 2 == 0)
        {
            base = table1[idx / 32] >> 16;
            int j = idx % 16;
            int tmp = table0[idx / 32] >> (31 - j);
            for (; j >= 0; j--)
            {
                base += tmp & 1;
                tmp = tmp >> 1;
            }
        }
        else
        {
            base = table1[idx / 32] & 0x0000FFFF;
            int j = idx % 16;
            int tmp = table0[idx / 32] >> (15 - j);
            for (; j >= 0; j--)
            {
                base += tmp & 1;
                tmp = tmp >> 1;
            }
        }
        return base;
    }

private:
    float value; // 4B
    // vector<double> table; //2c*8 Byte
    vector<unsigned int> table0; // 2c/32*8 = c/2 Byte
    vector<unsigned int> table1; // c/2 Byte
    int childNumber;    // 4B
    double minValue;    // 8B
};

void HistogramModel::Train(const vector<pair<double, double>> &dataset, int len)
{
    if (dataset.size() == 0)
        return;
    double maxValue;
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
        {
            minValue = dataset[i].first;
            break;
        }
    }
    for (int i = dataset.size() - 1; i >= 0; i--)
    {
        if (dataset[i].first != -1)
        {
            maxValue = dataset[i].first;
            break;
        }
    }
    value = float(maxValue - minValue) / childNumber;
    vector<int> table;
    for (int i = 0; i < childNumber; i++)
        table.push_back(0);
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
        {
            int idx = float(dataset[i].first - minValue) / value;
            idx = min(idx, int(table.size()) - 1);
            table[idx]++;
        }
    }
    int cnt = 0;
    int nowSize = 0;
    int avg = dataset.size() / len;
    for (int i = 0; i < table.size(); i++)
    {
        nowSize += table[i];
        if (table[i] >= avg || nowSize >= avg)
        {
            cnt++;
            nowSize = 0;
        }
        table[i] = cnt;
    }

    int i = 0;
    for (; i < childNumber; i += 32)
    {
        //  & 0x0FFFFFFF;
        unsigned int start_idx = table[i];
        int tmp = 0;
        for (int j = i; j < i + 32; j++)
        {
            if (j - i == 16)
                start_idx = table[i + 16];
            if (j >= childNumber)
            {
                while (j < i + 32)
                {
                    tmp = tmp << 1;
                    j++;
                }
                table1.push_back(int(table[i]) << 16);
                if (i + 16 < childNumber)
                    table1[table1.size() - 1] = (int(table[i]) << 16) + int(table[i + 16]);
                table0.push_back(tmp);
                // cout << "table[i]:" << table[i] << "\ttable1[table1.size()-1]:" << table1[table1.size() - 1] << "\ttmp:" << tmp << endl;
                // cout << "TOTAL sSIZ:" << table1.size() << endl;
                return;
            }
            int diff = int(table[j]) - start_idx;
            // cout << "j:" << j << "\ttable[j]:" << table[j] << "\tstart:" << start_idx << "\tdiff:" << diff << endl;
            // if (diff >= 2)
                // cout << "wrong!\t"
                    //  << "table[j]:" << table[j] << "\tidx:" << start_idx << "\tdiff:" << diff << endl;
            tmp = (tmp << 1) + diff;
            if (diff > 0)
                start_idx += diff;
        }
        start_idx = (int(table[i]) << 16) + int(table[i + 16]);
        table1.push_back(start_idx);
        table0.push_back(tmp);
        // cout<<"i:"<<i << "\ttable[i]:" << table[i] << "\tstartidx:" << start_idx << "\ttmp:" << tmp << endl;
    }
}

#endif