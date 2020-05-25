#include "../head.h"

int datasetSize = 10000;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

int kLeafNodeID = 1;
int kInnerNodeID = 3;
int kNeuronNumber = 8;

int childNum = 25;
int kThreshold = 1000;
int kMaxKeyNum = 200000;

LognormalDataset logData = LognormalDataset(datasetSize, 0.9);

TEST(TestInit, Init)
{
    logData.GenerateDataset(dataset, insertDataset);
    LRNode *node = new LRNode(childNum);
    node->Initialize(dataset);
    EXPECT_EQ(childNum, node->GetChildNum());
}

TEST(TestFind, Find)
{
    BinarySearchNode *node = new BinarySearchNode(childNum);
    node->Initialize(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        auto res = node->Find(dataset[i].first);
        EXPECT_EQ(dataset[i], res)<<"find wrong! i:"<<i<<endl;
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
        EXPECT_EQ(insertDataset[i], res)<<"insert wrong! i:"<<i<<endl;
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
        EXPECT_EQ(DBL_MIN, res.second)<<"delete wrong! i:"<<i<<endl;
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
        EXPECT_EQ(1.11, res.second)<<"update wrong! i:"<<i<<endl;
    }
}


TEST(TestAdaptiveInit, AdaptiveInit)
{
    AdaptiveBin *node = new AdaptiveBin(childNum);
    node->Initialize(dataset);
    EXPECT_EQ(childNum, node->GetChildNum());
}

TEST(TestAdaptiveFind, AdaptiveFind)
{
    AdaptiveBin *node = new AdaptiveBin(childNum);
    node->Initialize(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        auto res = node->Find(dataset[i].first);
        EXPECT_EQ(dataset[i], res)<<"find wrong! i:"<<i<<endl;
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
        EXPECT_EQ(insertDataset[i], res)<<"insert wrong! i:"<<i<<endl;
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
        EXPECT_EQ(DBL_MIN, res.second)<<"delete wrong! i:"<<i<<endl;
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
        EXPECT_EQ(1.11, res.second)<<"update wrong! i:"<<i<<endl;
    }
}