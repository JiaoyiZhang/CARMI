
#include "function.h"
#include "./dataset/lognormal_distribution.h"
#include "./dataset/uniform_distribution.h"
#include "./dataset/normal_distribution.h"
#include "./dataset/exponential_distribution.h"
#include "./dataset/longitudes.h"
#include "./dataset/longlat.h"
#include "reconstruction.h"
#include "../cpp-btree/btree_map.h"
#include <algorithm>
#include <random>
#include <time.h>
#include <iostream>
#include <algorithm> 
#include <sys/time.h>                
using namespace std;

int datasetSize = 1000000;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

btree::btree_map<double, double> btreemap;

extern vector<void *> INDEX;  // store the entire index

//////////////////////////////
vector<int> test_int;
vector<float> test_float;
int rep = 10000000;
int sum_int = 0;
float sum_float = 0.0;
//////////////////////////////
int kLeafNodeID = 0;
int kInnerNodeID = 0;
int kNeuronNumber = 8;

int childNum = 70125;
int kThreshold = 256;
int kMaxKeyNum = 256;
double kRate = 0.4;

void printResult(int r, double &time0, double &time1, double &time2, double &time3)
{
    cout << "Average time: " << endl;
    cout << "Find time:" << time0 / (float)dataset.size() / float(r) << endl;
    cout << "Insert time:" << time1 / (float)insertDataset.size() / float(r) << endl;
    cout << "Update time:" << time2 / (float)insertDataset.size() / float(r) << endl;
    cout << "Delete time:" << time3 / (float)insertDataset.size() / float(r) << endl;
    cout << "Total time(find and insert):" << (time0 + time1) / (float)(dataset.size() + insertDataset.size()) / float(r) << endl;
    cout << "***********************" << endl;
}

void btree_test(double &time0, double &time1, double &time2, double &time3)
{
    clock_t s,e;
    s = clock();
    for (int i = 0; i < dataset.size(); i++)
        btreemap.find(dataset[i].first);
    e = clock();
    time0 += (double)(e - s) / CLOCKS_PER_SEC;
  
    s = clock();
    for (int i = 0; i < insertDataset.size(); i++)
        btreemap.insert(insertDataset[i]);  
    e = clock();
    time1 += (double)(e - s) / CLOCKS_PER_SEC;

    // QueryPerformanceCounter(&s);
    // for (int i = 0; i < insertDataset.size(); i++)
    //     btreemap.find(insertDataset[i].first);
    // QueryPerformanceCounter(&e);
    // time2 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    // cout << "Update time:" << time2 << endl;

    // QueryPerformanceCounter(&s);
    // for (int i = 0; i < insertDataset.size(); i++)
    //     btreemap.erase(insertDataset[i].first);
    // QueryPerformanceCounter(&e);
    // time3 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    // cout << "Delete time:" << time3 << endl;
    cout << endl;
}

void totalTest(int repetitions, bool mode)
{
    double btree_time0 = 0.0, btree_time1 = 0.0, btree_time2 = 0.0, btree_time3 = 0.0;
    double time[4][4] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    for (int rep = 0; rep < repetitions; rep++)
    {
        btree::btree_map<double, double> btree;
        for (int l = 0; l < dataset.size(); l++)
            btree.insert(dataset[l]);
        btreemap = btree;
        cout << "btree:    " << rep << endl;
        btree_test(btree_time0, btree_time1, btree_time2, btree_time3);
        cout << endl;
        cout << "-------------------------------" << endl;

        for(int j=0;j<4;j++)
        {
            kInnerNodeID = j;
            cout<<"childNum is: "<<childNum<<endl;
            cout << "repetition:" << rep << "\troot type:" << kInnerNodeID << endl;
            Initialize(dataset, childNum);
            cout << "index init over!" << endl;

            if(mode)
            {
                clock_t s,e;
                s = clock();
                for (int i = 0; i < dataset.size(); i++)
                    Find(dataset[i].first);
                e = clock();
                double time0 = (double)(e - s) / CLOCKS_PER_SEC;
                time[j][0] += time0;
                cout<<"TIme0:"<<time0 / (float)dataset.size() <<endl;

                s = clock();
                for (int i = 0; i < insertDataset.size(); i++)
                    Insert(insertDataset[i]);
                e = clock();
                time0 = (double)(e - s) / CLOCKS_PER_SEC;
                time[j][1] += time0;

                s = clock();
                for (int i = 0; i < insertDataset.size(); i++)
                    Update({insertDataset[i].first, 1.11});
                e = clock();
                time0 = (double)(e - s) / CLOCKS_PER_SEC;
                time[j][2] += time0;


                s = clock();
                for (int i = 0; i < insertDataset.size(); i++)
                    Delete(insertDataset[i].first);
                e = clock();
                time0 = (double)(e - s) / CLOCKS_PER_SEC;
                time[j][3] += time0;
                INDEX.clear();
                cout << "-------------------------------" << endl;
            }
            else
            {   // check WRONG
                clock_t s,e;
                s = clock();
                for (int i = 0; i < dataset.size(); i++)
                {
                    auto res = Find(dataset[i].first);
                    if(res.second != dataset[i].first * 10)
                        cout<<"Find failed:\ti:"<<i<<"\t"<<dataset[i].first<<"\tres: "<<res.first<<"\t"<<res.second<<endl;    
                }
                e = clock();
                double time0 = (double)(e - s) / CLOCKS_PER_SEC;
                time[j][0] += time0;
                cout<<"check FIND over!"<<endl;

                s = clock();
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto r = Insert(insertDataset[i]);
                    if(!r)
                        cout<<"Insert failed:\ti:"<< i << "\t" <<insertDataset[i].first<<endl;
                    auto res = Find(insertDataset[i].first);
                    if(res.second != insertDataset[i].first * 10)
                        cout<<"Find failed:\ti:"<<i<<"\t"<<insertDataset[i].first<<"\tres: "<<res.first<<"\t"<<res.second<<endl;    
                }
                e = clock();
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto res = Find(insertDataset[i].first);
                    if(res.second != insertDataset[i].first * 10)
                        cout<<"Find Insert failed:\ti:"<<i<<"\t"<<insertDataset[i].first<<"\tres: "<<res.first<<"\t"<<res.second<<endl;    
                }
                time0 = (double)(e - s) / CLOCKS_PER_SEC;
                time[j][1] += time0;
                cout<<"check INSERT over!"<<endl;


                s = clock();
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto r = Update({insertDataset[i].first, 1.11});
                    if(!r)
                        cout<<"Update failed:\ti:"<< i << "\t" <<insertDataset[i].first<<endl;
                    auto res = Find(insertDataset[i].first);
                    if(res.second != 1.11)
                        cout<<"After Update failed:\ti:"<<i<<"\t"<<insertDataset[i].first<<"\tres: "<<res.first<<"\t"<<res.second<<endl;    
                }
                e = clock();
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto res = Find(insertDataset[i].first);
                    if(res.second != 1.11)
                        cout<<"Find Update failed:\ti:"<<i<<"\t"<<insertDataset[i].first<<"\tres: "<<res.first<<"\t"<<res.second<<endl;    
                }
                time0 = (double)(e - s) / CLOCKS_PER_SEC;
                time[j][2] += time0;
                cout<<"check UPDATE over!"<<endl;


                s = clock();
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto r = Delete(insertDataset[i].first);
                    if(!r)
                        cout<<"Delete failed:\ti:"<< i << "\t" <<insertDataset[i].first<<endl;
                    auto res = Find(insertDataset[i].first);
                    if(res.second == insertDataset[i].first * 10)
                        cout<<"After Delete failed:\ti:"<<i<<"\t"<<insertDataset[i].first<<"\tres: "<<res.first<<"\t"<<res.second<<endl;    
                }
                e = clock();
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto res = Find(insertDataset[i].first);
                    if(res.second == insertDataset[i].first * 10)
                        cout<<"Find Delete failed:\ti:"<<i<<"\t"<<insertDataset[i].first<<"\tres: "<<res.first<<"\t"<<res.second<<endl;    
                }
                time0 = (double)(e - s) / CLOCKS_PER_SEC;
                time[j][3] += time0;
                cout<<"check DELETE over!"<<endl;
                // INDEX.clear();
                cout << "-------------------------------" << endl;
            }
            cout << "start to clear all"<<endl;
            cout<<"index size is: "<<INDEX.size()<<endl;
            ClearAll(0, kInnerNodeID);
            INDEX.clear();
            vector<void *>().swap(INDEX);
        }
    }

    cout << "btreemap:" << endl;
    printResult(repetitions, btree_time0, btree_time1, btree_time2, btree_time3);

    cout << "lr:" << endl;
    printResult(repetitions, time[0][0], time[0][1], time[0][2], time[0][3]);

    cout << "nn:" << endl;
    printResult(repetitions, time[1][0], time[1][1], time[1][2], time[1][3]);

    cout << "his:" << endl;
    printResult(repetitions, time[2][0], time[2][1], time[2][2], time[2][3]);

    cout << "bin:" << endl;
    printResult(repetitions, time[3][0], time[3][1], time[3][2], time[3][3]);
}

void experiment(int repetitions, double initRatio, bool calculateTime)
{
    cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
    cout<<"initRatio is: "<<initRatio<<endl;
    // LongitudesDataset longData = LongitudesDataset(initRatio);
    // LonglatDataset latData = LonglatDataset(initRatio);
    LognormalDataset logData = LognormalDataset(datasetSize, initRatio);
    UniformDataset uniData = UniformDataset(datasetSize, initRatio);
    NormalDataset norData = NormalDataset(datasetSize, initRatio);
    ExponentialDataset expData = ExponentialDataset(datasetSize, initRatio);

    childNum = 3907;
    cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
    uniData.GenerateDataset(dataset, insertDataset);
    totalTest(repetitions, calculateTime);
    // testReconstructure(0, repetitions);

    cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
    expData.GenerateDataset(dataset, insertDataset);
    totalTest(repetitions, calculateTime);
    // totalTest(false, repetitions);

    cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
    norData.GenerateDataset(dataset, insertDataset);
    totalTest(repetitions, calculateTime);
    // testReconstructure(2, repetitions);

    cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
    logData.GenerateDataset(dataset, insertDataset);
    totalTest(repetitions, calculateTime);
    // testReconstructure(0, repetitions);
    
    // childNum = 70125;
    // cout << "+++++++++++ longlat dataset ++++++++++++++++++++++++++" << endl;
    // latData.GenerateDataset(dataset, insertDataset);
    // totalTest(repetitions, calculateTime);
    // // testReconstructure(2, repetitions);
    
    // cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++" << endl;
    // longData.GenerateDataset(dataset, insertDataset);
    // totalTest(repetitions, calculateTime);
    // testReconstructure(2, repetitions);
}

void calculateTime(int sum_int, int i)
{
    sum_int = sum_int + test_int[i];
}

int main()
{
    {
        // for(int i=0;i<rep;i++)
        // {
        //     test_int.push_back(i*i);
        //     test_float.push_back(i+3.1415926535898);
        // }
        // clock_t s, f;
        // int mid = (rep / 2) * (rep / 2);
        // sum_int = 0;
        // unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        // shuffle(test_int.begin(), test_int.end(), default_random_engine(seed));
        // s = clock();
        // for(int i=0;i<rep;i++)
        // {
        //     if(test_int[i]>mid)
        //         sum_int = sum_int + test_int[i];
        // }
        // f = clock();
        // double unsorted = (double)(f - s) / rep / CLOCKS_PER_SEC;
        // cout<<"unsorted time :"<<unsorted<<endl;

        // sum_int = 0;
        // sort(test_int.begin(), test_int.end());
        // s = clock();
        // for(int i=0;i<rep;i++)
        // {
        //     if(test_int[i]>mid)
        //         sum_int = sum_int + test_int[i];
        // }
        // f = clock();
        // double sorted = (double)(f - s) / rep / CLOCKS_PER_SEC;
        // cout<<"sorted time :"<<sorted<<endl;

        // sum_int = 0;
        // s = clock();
        // for(int i=0;i<rep;i++)
        // {
        //     sum_int = sum_int + test_int[i];
        // }
        // f = clock();
        // double integer_time = (double)(f - s) / rep / CLOCKS_PER_SEC;
        // cout<<"Integer time in Main():"<<integer_time<<endl;

        // sum_int = 0;
        // s = clock();
        // for(int i=0;i<rep;i++)
        //     calculateTime(sum_int, i);
        // f = clock();
        // double func = (double)(f - s) / rep / CLOCKS_PER_SEC;
        // cout<<"f:"<<f<<"\ns:"<<s<<"\nclock:"<<CLOCKS_PER_SEC<<endl;
        // cout<<"Integer time in function():"<<func<<endl;

        // sum_int = 0;
        // s = clock();
        // for(int i=0;i<rep;i++)
        // {
        //     sum_int = test_int[i];
        // }
        // f = clock();
        // double assign = (double)(f - s) / rep / CLOCKS_PER_SEC;
        // cout<<"assign time in Main():"<<assign<<endl;
        // cout<<"Integer time after: "<<integer_time - assign<<endl;

        // sum_float = 0.0;
        // s = clock();
        // for(int i=0;i<rep;i++)
        // {
        //     sum_float = sum_float + test_float[i];
        // }
        // f = clock();
        // double float_time = (double)(f - s) / rep / CLOCKS_PER_SEC;
        // cout<<"float time in Main():"<<float_time<<endl;

        // sum_float = 0.0;
        // s = clock();
        // for(int i=0;i<rep;i++)
        // {
        //     sum_float = test_float[i];
        // }
        // f = clock();
        // double float_assign = (double)(f - s) / rep / CLOCKS_PER_SEC;
        // cout<<"assign time in Main():"<<float_assign<<endl;
        // cout<<"float time after: "<<float_time - float_assign<<endl;

        // cout<<"\t"<<"unsorted / sorted: "<<float(unsorted) / sorted<<endl;
        // cout<<"\t"<<"function / main: "<<float(func) / integer_time<<endl;
        // cout<<"\t"<<"float / int (with assign): "<<float(float_time ) / float(integer_time)<<endl;
        // cout<<"\t"<<"float / int: "<<float(float_time - float_assign) / float(integer_time - assign)<<endl;

    }

    cout<<"kLeafNodeID:"<<kLeafNodeID<<endl;
    if(kLeafNodeID == 1)
    {
        kThreshold = 256;  // ga
        // kMaxKeyNum = 256;
    }
    else
    {
        kThreshold = 50000;  // array
        // kMaxKeyNum = 6000;
    }
    cout<<"kThreshold is: "<<kThreshold<<endl;
    int repetitions = 1;
    bool calculateTime = true;
    cout << "MODE: " << (calculateTime ? "CALCULATE TIME\n" : "CHECK CORRECTNESS\n");
    experiment(repetitions, 0.9, calculateTime);
    // experiment(repetitions, 1, isStatic);  // read-only
    // experiment(repetitions, 0.5, isStatic);  // balance
    // experiment(repetitions, 0, isStatic);  // partial

    return 0;
}