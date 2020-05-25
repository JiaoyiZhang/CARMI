#include "../head.h"

int datasetSize = 1000000;
int maxNum = 1000001;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

int kLeafNodeID = 1;
int kInnerNodeID = 0;
int kNeuronNumber = 8;

int childNum = 25;
int kThreshold = 1000;
int kMaxKeyNum = 200000;

LognormalDataset logData = LognormalDataset(datasetSize, 0.9);

TEST(TestGappedArraySetDataset, SetGappedArrayDataset)
{
    logData.GenerateDataset(dataset, insertDataset);
    GappedArray *ga = new GappedArray(maxNum);
    ga->SetDataset(dataset);
    EXPECT_EQ(dataset.size(), ((BasicLeafNode *)ga)->GetSize());
    EXPECT_LE(-dataset.size(), ((BasicLeafNode *)ga)->GetNegativeError());
    EXPECT_GE(dataset.size(), ((BasicLeafNode *)ga)->GetPositiveError());
}

TEST(TESTGappedArrayBinarySearch, GappedArrayBinarySearch)
{
    GappedArray *ga = new GappedArray(maxNum);
    ga->SetDataset(dataset);
    int positve = ((BasicLeafNode *)ga)->GetPositiveError();
    int negative = ((BasicLeafNode *)ga)->GetNegativeError();
    vector<pair<double, double>> subDataset;
    ((BasicLeafNode *)ga)->GetTotalDataset(&subDataset);
    int maxIndex = ga->GetMaxIndex();
    for (int i = 0; i < dataset.size(); i++)
    {
        int preIdx = ga->GetPredictIndex(dataset[i].first);
        if (subDataset[preIdx].first != dataset[i].first)
        {
            int start = max(0, preIdx + negative);
            int end = min(maxIndex, preIdx + positve);
            int res = ga->BinarySearch(dataset[i].first, preIdx, start, end);
            EXPECT_EQ(dataset[i].first, subDataset[res].first)<<"find wrong!"<<endl;
        }
    }
    for (int i = 0; i < insertDataset.size(); i++)
    {
        int preIdx = ga->GetPredictIndex(insertDataset[i].first);
        int start = max(0, preIdx + negative);
        int end = min(maxIndex, preIdx + positve);
        preIdx = ga->BinarySearch(insertDataset[i].first, preIdx, start, end);
        if(preIdx <= start)
        {
            EXPECT_LE(insertDataset[i].first, subDataset[preIdx].first) <<"less than start wrong! i:"<<i<<"\tpreIdx:"<<preIdx<<endl;
            preIdx = ga->BinarySearch(insertDataset[i].first, preIdx, 0, start);
        }
        else if(preIdx >= end)
        {
            if(preIdx != maxIndex && subDataset[preIdx].first!=-1)
                EXPECT_LE(insertDataset[i].first, subDataset[preIdx].first)<<"greater than end wrong! i:"<<i<<"\tpreIdx:"<<preIdx<<endl;
            preIdx = ga->BinarySearch(insertDataset[i].first, preIdx, preIdx, maxIndex);
        }
        if(preIdx != maxIndex  && subDataset[preIdx].first!=-1)
            EXPECT_LE(insertDataset[i].first, subDataset[preIdx].first)<<"LE wrong! i:"<<i<<"\tpreIdx:"<<preIdx<<endl;
        if(preIdx != 0 && subDataset[preIdx-1].first!=-1)
            EXPECT_GT(insertDataset[i].first, subDataset[preIdx-1].first)<<"GT wrong! i:"<<i<<"\tpreIdx:"<<preIdx<<endl;
    }
}

TEST(TESTGappedArrayExponentialSearch, GappedArrayExponentialSearch)
{
    GappedArray *ga = new GappedArray(maxNum);
    ga->SetDataset(dataset);
    int positve = ((BasicLeafNode *)ga)->GetPositiveError();
    int negative = ((BasicLeafNode *)ga)->GetNegativeError();
    vector<pair<double, double>> subDataset;
    ((BasicLeafNode *)ga)->GetTotalDataset(&subDataset);
    int maxIndex = ga->GetMaxIndex();
    for (int i = 0; i < dataset.size(); i++)
    {
        int preIdx = ga->GetPredictIndex(dataset[i].first);
        if (subDataset[preIdx].first != dataset[i].first)
        {
            int start = max(0, preIdx + negative);
            int end = min(maxIndex, preIdx + positve);
            int res = ga->ExponentialSearch(dataset[i].first, preIdx, start, end);
            EXPECT_EQ(dataset[i].first, subDataset[res].first)<<"find wrong!"<<endl;
        }
    }
    for (int i = 0; i < insertDataset.size(); i++)
    {
        int preIdx = ga->GetPredictIndex(insertDataset[i].first);
        int start = max(0, preIdx + negative);
        int end = min(maxIndex, preIdx + positve);
        preIdx = ga->ExponentialSearch(insertDataset[i].first, preIdx, start, end);
        if(preIdx <= start)
        {
            EXPECT_LE(insertDataset[i].first, subDataset[preIdx].first) <<"less than start wrong! i:"<<i<<"\tpreIdx:"<<preIdx<<endl;
            preIdx = ga->ExponentialSearch(insertDataset[i].first, preIdx, 0, start);
        }
        else if(preIdx >= end)
        {
            if(preIdx != maxIndex && subDataset[preIdx].first!=-1)
                EXPECT_LE(insertDataset[i].first, subDataset[preIdx].first)<<"greater than end wrong! i:"<<i<<"\tpreIdx:"<<preIdx<<endl;
            preIdx = ga->ExponentialSearch(insertDataset[i].first, preIdx, preIdx, maxIndex);
        }
        if(preIdx != maxIndex && subDataset[preIdx].first!=-1)
            EXPECT_LE(insertDataset[i].first, subDataset[preIdx].first)<<"LE wrong! i:"<<i <<"\tpreIdx:"<<preIdx<<endl;
        if(preIdx != 0 && subDataset[preIdx-1].first!=-1)
            EXPECT_GE(insertDataset[i].first, subDataset[preIdx-1].first)<<"GE wrong! i:"<<i<<"\tpreIdx:"<<preIdx<<endl;
    }
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