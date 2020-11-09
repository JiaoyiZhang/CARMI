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
        childNumber = childNum * 2 - 1;  // means the max idx of table
        value = 1;
        minValue = 0;
        for (int i = 0; i <= childNumber; i++)
            table.push_back(0);
    }
    void Train(const vector<pair<double, double>> &dataset, int len);
    int Predict(double key)
    {   
        // return the idx in children
        int idx = float(key-minValue) / value;
        if(idx < 0)
            idx = 0;
        else if(idx > childNumber)
            idx = childNumber;
        return table[idx];
        // int base;
        // if((idx/16)%2 == 0)
        // {
        //     base = table1[idx / 32] >> 16;
        //     cout<<"table1:"<<table1[idx / 32]<<"\tbase:"<<base<<endl;
        //     int j = idx % 16;
        //     int tmp = table0[idx / 32] >> (31 - j);
        //     cout<<"table0:"<<table0[idx / 32]<<endl;
        //     cout<<"+ idx:"<<idx<<"\tbase:"<<base<<"\ttmp:"<<tmp<<endl;
        //     for(;j>=0;j--)
        //     {
        //         base += tmp & 1;
        //         tmp = tmp >> 1;
        //     }
        // }
        // else
        // {
        //     base = table1[idx / 32] & 0x0000FFFF;
        //     cout<<"table1:"<<table1[idx / 32]<<"\tbase:"<<base<<endl;
        //     int j = idx % 16;
        //     int tmp = table0[idx / 32] >> (15 - j);
        //     cout<<"table0:"<<table0[idx / 32]<<endl;
        //     cout<<"- idx:"<<idx<<"\tbase:"<<base<<"\ttmp:"<<tmp<<endl;
        //     for(;j>=0;j--)
        //     {
        //         base += tmp & 1;
        //         tmp = tmp >> 1;
        //     }
        // }
        // cout<<"base:"<<base<<"\ttable[]:"<<int(table[idx])<<endl;
        // if(base != int(table[idx]))
        //     cout<<"WRONG!"<<endl;
        // return base;
    }

    // designed for test
    float GetValue(){return value;}
    void GetTable(vector<double> &t)
    {
        for(int i=0;i<table.size();i++)
            t.push_back(table[i]);
    }

private:
    float value;  // 4B
    vector<double> table;  //2c*8 Byte
    // vector<int> table0;
    // vector<int> table1;
    int childNumber;  // 4B
    double minValue;  // 8B
    // int length;
};

void HistogramModel::Train(const vector<pair<double, double>> &dataset, int len)
{
    // length = len - 1;
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
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
        {
            int idx = float(dataset[i].first-minValue) / value;
            table[idx]++;
        }
    }
    table[0] = float(table[0]) / dataset.size();
    for (int i = 1; i <= childNumber; i++)
    {
        table[i] = table[i - 1] + float(table[i]) / dataset.size();
    }
    for(int i=0;i<=childNumber;i++)
        table[i] *= len-1;
        
    // int i=0;
    // for(;i<=childNumber;i+=32)
    // {
    //     //  & 0x0FFFFFFF;
    //     int start_idx = table[i];
    //     int tmp = 0;
    //     for(int j=i;j<i+32;j++)
    //     {
    //         if(j-i==16)
    //             start_idx = table[i+16];
    //         if(j>childNumber)
    //         {
    //             while(j<i+32)
    //             {
    //                 tmp=tmp<<1;
    //                 j++;
    //             }
    //             table1.push_back(int(table[i])<<16);
    //             if(i+16<=childNumber)
    //                 table1[table1.size()-1] = (int(table[i]) << 16) + int(table[i+16]);
    //             table0.push_back(tmp);
    //             cout<<"table[i]:"<<table[i]<<"\ttable1[table1.size()-1]:"<<table1[table1.size()-1]<<"\ttmp:"<<tmp<<endl;
    //             cout<<"TOTAL sSIZ:"<<table1.size()<<endl;
    //             return ;
    //         }
    //         int diff = int(table[j]) - start_idx;
    //         cout<<"j:"<<j<<"\ttable[j]:"<<table[j]<<"\tint(table[j]):"<<int(table[j])<<"\tstart:"<<start_idx<<"\tdiff:"<<diff<<endl;
    //         if(diff>=2)
    //             cout<<"wrong!\t"<<"table[j]:"<<table[j]<<"\tidx:"<<start_idx<<"\tdiff:"<<diff<<endl;
    //         tmp = (tmp << 1) + diff;
    //         if(diff>0)
    //             start_idx+=diff;
    //     }
    //     start_idx = (int(table[i]) << 16) + int(table[i+16]);
    //     table1.push_back(start_idx);
    //     table0.push_back(tmp);
    //     cout<<"table[i]:"<<table[i]<<"\tstartidx:"<<start_idx<<"\ttmp:"<<tmp<<endl;
    // }
}

#endif