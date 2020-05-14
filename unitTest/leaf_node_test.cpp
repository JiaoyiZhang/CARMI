#include "head.h"

int datasetSize = 10000;
int maxNum = 10001;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

int kLeafNodeID = 1;
int kInnerNodeID = 0;
int kNeuronNumber = 8;

LognormalDataset logData = LognormalDataset(datasetSize, 0.9);

TEST(TestArraySetDataset, SetArrayDataset)
{
    logData.GenerateDataset(dataset, insertDataset);
    ArrayNode *arr = new ArrayNode(maxNum);
    arr->SetDataset(dataset);
    EXPECT_EQ(dataset.size(), ((BasicLeafNode *)arr)->GetSize());
}

TEST(TestArrayFind, ArrayFind)
{
    ArrayNode *arr = new ArrayNode(maxNum);
    arr->SetDataset(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        auto res = arr->Find(dataset[i].first);
        EXPECT_EQ(dataset[i], res);
    }
}

TEST(TestArrayInsert, ArrayInsert)
{
    ArrayNode *arr = new ArrayNode(maxNum);
    arr->SetDataset(dataset);
    for (int i = 0; i < insertDataset.size(); i++)
    {
        arr->Insert(insertDataset[i]);
        auto res = arr->Find(insertDataset[i].first);
        EXPECT_EQ(insertDataset[i], res);
    }
}

TEST(TestArrayDelete, ArrayDelete)
{
    ArrayNode *arr = new ArrayNode(maxNum);
    arr->SetDataset(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        arr->Delete(dataset[i].first);
        auto res = arr->Find(dataset[i].first);
        EXPECT_EQ(0, res.second);
    }
}

TEST(TestArrayUpdate, ArrayUpdate)
{
    ArrayNode *arr = new ArrayNode(maxNum);
    arr->SetDataset(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        arr->Update({dataset[i].first, 1.11});
        auto res = arr->Find(dataset[i].first);
        EXPECT_EQ(1.11, res.second);
    }
}


TEST(TestGappedArraySetDataset, SetGappedArrayDataset)
{
    GappedArray *ga = new GappedArray(maxNum);
    ga->SetDataset(dataset);
    EXPECT_EQ(dataset.size(), ((BasicLeafNode *)ga)->GetSize());
}

TEST(TestGappedArrayFind, GappedArrayFind)
{
    GappedArray *ga = new GappedArray(maxNum);
    ga->SetDataset(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        auto res = ga->Find(dataset[i].first);
        EXPECT_EQ(dataset[i], res);
    }
}

TEST(TestGappedArrayInsert, GappedArrayInsert)
{
    GappedArray *ga = new GappedArray(maxNum);
    ga->SetDataset(dataset);
    for (int i = 0; i < insertDataset.size(); i++)
    {
        ga->Insert(insertDataset[i]);
        auto res = ga->Find(insertDataset[i].first);
        EXPECT_EQ(insertDataset[i], res);
    }
}

TEST(TestGappedArrayDelete, GappedArrayDelete)
{
    GappedArray *ga = new GappedArray(maxNum);
    ga->SetDataset(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        ga->Delete(dataset[i].first);
        auto res = ga->Find(dataset[i].first);
        EXPECT_EQ(DBL_MIN, res.second);
    }
}

TEST(TestGappedArrayUpdate, GappedArrayUpdate)
{
    GappedArray *ga = new GappedArray(maxNum);
    ga->SetDataset(dataset);
    for (int i = 0; i < dataset.size(); i++)
    {
        ga->Update({dataset[i].first, 1.11});
        auto res = ga->Find(dataset[i].first);
        EXPECT_EQ(1.11, res.second);
    }
}