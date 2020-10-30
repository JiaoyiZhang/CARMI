
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

extern vector<LRType> LRVector;
extern vector<NNType> NNVector;
extern vector<HisType> HisVector;
extern vector<BSType> BSVector;
extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;

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
    cout<<"btree:"<<endl;
    clock_t s,e;
    s = clock();
    for (int i = 0; i < dataset.size(); i++)
        btreemap.find(dataset[i].first);
    e = clock();
    time0 += (double)(e - s) / CLOCKS_PER_SEC;
    cout<<"Find time:"<<(double)(e - s) / CLOCKS_PER_SEC / (float)dataset.size() <<endl;
  
    s = clock();
    for (int i = 0; i < insertDataset.size(); i++)
        btreemap.insert(insertDataset[i]);  
    e = clock();
    time1 += (double)(e - s) / CLOCKS_PER_SEC;
    cout<<"Insert time:"<<(double)(e - s) / CLOCKS_PER_SEC / (float)insertDataset.size() <<endl;


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
        btree_test(btree_time0, btree_time1, btree_time2, btree_time3);

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
                cout<<"Find time:"<<time0 / (float)dataset.size() <<endl;

                s = clock();
                for (int i = 0; i < insertDataset.size(); i++)
                    Insert(insertDataset[i]);
                e = clock();
                time0 = (double)(e - s) / CLOCKS_PER_SEC;
                time[j][1] += time0;
                cout<<"Insert time:"<<time0 / (float)insertDataset.size() <<endl;

                // s = clock();
                // for (int i = 0; i < insertDataset.size(); i++)
                //     Update({insertDataset[i].first, 1.11});
                // e = clock();
                // time0 = (double)(e - s) / CLOCKS_PER_SEC;
                // time[j][2] += time0;


                // s = clock();
                // for (int i = 0; i < insertDataset.size(); i++)
                //     Delete(insertDataset[i].first);
                // e = clock();
                // time0 = (double)(e - s) / CLOCKS_PER_SEC;
                // time[j][3] += time0;
                cout << "-------------------------------" << endl;
            }
            else
            {   
                // check WRONG
                clock_t s,e;
                double time0;
                s = clock();
                for (int i = 0; i < dataset.size(); i++)
                {
                    auto res = Find(dataset[i].first);
                    if(res.second != dataset[i].first * 10)
                        cout<<"Find failed:\ti:"<<i<<"\t"<<dataset[i].first<<"\tres: "<<res.first<<"\t"<<res.second<<endl;   
                }
                e = clock();
                time0 = (double)(e - s) / CLOCKS_PER_SEC;
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
                cout << "-------------------------------" << endl;
            }
            vector<LRType>().swap(LRVector);
            vector<NNType>().swap(NNVector);
            vector<HisType>().swap(HisVector);
            vector<BSType>().swap(BSVector);
            vector<ArrayType>().swap(ArrayVector);
            vector<GappedArrayType>().swap(GAVector);
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
    LongitudesDataset longData = LongitudesDataset(initRatio);
    LonglatDataset latData = LonglatDataset(initRatio);
    LognormalDataset logData = LognormalDataset(datasetSize, initRatio);
    UniformDataset uniData = UniformDataset(datasetSize, initRatio);
    NormalDataset norData = NormalDataset(datasetSize, initRatio);
    ExponentialDataset expData = ExponentialDataset(datasetSize, initRatio);

    childNum = 3907;
    cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
    uniData.GenerateDataset(dataset, insertDataset);
    totalTest(repetitions, calculateTime);

    cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
    expData.GenerateDataset(dataset, insertDataset);
    totalTest(repetitions, calculateTime);

    cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
    norData.GenerateDataset(dataset, insertDataset);
    totalTest(repetitions, calculateTime);

    cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
    logData.GenerateDataset(dataset, insertDataset);
    totalTest(repetitions, calculateTime);
    
    childNum = 70125;
    cout << "+++++++++++ longlat dataset ++++++++++++++++++++++++++" << endl;
    latData.GenerateDataset(dataset, insertDataset);
    totalTest(repetitions, calculateTime);
    
    cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++" << endl;
    longData.GenerateDataset(dataset, insertDataset);
    totalTest(repetitions, calculateTime);
}

int main()
{
    for(int l = 0; l < 2; l++)
    {
        kLeafNodeID = l;
        cout<<"kLeafNodeID:"<<(kLeafNodeID ? "Gapped array leaf node\n" : "Array leaf node\n")<<endl;
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
        experiment(repetitions, 1, calculateTime);  // read-only
        experiment(repetitions, 0.5, calculateTime);  // balance
        experiment(repetitions, 0, calculateTime);  // partial
    }


    ofstream outFile;
    outFile.open("nn.csv", ios::out);
    outFile<<"\n";
    outFile.open("lr.csv", ios::out);
    outFile<<"\n";
    outFile.open("his.csv", ios::out);
    outFile<<"\n";
    outFile.open("bin.csv", ios::out);
    outFile<<"\n";

    return 0;
}