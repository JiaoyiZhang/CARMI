
#include "function.h"
#include "./dataset/lognormal_distribution.h"
#include "./dataset/uniform_distribution.h"
#include "./dataset/normal_distribution.h"
#include "./dataset/exponential_distribution.h"
#include "./dataset/longitudes.h"
#include "./dataset/longlat.h"
#include "reconstruction.h"
// #include "../cpp-btree/btree_map.h"
#include "../stx_btree/btree_map.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <algorithm> 
#include <chrono>         
#include "../art_tree/art.h"
#include "../art_tree/art.cpp"    
using namespace std;

int datasetSize = 1000000;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

stx::btree_map<double, double> btreemap;

extern vector<LRType> LRVector;
extern vector<NNType> NNVector;
extern vector<HisType> HisVector;
extern vector<BSType> BSVector;
extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;


int kMaxSpace = 1024 * 1024;  // Byte
int kLeafNodeID = 0;
int kInnerNodeID = 0;
int kNeuronNumber = 8;

int childNum = 70125;
int kThreshold = 256;
int kMaxKeyNum = 256;
double kRate = 0.4;

ofstream outRes;

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

void artTree_test()
{
    art_tree t;
    art_tree_init(&t);
    vector<char *> artKey, artValue;
    for(int i=0;i<dataset.size();i++)
    {
        auto key = (const unsigned char *)to_string(dataset[i].first).data();
        auto value = (const unsigned char *)to_string(dataset[i].second).data();
        artKey.push_back((char *)key);
        artValue.push_back((char *)value);
        art_insert(&t, key, strlen((const char*)key), (uint64_t)dataset[i].second);
    }
    vector<uint64_t> rets;
    chrono::_V2::system_clock::time_point s, e;
    double tmp, tmp1;
    s = chrono::system_clock::now();
    for (int i = 0; i < artKey.size(); i++)
    {
        auto key = (const unsigned char *)to_string(dataset[i].first).data();
    }
    e = chrono::system_clock::now();
    tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den;

    s = chrono::system_clock::now();
    for (int i = 0; i < artKey.size(); i++)
    {
        auto key = (const unsigned char *)to_string(dataset[i].first).data();
        art_search(&t, key, strlen((const char*)key), rets);
    }
    e = chrono::system_clock::now();
    tmp1 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den;
    cout<<"art time: "<<tmp1 / (float)dataset.size()<<endl;
    cout<<"other time:"<<tmp / (float)dataset.size()<<endl;
    cout<<"final time:"<<(tmp1-tmp) / (float)dataset.size()<<endl;
}

void btree_test(double &time0, double &time1, double &time2, double &time3)
{
    cout<<"btree:"<<endl;
    chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = chrono::system_clock::now();
    for (int i = 0; i < dataset.size(); i++)
        btreemap.find(dataset[i].first);
    e = chrono::system_clock::now();
    tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den;
    time0 += tmp;
    cout<<"Find time:"<< tmp / (float)dataset.size() <<endl;
    outRes << "btree," << tmp / (float)dataset.size() <<",";
  
    s = chrono::system_clock::now();
    for (int i = 0; i < insertDataset.size(); i++)
        btreemap.insert(insertDataset[i]);  
    e = chrono::system_clock::now();
    tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den;
    time1 += tmp;
    cout<<"Insert time:"<< tmp / (float)insertDataset.size() <<endl;
    outRes << tmp / (float)insertDataset.size() << ",";

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
        // artTree_test();
        // stx::btree_map<double, double> btree;
        // for (int l = 0; l < dataset.size(); l++)
        //     btree.insert(dataset[l]);
        // btreemap = btree;
        // outRes << "childNum is: "<<childNum<<endl;
        // btree_test(btree_time0, btree_time1, btree_time2, btree_time3);
        // cout << "-------------------------------" << endl;

        for(int j=0;j<4;j++)
        {
            kInnerNodeID = j;
            cout<<"childNum is: "<<childNum<<endl;
            cout << "repetition:" << rep << "\troot type:" << kInnerNodeID << endl;
            Initialize(dataset, childNum);
            cout << "index init over!" << endl;
            switch (kInnerNodeID)
            {
                case 0:
                outRes<<"lr,";
                break;
                case 1:
                outRes<<"nn,";
                break;
                case 2:
                outRes<<"his,";
                break;
                case 3:
                outRes<<"bin,";
                break;
            }

            if(mode)
            {
                chrono::_V2::system_clock::time_point s, e;
                double tmp;
                s = chrono::system_clock::now();
                for (int i = 0; i < dataset.size(); i++)
                    Find(dataset[i].first);
                e = chrono::system_clock::now();
                tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den;
                time[j][0] += tmp;
                cout<<"Find time:"<<tmp / (float)dataset.size() <<endl;
                outRes << tmp / (float)dataset.size() << ",";
                
                auto entropy = GetEntropy(dataset.size());
                cout<<"Entropy:"<< entropy <<endl;
                outRes << "ENTROPY," << entropy << ",";
                cout<<"time / entropy: "<<tmp / (float)dataset.size() / entropy<<endl;
                outRes<<"ratio,"<<tmp / (float)dataset.size() / entropy<<",";


                // s = chrono::system_clock::now();
                // for (int i = 0; i < insertDataset.size(); i++)
                //     Insert(insertDataset[i]);
                // e = chrono::system_clock::now();
                // tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den;
                // time[j][1] += tmp;
                // cout<<"Insert time:"<<tmp / (float)insertDataset.size() <<endl;
                // outRes << tmp / (float)insertDataset.size() << ",";

                // s = chrono::system_clock::now();
                // for (int i = 0; i < insertDataset.size(); i++)
                //     Update({insertDataset[i].first, 1.11});
                // e = chrono::system_clock::now();
                // tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den;
                // time[j][2] += tmp;


                // s = chrono::system_clock::now();
                // for (int i = 0; i < insertDataset.size(); i++)
                //     Delete(insertDataset[i].first);
                // e = chrono::system_clock::now();
                // tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den;
                // time[j][3] += tmp;
                cout << "-------------------------------" << endl;
            }
            else
            {   
                // check WRONG
                chrono::_V2::system_clock::time_point s, e;
                double tmp;
                s = chrono::system_clock::now();
                for (int i = 0; i < dataset.size(); i++)
                {
                    auto res = Find(dataset[i].first);
                    if(res.second != dataset[i].second)
                        cout<<"Find failed:\ti:"<<i<<"\tdata:"<<dataset[i].first<<"\t"<<dataset[i].second<<"\tres: "<<res.first<<"\t"<<res.second<<endl;   
                }
                e = chrono::system_clock::now();
                tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den;
                time[j][0] += tmp;
                cout<<"check FIND over!"<<endl;

                s = chrono::system_clock::now();
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto r = Insert(insertDataset[i]);
                    if(!r)
                        cout<<"Insert failed:\ti:"<< i << "\t" <<insertDataset[i].first<<endl;
                    auto res = Find(insertDataset[i].first);
                    if(res.second != insertDataset[i].second)
                        cout<<"Find failed:\ti:"<<i<<"\t"<<insertDataset[i].first<<"\tres: "<<res.first<<"\t"<<res.second<<endl;   
                }
                e = chrono::system_clock::now();
                tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den;
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto res = Find(insertDataset[i].first);
                    if(res.second != insertDataset[i].second)
                        cout<<"Find Insert failed:\ti:"<<i<<"\t"<<insertDataset[i].first<<"\tres: "<<res.first<<"\t"<<res.second<<endl;    
                }
                time[j][1] += tmp;
                cout<<"check INSERT over!"<<endl;


                s = chrono::system_clock::now();
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto r = Update({insertDataset[i].first, 1.11});
                    if(!r)
                        cout<<"Update failed:\ti:"<< i << "\t" <<insertDataset[i].first<<endl;
                    auto res = Find(insertDataset[i].first);
                    if(res.second != 1.11)
                        cout<<"After Update failed:\ti:"<<i<<"\t"<<insertDataset[i].first<<"\tres: "<<res.first<<"\t"<<res.second<<endl;    
                }
                e = chrono::system_clock::now();
                tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den;
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto res = Find(insertDataset[i].first);
                    if(res.second != 1.11)
                        cout<<"Find Update failed:\ti:"<<i<<"\t"<<insertDataset[i].first<<"\tres: "<<res.first<<"\t"<<res.second<<endl;    
                }
                time[j][2] += tmp;
                cout<<"check UPDATE over!"<<endl;


                s = chrono::system_clock::now();
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto r = Delete(insertDataset[i].first);
                    if(!r)
                        cout<<"Delete failed:\ti:"<< i << "\t" <<insertDataset[i].first<<endl;
                    auto res = Find(insertDataset[i].first);
                    if(res.second == insertDataset[i].second || res.second == 1.11)
                        cout<<"After Delete failed:\ti:"<<i<<"\t"<<insertDataset[i].first<<"\tres: "<<res.first<<"\t"<<res.second<<endl;    
                }
                e = chrono::system_clock::now();
                tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den;
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto res = Find(insertDataset[i].first);
                    if(res.second == insertDataset[i].second || res.second == 1.11)
                        cout<<"Find Delete failed:\ti:"<<i<<"\t"<<insertDataset[i].first<<"\tres: "<<res.first<<"\t"<<res.second<<endl;    
                }
                tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den;
                time[j][3] += tmp;
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
        outRes << endl << endl; 
    }

    // cout << "btreemap:" << endl;
    // printResult(repetitions, btree_time0, btree_time1, btree_time2, btree_time3);

    // cout << "lr:" << endl;
    // printResult(repetitions, time[0][0], time[0][1], time[0][2], time[0][3]);

    // cout << "nn:" << endl;
    // printResult(repetitions, time[1][0], time[1][1], time[1][2], time[1][3]);

    // cout << "his:" << endl;
    // printResult(repetitions, time[2][0], time[2][1], time[2][2], time[2][3]);

    // cout << "bin:" << endl;
    // printResult(repetitions, time[3][0], time[3][1], time[3][2], time[3][3]);
}

void experiment(int repetitions, double initRatio, bool calculateTime)
{
    cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
    cout<<"initRatio is: "<<initRatio<<endl;
    outRes<<"initRatio is: "<<initRatio<<endl;
    LongitudesDataset longData = LongitudesDataset(initRatio);
    LonglatDataset latData = LonglatDataset(initRatio);
    LognormalDataset logData = LognormalDataset(datasetSize, initRatio);
    UniformDataset uniData = UniformDataset(datasetSize, initRatio);
    NormalDataset norData = NormalDataset(datasetSize, initRatio);
    ExponentialDataset expData = ExponentialDataset(datasetSize, initRatio);

    vector<int> childNum_synthetic = {2000, 3000, 3907, 5000, 7500, 10000, 12500, 15000, 17500, 20000, 50000, 100000};
    vector<int> childNum_map = {25000, 50000, 60000, 70125, 80000, 90000, 100000, 250000, 350000, 500000, 750000, 1000000};
    for(int i=0;i<childNum_synthetic.size();i++)
    // for(int i=2;i<3;i++)
    {
        childNum = childNum_synthetic[i];
        cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        uniData.GenerateDataset(dataset, insertDataset);
        outRes << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
        totalTest(repetitions, calculateTime);

        cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        expData.GenerateDataset(dataset, insertDataset);
        outRes << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
        totalTest(repetitions, calculateTime);

        cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        norData.GenerateDataset(dataset, insertDataset);
        outRes << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
        totalTest(repetitions, calculateTime);

        cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        logData.GenerateDataset(dataset, insertDataset);
        outRes << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
        totalTest(repetitions, calculateTime);
        
        childNum = childNum_map[i];
        cout << "+++++++++++ longlat dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        latData.GenerateDataset(dataset, insertDataset);
        outRes << "+++++++++++ longlat dataset ++++++++++++++++++++++++++" << endl;
        totalTest(repetitions, calculateTime);
        
        cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        longData.GenerateDataset(dataset, insertDataset);
        outRes << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++" << endl;
        totalTest(repetitions, calculateTime);
    }
}

int main()
{
    ofstream outFile;
    outFile.open("nn.csv", ios::out);
    outFile<<"\n";
    outFile.open("lr.csv", ios::out);
    outFile<<"\n";
    outFile.open("his.csv", ios::out);
    outFile<<"\n";
    outFile.open("bin.csv", ios::out);
    outFile<<"\n";

    outRes.open("res_1109_totalRes.csv", ios::app);
    outRes<<"Test time: "<<__TIMESTAMP__<<endl;
    for(int l = 0; l < 2; l++)
    {

        kLeafNodeID = l;
        cout<<"kLeafNodeID:"<<(kLeafNodeID ? "Gapped array leaf node" : "Array leaf node")<<endl;
        outRes<<"kLeafNodeID:"<<(kLeafNodeID ? "Gapped array leaf node" : "Array leaf node")<<endl;
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
        outRes<<"kThreshold is: "<<kThreshold<<endl;
        int repetitions = 1;
        bool calculateTime = true;
        cout << "MODE: " << (calculateTime ? "CALCULATE TIME\n" : "CHECK CORRECTNESS\n");
        outRes << "MODE: " << (calculateTime ? "CALCULATE TIME\n" : "CHECK CORRECTNESS\n");
        // experiment(repetitions, 0.9, calculateTime);
        experiment(repetitions, 1, calculateTime);  // read-only
        // experiment(repetitions, 0.5, calculateTime);  // balance
        // experiment(repetitions, 0, calculateTime);  // partial
    }
    outRes << "----------------------------------------------" << endl;


    return 0;
}