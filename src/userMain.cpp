#include <iostream>
#include "normalNode.h"
#include "upperModel.h"
#include "gappedNode.h"
#include "segModelWithSplit.h"
#include <algorithm>
#include <random>
#include <iomanip>
int datasetSize = 10000;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;
model modelName;
btree::btree_map<double, double> btreemap;
upperModel<normalNode> rmi;
upperModel<gappedNode> ga;
segModelWithSplit seg;
enum model
{
    LEARNED_INDEX,
    GAPPED_ARRAY,
    SEG_MODEL,
    B_TREE
};
void generateDataset()
{
    float maxValue = 10000.00;

    // create dataset randomly
    std::default_random_engine generator;
    std::lognormal_distribution<double> distribution(0.0, 2.0);
    vector<double> ds;

    for (int i = 0; i < datasetSize; i++)
    {
        ds.push_back(distribution(generator));
    }
    std::sort(ds.begin(), ds.end());

    double maxV = ds[ds.size() - 1];
    double factor = maxValue / maxV;
    for (int i = 0; i < ds.size(); i++)
    {
        if (i % 10 != 0)
        {
            dataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
            btreemap.insert({double(ds[i] * factor), double(ds[i] * factor) * 10});
        }
        else
            insertDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
    }
    datasetSize = dataset.size();
}
void createModel()
{
    params firstStageParams(0.01, 1000, 8);
    params secondStageParams(0.01, 1000, 8);

    rmi = upperModel<normalNode>(dataset, firstStageParams, secondStageParams, 1024, 128, 200);
    rmi.train();
    cout << "rmi model over! " << endl;

    ga = upperModel<gappedNode>(dataset, firstStageParams, secondStageParams, 600, 128, 400);
    ga.train();
    cout << "ga model over! " << endl;

    seg = segModelWithSplit(dataset);
    seg.preProcess();
    cout << "seg model over! " << endl;
}
pair<double, double> find(int num, double key)
{
    switch (num)
    {
    case 0:
        return rmi.find(key);
    case 1:
        return ga.find(key);
    case 2:
        return seg.find(key);
    case 3:
        auto res = btreemap.find(key);
        return {res->first, res->second};
    default:
        break;
    }
}
bool del(int num, double key)
{
    switch (num)
    {
    case 0:
        return rmi.del(key);
    case 1:
        return ga.del(key);
    case 2:
        return seg.del(key);
    case 3:
        auto res = btreemap.erase(key);
        return true;
    default:
        break;
    }
}
bool update(int num, pair<double, double> data)
{
    switch (num)
    {
    case 0:
        return rmi.update(data);
    case 1:
        return ga.update(data);
    case 2:
        return seg.update(data.first, data.second);
    case 3:
        auto res = btreemap.find(data.first);
        return true;
    default:
        break;
    }
}
bool insert(int num, pair<double, double> data)
{
    switch (num)
    {
    case 0:
        return rmi.insert(data);
    case 1:
        return ga.insert(data);
    case 2:
        return seg.insert(data.first, data.second);
    case 3:
        auto res = btreemap.insert(data);
        return true;
    default:
        break;
    }
}
int main()
{
    generateDataset();
    createModel();
    insert(B_TREE, {1.11, 1.11});
    find(LEARNED_INDEX, dataset[0].first);
    update(GAPPED_ARRAY, {dataset[1].first, 2.22});
    del(LEARNED_INDEX, dataset[3].first);
    
    return 0;
}
