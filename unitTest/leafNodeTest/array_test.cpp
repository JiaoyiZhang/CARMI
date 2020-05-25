#include "../head.h"

int datasetSize = 10000;
int maxNum = 10001;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

int kLeafNodeID = 1;
int kInnerNodeID = 0;
int kNeuronNumber = 8;

int childNum = 25;
int kThreshold = 1000;
int kMaxKeyNum = 200000;

LognormalDataset logData = LognormalDataset(datasetSize, 0.9);

TEST(TestArraySetDataset, SetArrayDataset)
{
    logData.GenerateDataset(dataset, insertDataset);
    ArrayNode *arr = new ArrayNode(maxNum);
    arr->SetDataset(dataset);
    EXPECT_EQ(dataset.size(), ((BasicLeafNode *)arr)->GetSize());
    EXPECT_LE(-dataset.size(), ((BasicLeafNode *)arr)->GetNegativeError());
    EXPECT_GE(dataset.size(), ((BasicLeafNode *)arr)->GetPositiveError());
}

TEST(TESTArrayBinarySearch, ArrayBinarySearch)
{
    ArrayNode *arr = new ArrayNode(maxNum);
    arr->SetDataset(dataset);
    int positve = ((BasicLeafNode *)arr)->GetPositiveError();
    int negative = ((BasicLeafNode *)arr)->GetNegativeError();
    vector<pair<double, double>> subDataset;
    ((BasicLeafNode *)arr)->GetDataset(&subDataset);
    int m_datasetSize = ((BasicLeafNode *)arr)->GetSize();
    for (int i = 0; i < dataset.size(); i++)
    {
        int preIdx = arr->GetPredictIndex(dataset[i].first);
        if (subDataset[preIdx].first != dataset[i].first)
        {
            int start = max(0, preIdx + negative);
            int end = min(m_datasetSize - 1, preIdx + positve);
            int res = arr->BinarySearch(dataset[i].first, preIdx, start, end);
            EXPECT_EQ(dataset[i].first, subDataset[res].first)<<"find wrong!"<<endl;
        }
    }
    for (int i = 0; i < insertDataset.size(); i++)
    {
        int preIdx = arr->GetPredictIndex(insertDataset[i].first);
        int start = max(0, preIdx + negative);
        int end = min(m_datasetSize - 1, preIdx + positve);
        int res = arr->BinarySearch(insertDataset[i].first, preIdx, start, end);
        if(res <= start)
        {
            EXPECT_LE(insertDataset[i].first, subDataset[res].first) <<"less than start wrong! i:"<<i<<"\tres:"<<res<<endl;
            res = arr->BinarySearch(insertDataset[i].first, preIdx, 0, start);
        }
        else if(res >= end)
        {
            if(res != m_datasetSize - 1)
                EXPECT_LE(insertDataset[i].first, subDataset[res].first)<<"greater than end wrong! i:"<<i<<"\tres:"<<res<<endl;
            res = arr->BinarySearch(insertDataset[i].first, res, res, m_datasetSize - 1);
        }
        if(res != m_datasetSize - 1)
            EXPECT_LE(insertDataset[i].first, subDataset[res].first)<<"LE wrong! i:"<<i<<"\tres:"<<res<<endl;
        if(res != 0)
            EXPECT_GT(insertDataset[i].first, subDataset[res-1].first)<<"GT wrong! i:"<<i<<"\tres:"<<res<<endl;
    }
}

TEST(TESTArrayExponentialSearch, ArrayExponentialSearch)
{
    ArrayNode *arr = new ArrayNode(maxNum);
    arr->SetDataset(dataset);
    int positve = ((BasicLeafNode *)arr)->GetPositiveError();
    int negative = ((BasicLeafNode *)arr)->GetNegativeError();
    vector<pair<double, double>> subDataset;
    ((BasicLeafNode *)arr)->GetDataset(&subDataset);
    int m_datasetSize = ((BasicLeafNode *)arr)->GetSize();
    for (int i = 0; i < dataset.size(); i++)
    {
        int preIdx = arr->GetPredictIndex(dataset[i].first);
        if (subDataset[preIdx].first != dataset[i].first)
        {
            int start = max(0, preIdx + negative);
            int end = min(m_datasetSize - 1, preIdx + positve);
            int res = arr->BinarySearch(dataset[i].first, preIdx, start, end);
            EXPECT_EQ(dataset[i].first, subDataset[res].first)<<"find wrong!"<<endl;
        }
    }
    for (int i = 0; i < insertDataset.size(); i++)
    {
        int preIdx = arr->GetPredictIndex(insertDataset[i].first);
        int start = max(0, preIdx + negative);
        int end = min(m_datasetSize - 1, preIdx + positve);
        int res = arr->ExponentialSearch(insertDataset[i].first, preIdx, start, end);
        if(res <= start)
        {
            EXPECT_LE(insertDataset[i].first, subDataset[res].first) <<"less than start wrong! i:"<<i<<"\tres:"<<res<<endl;
            res = arr->ExponentialSearch(insertDataset[i].first, preIdx, 0, start);
        }
        else if(res >= end)
        {
            if(res != m_datasetSize - 1)
                EXPECT_LE(insertDataset[i].first, subDataset[res].first)<<"greater than end wrong! i:"<<i<<"\tres:"<<res<<endl;
            res = arr->ExponentialSearch(insertDataset[i].first, res, res, m_datasetSize - 1);
        }
        if(res != m_datasetSize - 1)
            EXPECT_LE(insertDataset[i].first, subDataset[res].first)<<"LE wrong! i:"<<i<<"\tres:"<<res<<endl;
        if(res != 0)
            EXPECT_GT(insertDataset[i].first, subDataset[res-1].first)<<"GT wrong! i:"<<i<<"\tres:"<<res<<endl;
    }
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
    for (int i = 0; i < insertDataset.size(); i++)
    {
        arr->Insert(insertDataset[i]);
    }
    for (int i = 0; i < insertDataset.size(); i++)
    {
        arr->Delete(insertDataset[i].first);
        auto res = arr->Find(insertDataset[i].first);
        EXPECT_EQ(0, res.second);
    }
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
    for (int i = 0; i < insertDataset.size(); i++)
        arr->Insert(insertDataset[i]);
    for (int i = 0; i < insertDataset.size(); i++)
    {
        arr->Update({insertDataset[i].first, 1.11});
        auto res = arr->Find(insertDataset[i].first);
        EXPECT_EQ(1.11, res.second);
    }
    for (int i = 0; i < dataset.size(); i++)
    {
        arr->Update({dataset[i].first, 1.11});
        auto res = arr->Find(dataset[i].first);
        EXPECT_EQ(1.11, res.second);
    }
}
