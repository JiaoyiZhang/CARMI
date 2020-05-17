#include "head.h"

int datasetSize = 10000;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

int kLeafNodeID = 1;
int kInnerNodeID = 3;
int kNeuronNumber = 8;

int childNum = 15;

LognormalDataset logData = LognormalDataset(datasetSize, 0.9);

TEST(TestFind, Find)
{
    logData.GenerateDataset(dataset, insertDataset);
    BinarySearchNode *node = new BinarySearchNode(childNum);
    node->Initialize(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        auto res = node->Find(dataset[i].first);
        EXPECT_EQ(dataset[i], res);
    }
}

TEST(TestInsert, Insert)
{
    BinarySearchNode *node = new BinarySearchNode(childNum);
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
    BinarySearchNode *node = new BinarySearchNode(childNum);
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
    BinarySearchNode *node = new BinarySearchNode(childNum);
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
    AdaptiveBin *node = new AdaptiveBin(childNum);
    node->Initialize(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        auto res = node->Find(dataset[i].first);
        EXPECT_EQ(dataset[i], res);
    }
}

TEST(TestAdaptiveInsert, AdaptiveInsert)
{
    AdaptiveBin *node = new AdaptiveBin(childNum);
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
    AdaptiveBin *node = new AdaptiveBin(childNum);
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
    AdaptiveBin *node = new AdaptiveBin(childNum);
    node->Initialize(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        node->Update({dataset[i].first, 1.11});
        auto res = node->Find(dataset[i].first);
        EXPECT_EQ(1.11, res.second);
    }
}