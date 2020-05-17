#include "head.h"

int datasetSize = 10000;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

int kLeafNodeID = 1;
int kInnerNodeID = 1;
int kNeuronNumber = 8;

int childNum = 15;

LognormalDataset logData = LognormalDataset(datasetSize, 0.9);

TEST(TestFind, Find)
{
    logData.GenerateDataset(dataset, insertDataset);
    NetworkNode *node = new NetworkNode(childNum);
    node->Initialize(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        auto res = node->Find(dataset[i].first);
        EXPECT_EQ(dataset[i], res);
    }
}

TEST(TestInsert, Insert)
{
    NetworkNode *node = new NetworkNode(childNum);
    node->Initialize(dataset);
    for (int i = 0; i < insertDataset.size(); i++)
    {
        node->Insert(insertDataset[i]);
        auto res = node->Find(insertDataset[i].first);
        EXPECT_EQ(insertDataset[i], res);
    }
}

TEST(TestDelete, Delete)
{
    NetworkNode *node = new NetworkNode(childNum);
    node->Initialize(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        node->Delete(dataset[i].first);
        auto res = node->Find(dataset[i].first);
        EXPECT_EQ(DBL_MIN, res.second);
    }
}

TEST(TestUpdate, Update)
{
    NetworkNode *node = new NetworkNode(childNum);
    node->Initialize(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        node->Update({dataset[i].first, 1.11});
        auto res = node->Find(dataset[i].first);
        EXPECT_EQ(1.11, res.second);
    }
}


TEST(TestAdaptiveFind, AdaptiveFind)
{
    AdaptiveNN *node = new AdaptiveNN(childNum);
    node->Initialize(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        auto res = node->Find(dataset[i].first);
        EXPECT_EQ(dataset[i], res);
    }
}

TEST(TestAdaptiveInsert, AdaptiveInsert)
{
    AdaptiveNN *node = new AdaptiveNN(childNum);
    node->Initialize(dataset);
    for (int i = 0; i < insertDataset.size(); i++)
    {
        node->Insert(insertDataset[i]);
        auto res = node->Find(insertDataset[i].first);
        EXPECT_EQ(insertDataset[i], res);
    }
}

TEST(TestAdaptiveDelete, AdaptiveDelete)
{
    AdaptiveNN *node = new AdaptiveNN(childNum);
    node->Initialize(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        node->Delete(dataset[i].first);
        auto res = node->Find(dataset[i].first);
        EXPECT_EQ(DBL_MIN, res.second);
    }
}

TEST(TestAdaptiveUpdate, AdaptiveUpdate)
{
    AdaptiveNN *node = new AdaptiveNN(childNum);
    node->Initialize(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        node->Update({dataset[i].first, 1.11});
        auto res = node->Find(dataset[i].first);
        EXPECT_EQ(1.11, res.second);
    }
}