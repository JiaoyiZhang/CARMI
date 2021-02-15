#ifndef STATIC_H
#define STATIC_H

#include "../func/function.h"

#include "../workload/workloada.h"
#include "../workload/workloadb.h"
#include "../workload/workloadc.h"
#include "../workload/workloadd.h"
#include "../workload/workloade.h"

using namespace std;

extern vector<BaseNode> entireChild;
extern int childNum;
extern ofstream outRes;

extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;

void RunStatic(int initRatio)
{
    findActualDataset = dataset;
    insertActualDataset = insertDataset;
    for (int j = 0; j < 1; j++)
    {
        kInnerNodeID = j;
        cout << "childNum is: " << childNum << endl;
        cout << "root type:" << kInnerNodeID << endl;
        initEntireData(0, dataset.size() * 1.1, false);
        initEntireChild(dataset.size() * 1.1);
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

        if (initRatio == 0.5)
            WorkloadA(kInnerNodeID); // write-heavy
        else if (initRatio == 0.95)
            WorkloadB(kInnerNodeID); // read-mostly
        else if (initRatio == 1)
            WorkloadC(kInnerNodeID); // read-only
        else if (initRatio == 0)
            WorkloadD(kInnerNodeID); // write-partially
        else if (initRatio == 2)
            WorkloadE(kInnerNodeID); // range scan

        // chrono::_V2::system_clock::time_point s, e;
        // double tmp;

        // unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        // shuffle(dataset.begin(), dataset.end(), default_random_engine(seed));

        // s = chrono::system_clock::now();
        // for (int i = 0; i < dataset.size(); i++)
        //     Find(kInnerNodeID, dataset[i].first);
        // e = chrono::system_clock::now();
        // double temp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den * 1000000000;
        // cout << "Find time:" << temp / (float)dataset.size() << endl;

        // auto entropy = GetEntropy(dataset.size());
        // auto entropy = 2;
        // cout << "Entropy:" << entropy << endl;
        // outRes << "ENTROPY," << entropy << ",";
        // cout << "time / entropy: " << tmp / (float)dataset.size() / entropy << endl;
        // outRes << "ratio," << tmp / (float)dataset.size() / entropy << ",";

        // s = chrono::system_clock::now();
        // for (int i = 0; i < insertDataset.size(); i++)
        //     Insert(kInnerNodeID, insertDataset[i]);
        // e = chrono::system_clock::now();
        // tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den * 1000000000;
        // cout << "Insert time:" << tmp / (float)insertDataset.size() << endl;
        // outRes << tmp / (float)insertDataset.size() << ",";
        // cout << "-------------------------------" << endl;

        // std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        //     return p1.first < p2.first;
        // });
    }
    outRes << endl;
}

void TestStatic()
{
    for (int j = 0; j < 4; j++)
    {
        kInnerNodeID = j;
        cout << "childNum is: " << childNum << endl;
        cout << "root type:" << kInnerNodeID << endl;
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
        double tmp;
        for (int i = 0; i < dataset.size(); i++)
        {
            auto res = Find(kInnerNodeID, dataset[i].first);
            if (res.second != dataset[i].second)
                cout << "Find failed:\ti:" << i << "\tdata:" << dataset[i].first << "\t" << dataset[i].second << "\tres: " << res.first << "\t" << res.second << endl;
        }
        cout << "check FIND over!" << endl;

        for (int i = 0; i < insertDataset.size(); i++)
        {
            auto r = Insert(kInnerNodeID, insertDataset[i]);
            if (!r)
                cout << "Insert failed:\ti:" << i << "\t" << insertDataset[i].first << endl;
            auto res = Find(kInnerNodeID, insertDataset[i].first);
            if (res.second != insertDataset[i].second)
                cout << "after insert, Find failed:\ti:" << i << "\t" << insertDataset[i].first << "\tres: " << res.first << "\t" << res.second << endl;
        }
        for (int i = 0; i < insertDataset.size(); i++)
        {
            auto res = Find(kInnerNodeID, insertDataset[i].first);
            if (res.second != insertDataset[i].second)
                cout << "Find Insert failed:\ti:" << i << "\t" << insertDataset[i].first << "\tres: " << res.first << "\t" << res.second << endl;
        }
        cout << "check INSERT over!" << endl;

        for (int i = 0; i < insertDataset.size(); i++)
        {
            auto r = Update(kInnerNodeID, {insertDataset[i].first, 1.11});
            if (!r)
                cout << "Update failed:\ti:" << i << "\t" << insertDataset[i].first << endl;
            auto res = Find(kInnerNodeID, insertDataset[i].first);
            if (res.second != 1.11)
                cout << "After Update failed:\ti:" << i << "\t" << insertDataset[i].first << "\tres: " << res.first << "\t" << res.second << endl;
        }
        for (int i = 0; i < insertDataset.size(); i++)
        {
            auto res = Find(kInnerNodeID, insertDataset[i].first);
            if (res.second != 1.11)
                cout << "Find Update failed:\ti:" << i << "\t" << insertDataset[i].first << "\tres: " << res.first << "\t" << res.second << endl;
        }
        cout << "check UPDATE over!" << endl;

        for (int i = 0; i < insertDataset.size(); i++)
        {
            auto r = Delete(kInnerNodeID, insertDataset[i].first);
            if (!r)
                cout << "Delete failed:\ti:" << i << "\t" << insertDataset[i].first << endl;
            auto res = Find(kInnerNodeID, insertDataset[i].first);
            if ((res.second == insertDataset[i].second) || (res.second == 1.11))
                cout << "After Delete failed:\ti:" << i << "\t" << insertDataset[i].first << "\tres: " << res.first << "\t" << res.second << endl;
        }
        for (int i = 0; i < insertDataset.size(); i++)
        {
            auto res = Find(kInnerNodeID, insertDataset[i].first);
            if ((res.second == insertDataset[i].second) || (res.second == 1.11))
                cout << "Find Delete failed:\ti:" << i << "\t" << insertDataset[i].first << "\tres: " << res.first << "\t" << res.second << endl;
        }
        cout << "check DELETE over!" << endl;
        cout << "-------------------------------" << endl;
    }

    outRes << endl;
}

#endif // !STATIC_H
