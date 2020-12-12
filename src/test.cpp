
#include "function.h"
#include "./dataset/lognormal_distribution.h"
#include "./dataset/uniform_distribution.h"
#include "./dataset/normal_distribution.h"
#include "./dataset/exponential_distribution.h"
#include "./dataset/longitudes.h"
#include "./dataset/longlat.h"
#include "./construct/construction.h"
#include "../stx_btree/btree_map.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <assert.h>
#include "../art_tree/art.h"
#include "../art_tree/art.cpp"
using namespace std;

int datasetSize = 1000000;
int initDatasetSize;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;
stx::btree_map<double, double> btreemap;
pair<double, double> *entireData; // global array, store all leaf nodes(0:unused, 1:used)
unsigned int entireDataSize;      // the size of entireData
vector<EmptyBlock> emptyBlocks;

vector<pair<double, double>> testDataset;

int *entireChild;
unsigned int entireChildNumber;
unsigned int nowChildNumber;

extern vector<LRType> LRVector;
extern vector<NNType> NNVector;
extern vector<HisType> HisVector;
extern vector<BSType> BSVector;
extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;

vector<pair<double, double>> findActualDataset;

int kLeafNodeID = 0;
int kInnerNodeID = 0;
int kNeuronNumber = 8;

int childNum = 70125;
int kThreshold = 256;
int kMaxKeyNum = 1024;
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

void printStructure(int level, int type, int idx)
{
    switch (type)
    {
    case 0:
    {
        cout << "level " << level << ": now root is lr, idx:" << idx << ", childNumber:" << LRVector[idx].childNumber;
        vector<int> tree = {0, 0, 0, 0, 0, 0};
        for (int i = 0; i < LRVector[idx].childNumber; i++)
        {
            auto content = entireChild[LRVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[0])
            cout << "\tlr:" << tree[0];
        if (tree[1])
            cout << "\tnn:" << tree[1];
        if (tree[2])
            cout << "\this:" << tree[2];
        if (tree[3])
            cout << "\tbin:" << tree[3];
        if (tree[4])
            cout << "\tarray:" << tree[4];
        if (tree[5])
            cout << "\tga:" << tree[5];
        cout << endl;
        for (int i = 0; i < LRVector[idx].childNumber; i++)
        {
            auto content = entireChild[LRVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 1:
    {
        cout << "level " << level << ": now root is nn, idx:" << idx << ", childNumber:" << NNVector[idx].childNumber;
        vector<int> tree = {0, 0, 0, 0, 0, 0};
        for (int i = 0; i < NNVector[idx].childNumber; i++)
        {
            auto content = entireChild[NNVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[0])
            cout << "\tlr:" << tree[0];
        if (tree[1])
            cout << "\tnn:" << tree[1];
        if (tree[2])
            cout << "\this:" << tree[2];
        if (tree[3])
            cout << "\tbin:" << tree[3];
        if (tree[4])
            cout << "\tarray:" << tree[4];
        if (tree[5])
            cout << "\tga:" << tree[5];
        cout << endl;
        for (int i = 0; i < NNVector[idx].childNumber; i++)
        {
            auto content = entireChild[NNVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 2:
    {
        cout << "level " << level << ": now root is his, idx:" << idx << ", childNumber:" << HisVector[idx].childNumber;
        vector<int> tree = {0, 0, 0, 0, 0, 0};
        for (int i = 0; i < HisVector[idx].childNumber; i++)
        {
            auto content = entireChild[HisVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[0])
            cout << "\tlr:" << tree[0];
        if (tree[1])
            cout << "\tnn:" << tree[1];
        if (tree[2])
            cout << "\this:" << tree[2];
        if (tree[3])
            cout << "\tbin:" << tree[3];
        if (tree[4])
            cout << "\tarray:" << tree[4];
        if (tree[5])
            cout << "\tga:" << tree[5];
        cout << endl;
        for (int i = 0; i < HisVector[idx].childNumber; i++)
        {
            auto content = entireChild[HisVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 3:
    {
        cout << "level " << level << ": now root is bin, idx:" << idx << ", childNumber:" << BSVector[idx].childNumber;
        vector<int> tree = {0, 0, 0, 0, 0, 0};
        for (int i = 0; i < BSVector[idx].childNumber; i++)
        {
            auto content = entireChild[BSVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[0])
            cout << "\tlr:" << tree[0];
        if (tree[1])
            cout << "\tnn:" << tree[1];
        if (tree[2])
            cout << "\this:" << tree[2];
        if (tree[3])
            cout << "\tbin:" << tree[3];
        if (tree[4])
            cout << "\tarray:" << tree[4];
        if (tree[5])
            cout << "\tga:" << tree[5];
        cout << endl;
        for (int i = 0; i < BSVector[idx].childNumber; i++)
        {
            auto content = entireChild[BSVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    }
}

void artTree_test()
{
    art_tree t;
    art_tree_init(&t);
    vector<char *> artKey, artValue;
    for (int i = 0; i < dataset.size(); i++)
    {
        auto key = (const unsigned char *)to_string(dataset[i].first).data();
        auto value = (const unsigned char *)to_string(dataset[i].second).data();
        artKey.push_back((char *)key);
        artValue.push_back((char *)value);
        art_insert(&t, key, strlen((const char *)key), (uint64_t)dataset[i].second);
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
    tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

    s = chrono::system_clock::now();
    for (int i = 0; i < artKey.size(); i++)
    {
        auto key = (const unsigned char *)to_string(dataset[i].first).data();
        art_search(&t, key, strlen((const char *)key), rets);
    }
    e = chrono::system_clock::now();
    // tmp1 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    cout << "art time: " << tmp1 / (float)dataset.size() * 1000000000 << endl;
    cout << "other time:" << tmp / (float)dataset.size() * 1000000000 << endl;
    cout << "final time:" << (tmp1 - tmp) / (float)dataset.size() * 1000000000 << endl;
}

void btree_test()
{
    cout << "btree:" << endl;
    chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = chrono::system_clock::now();
    for (int i = 0; i < dataset.size(); i++)
        btreemap.find(dataset[i].first);
    e = chrono::system_clock::now();
    tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    cout << "Find time:" << tmp / (float)dataset.size() * 1000000000 << "ns" << endl;
    outRes << "btree," << tmp / (float)dataset.size() * 1000000000 << ",";

    s = chrono::system_clock::now();
    for (int i = 0; i < insertDataset.size(); i++)
        btreemap.insert(insertDataset[i]);
    e = chrono::system_clock::now();
    tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    cout << "Insert time:" << tmp / (float)insertDataset.size() * 1000000000 << endl;
    outRes << tmp / (float)insertDataset.size() * 1000000000 << "\n";

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

        for (int j = 0; j < 4; j++)
        {
            kInnerNodeID = j;
            cout << "childNum is: " << childNum << endl;
            cout << "repetition:" << rep << "\troot type:" << kInnerNodeID << endl;
            initEntireData(0, dataset.size() + insertDataset.size(), false);
            initEntireChild(dataset.size() + insertDataset.size());
            Initialize(dataset, childNum);
            cout << "index init over!" << endl;
            switch (kInnerNodeID)
            {
            case 0:
                outRes << "lr,";
                break;
            case 1:
                outRes << "nn,";
                break;
            case 2:
                outRes << "his,";
                break;
            case 3:
                outRes << "bin,";
                break;
            }

            if (mode)
            {
                chrono::_V2::system_clock::time_point s, e;
                double tmp;
                s = chrono::system_clock::now();
                for (int i = 0; i < dataset.size(); i++)
                    Find(kInnerNodeID, dataset[i].first);
                e = chrono::system_clock::now();
                tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den * 1000000000;
                time[j][0] += tmp;
                cout << "Find time:" << tmp / (float)dataset.size() << endl;
                outRes << tmp / (float)dataset.size() << ",";

                auto entropy = GetEntropy(dataset.size());
                cout << "Entropy:" << entropy << endl;
                outRes << "ENTROPY," << entropy << ",";
                cout << "time / entropy: " << tmp / (float)dataset.size() / entropy << endl;
                outRes << "ratio," << tmp / (float)dataset.size() / entropy << ",";

                // s = chrono::system_clock::now();
                // for (int i = 0; i < insertDataset.size(); i++)
                //     Insert(kInnerNodeID, insertDataset[i]);
                // e = chrono::system_clock::now();
                // tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den* 1000000000 ;
                // time[j][1] += tmp;
                // cout<<"Insert time:"<<tmp / (float)insertDataset.size() <<endl;
                // outRes << tmp / (float)insertDataset.size() << ",";

                // s = chrono::system_clock::now();
                // for (int i = 0; i < insertDataset.size(); i++)
                //     Update(kInnerNodeID, {insertDataset[i].first, 1.11});
                // e = chrono::system_clock::now();
                // tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den* 1000000000 ;
                // time[j][2] += tmp;

                // s = chrono::system_clock::now();
                // for (int i = 0; i < insertDataset.size(); i++)
                //     Delete(kInnerNodeID, insertDataset[i].first);
                // e = chrono::system_clock::now();
                // tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count())/chrono::nanoseconds::period::den* 1000000000 ;
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
                    auto res = Find(kInnerNodeID, dataset[i].first);
                    if (res.second != dataset[i].second)
                        cout << "Find failed:\ti:" << i << "\tdata:" << dataset[i].first << "\t" << dataset[i].second << "\tres: " << res.first << "\t" << res.second << endl;
                }
                e = chrono::system_clock::now();
                tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
                time[j][0] += tmp;
                cout << "check FIND over!" << endl;

                s = chrono::system_clock::now();
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto r = Insert(kInnerNodeID, insertDataset[i]);
                    if (!r)
                        cout << "Insert failed:\ti:" << i << "\t" << insertDataset[i].first << endl;
                    auto res = Find(kInnerNodeID, insertDataset[i].first);
                    if (res.second != insertDataset[i].second)
                        cout << "after insert, Find failed:\ti:" << i << "\t" << insertDataset[i].first << "\tres: " << res.first << "\t" << res.second << endl;
                }
                e = chrono::system_clock::now();
                tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto res = Find(kInnerNodeID, insertDataset[i].first);
                    if (res.second != insertDataset[i].second)
                        cout << "Find Insert failed:\ti:" << i << "\t" << insertDataset[i].first << "\tres: " << res.first << "\t" << res.second << endl;
                }
                time[j][1] += tmp;
                cout << "check INSERT over!" << endl;

                s = chrono::system_clock::now();
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto r = Update(kInnerNodeID, {insertDataset[i].first, 1.11});
                    if (!r)
                        cout << "Update failed:\ti:" << i << "\t" << insertDataset[i].first << endl;
                    auto res = Find(kInnerNodeID, insertDataset[i].first);
                    if (res.second != 1.11)
                        cout << "After Update failed:\ti:" << i << "\t" << insertDataset[i].first << "\tres: " << res.first << "\t" << res.second << endl;
                }
                e = chrono::system_clock::now();
                tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto res = Find(kInnerNodeID, insertDataset[i].first);
                    if (res.second != 1.11)
                        cout << "Find Update failed:\ti:" << i << "\t" << insertDataset[i].first << "\tres: " << res.first << "\t" << res.second << endl;
                }
                time[j][2] += tmp;
                cout << "check UPDATE over!" << endl;

                s = chrono::system_clock::now();
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto r = Delete(kInnerNodeID, insertDataset[i].first);
                    if (!r)
                        cout << "Delete failed:\ti:" << i << "\t" << insertDataset[i].first << endl;
                    auto res = Find(kInnerNodeID, insertDataset[i].first);
                    if ((res.second == insertDataset[i].second) || (res.second == 1.11))
                        cout << "After Delete failed:\ti:" << i << "\t" << insertDataset[i].first << "\tres: " << res.first << "\t" << res.second << endl;
                }
                e = chrono::system_clock::now();
                tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
                for (int i = 0; i < insertDataset.size(); i++)
                {
                    auto res = Find(kInnerNodeID, insertDataset[i].first);
                    if ((res.second == insertDataset[i].second) || (res.second == 1.11))
                        cout << "Find Delete failed:\ti:" << i << "\t" << insertDataset[i].first << "\tres: " << res.first << "\t" << res.second << endl;
                }
                tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
                time[j][3] += tmp;
                cout << "check DELETE over!" << endl;
                cout << "-------------------------------" << endl;
            }
            vector<LRType>().swap(LRVector);
            vector<NNType>().swap(NNVector);
            vector<HisType>().swap(HisVector);
            vector<BSType>().swap(BSVector);
            vector<ArrayType>().swap(ArrayVector);
            vector<GappedArrayType>().swap(GAVector);
        }
        outRes << endl
               << endl;
    }
}

long double calculateSpace()
{
    long double space = 0;

    for (int i = 0; i < LRVector.size(); i++)
        space += sizeof(LRType) + LRVector[i].childNumber * 4;
    for (int i = 0; i < NNVector.size(); i++)
        space += sizeof(NNType) + NNVector[i].childNumber * 4 + 192;
    for (int i = 0; i < HisVector.size(); i++)
        space += sizeof(HisType) + HisVector[i].childNumber * 5;
    for (int i = 0; i < BSVector.size(); i++)
        space += sizeof(BSType) + BSVector[i].childNumber * 12;
    space += sizeof(ArrayType) * ArrayVector.size();
    space += sizeof(GappedArrayType) * GAVector.size();
    space = space / 1024 / 1024;
    cout << "\nTOTAL SPACE: " << space << "MB" << endl;
    space += float(dataset.size()) * 16 / 1024 / 1024;
    cout << "\nTOTAL SPACE (include data): " << space << "MB" << endl;
    return space;
}

void constructionTest()
{
    cout << "Space of different classes (sizeof):" << endl;
    cout << "LRType:" << sizeof(LRType) << "\tlr model:" << sizeof(LinearRegression) << endl;
    cout << "NNType:" << sizeof(NNType) << "\tnn model:" << sizeof(PiecewiseLR) << endl;
    cout << "HisType:" << sizeof(HisType) << "\this model:" << sizeof(HistogramModel) << endl;
    cout << "BSType:" << sizeof(BSType) << "\tbs model:" << sizeof(BinarySearchModel) << endl;
    cout << "ArrayType:" << sizeof(ArrayType) << endl;
    cout << "GappedArrayType:" << sizeof(GappedArrayType) << endl;

    cout << "kMaxKeyNum:" << kMaxKeyNum << "\tkRate:" << kRate << endl;
    findActualDataset = dataset;
    auto data1 = dataset;
    auto data2 = insertDataset;

    for (int i = 0; i < dataset.size(); i++)
        data1[i].second = 2;
    for (int i = 0; i < insertDataset.size(); i++)
        data2[i].second = 10;

    initEntireData(0, dataset.size() + insertDataset.size(), false);
    initEntireChild(dataset.size() + insertDataset.size());
    auto rootType = Construction(data1, data2);
    cout << "Construction over!" << endl;
    cout << endl;

    int size = 0;
    int cnt = 0;
    cout << "The index is:" << endl;
    cout << "LRVector: size: " << LRVector.size() << endl;
    cout << "NNVector: size: " << NNVector.size() << endl;
    cout << "HisVector: size: " << HisVector.size() << endl;
    cout << "BSVector: size: " << BSVector.size() << endl;
    cout << "ArrayVector size:" << ArrayVector.size() << endl;
    cout << "GAVector size:" << GAVector.size() << endl;
    cout << "the number of nodes: " << size << "\tcnt:" << cnt << endl;
    cout << "\nprint the space:" << endl;
    auto space = calculateSpace();
    outRes << space << ",";

    cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
    cout << "print structure:" << endl;
    printStructure(1, rootType, 0);
    cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;

    if (kRate == 1)
    {
        for (int i = 0; i < dataset.size(); i++)
        {
            auto res = Find(rootType, dataset[i].first);
            // if ((res.second != dataset[i].second) || (res.first != dataset[i].first))
            if (res.first != dataset[i].first)
                cout << "Find failed:\ti:" << i << "\tdata:" << dataset[i].first << "\t" << dataset[i].second << "\tres: " << res.first << "\t" << res.second << endl;
        }
        cout << "check FIND over!" << endl;
    }

    chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = chrono::system_clock::now();
    for (int i = 0; i < dataset.size(); i++)
        Find(rootType, dataset[i].first);
    e = chrono::system_clock::now();
    tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    cout << "Find time:" << tmp / (float)dataset.size() * 1000000000 - 5 << endl;
    outRes << tmp / (float)dataset.size() * 1000000000 - 5 << ",";

    s = chrono::system_clock::now();
    for (int i = 0; i < insertDataset.size(); i++)
        Insert(rootType, insertDataset[i]);
    e = chrono::system_clock::now();
    tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    cout << "Insert time:" << tmp / (float)insertDataset.size() - 5 << endl;
    outRes << tmp / (float)insertDataset.size() - 5 << "ns,";
}

void experiment(double isConstruction, int repetitions, double initRatio, bool calculateTime)
{
    cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
    cout << "initRatio is: " << initRatio << endl;
    outRes << "initRatio," << initRatio << endl;
    LongitudesDataset longData = LongitudesDataset(initRatio);
    LonglatDataset latData = LonglatDataset(initRatio);
    LognormalDataset logData = LognormalDataset(datasetSize, initRatio);
    UniformDataset uniData = UniformDataset(datasetSize, initRatio);
    NormalDataset norData = NormalDataset(datasetSize, initRatio);
    ExponentialDataset expData = ExponentialDataset(datasetSize, initRatio);
    if (isConstruction)
    {
        vector<double> rate = {1, 0.6, 0.1, 0.01, 0.001, 10, 50, 100, 1000};
        for (int r = 0; r < rate.size(); r++)
        {
            kRate = rate[r];
            kMaxKeyNum = 2;
            outRes << "kRate:" << kRate << endl;
            cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
            uniData.GenerateDataset(dataset, insertDataset);
            initDatasetSize = dataset.size();
            constructionTest();

            cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
            expData.GenerateDataset(dataset, insertDataset);
            initDatasetSize = dataset.size();
            constructionTest();

            cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
            norData.GenerateDataset(dataset, insertDataset);
            initDatasetSize = dataset.size();
            constructionTest();

            cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
            logData.GenerateDataset(dataset, insertDataset);
            initDatasetSize = dataset.size();
            constructionTest();

            // kMaxKeyNum = 16;
            // cout << "+++++++++++ longlat dataset ++++++++++++++++++++++++++" << endl;
            // latData.GenerateDataset(dataset, insertDataset);
            // constructionTest();

            // cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++" << endl;
            // longData.GenerateDataset(dataset, insertDataset);
            // constructionTest();

            outRes << endl;
        }
    }
    else
    {
        vector<int> childNum_synthetic = {2000, 3000, 3907, 5000, 7500, 10000, 12500, 15000, 17500, 20000, 50000, 100000};
        vector<int> childNum_map = {25000, 50000, 60000, 70125, 80000, 90000, 100000, 250000, 350000, 500000, 750000, 1000000};
        // for (int i = 0; i < childNum_synthetic.size(); i++)
        for (int i = 2; i < 3; i++)
        {
            childNum = childNum_synthetic[i];
            kMaxKeyNum = 256;
            cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
            outRes << "+++++++++++ childNum: " << childNum << endl;
            uniData.GenerateDataset(dataset, insertDataset);
            initDatasetSize = dataset.size();
            outRes << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
            totalTest(repetitions, calculateTime);

            cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
            outRes << "+++++++++++ childNum: " << childNum << endl;
            expData.GenerateDataset(dataset, insertDataset);
            initDatasetSize = dataset.size();
            outRes << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
            totalTest(repetitions, calculateTime);

            cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
            outRes << "+++++++++++ childNum: " << childNum << endl;
            norData.GenerateDataset(dataset, insertDataset);
            initDatasetSize = dataset.size();
            outRes << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
            totalTest(repetitions, calculateTime);

            cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
            outRes << "+++++++++++ childNum: " << childNum << endl;
            logData.GenerateDataset(dataset, insertDataset);
            initDatasetSize = dataset.size();
            outRes << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
            totalTest(repetitions, calculateTime);

            if (calculateTime)
            {
                kMaxKeyNum = 512;
                childNum = childNum_map[i];
                cout << "+++++++++++ longlat dataset ++++++++++++++++++++++++++" << endl;
                outRes << "+++++++++++ childNum: " << childNum << endl;
                latData.GenerateDataset(dataset, insertDataset);
                initDatasetSize = dataset.size();
                outRes << "+++++++++++ longlat dataset ++++++++++++++++++++++++++" << endl;
                totalTest(repetitions, calculateTime);

                cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++" << endl;
                outRes << "+++++++++++ childNum: " << childNum << endl;
                longData.GenerateDataset(dataset, insertDataset);
                initDatasetSize = dataset.size();
                outRes << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++" << endl;
                totalTest(repetitions, calculateTime);
            }
        }
    }
}

int main()
{
    outRes.open("res_1212.csv", ios::app);
    outRes << "\nTest time: " << __TIMESTAMP__ << endl;
    for (int l = 0; l < 1; l++)
    {

        kLeafNodeID = l;
        cout << "kLeafNodeID:" << (kLeafNodeID ? "Gapped array leaf node" : "Array leaf node") << endl;
        outRes << "kLeafNodeID:" << (kLeafNodeID ? "Gapped array leaf node" : "Array leaf node") << endl;
        kThreshold = 256;
        cout << "kThreshold is: " << kThreshold << endl;
        outRes << "kThreshold is: " << kThreshold << endl;
        int repetitions = 1;
        bool calculateTime = false;
        bool isConstruction = true;
        cout << "MODE: " << (calculateTime ? "CALCULATE TIME\n" : "CHECK CORRECTNESS\n");
        outRes << "MODE," << (calculateTime ? "CALCULATE TIME\n" : "CHECK CORRECTNESS\n");
        experiment(isConstruction, repetitions, 1, calculateTime);   // read-only
        experiment(isConstruction, repetitions, 0.5, calculateTime); // balance
        experiment(isConstruction, repetitions, 0.9, calculateTime);
        experiment(isConstruction, repetitions, 0, calculateTime); // partial
    }
    outRes << "----------------------------------------------" << endl;

    return 0;
}