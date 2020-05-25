#include "head.h"

int datasetSize = 10000;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

int kLeafNodeID = 1;
int kInnerNodeID = 0;
int kNeuronNumber = 8;

int childNum = 25;
int kThreshold = 1000;
int kMaxKeyNum = 200000;

LognormalDataset logData = LognormalDataset(datasetSize, 0.9);
vector<pair<double, double>> totalData;
btree::btree_map<double, pair<int, int>> cntTree;

TEST(TestCalculateCost, CalculateCost)
{
    logData.GenerateDataset(dataset, insertDataset);
    vector<pair<int, int>> cnt;
    for (int i = 0; i < dataset.size(); i++)
        totalData.push_back(dataset[i]);
    for (int i = 0; i < insertDataset.size(); i++)
        totalData.push_back(insertDataset[i]);
    std::sort(totalData.begin(), totalData.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });
    for (int i = 0; i < datasetSize; i++)
    {
        if ((i + 1) % 10 == 0)
            cnt.push_back({0, 1});
        else
            cnt.push_back({2, 0});
    }
    for (int i = 0; i < cnt.size(); i++)
        cntTree.insert({dataset[i].first, cnt[i]}); // <key, <read, write>>
    long double minCost = 1e100;
    int minNum = 0;
    int maxChildNum = dataset.size() / 50;
    long double tmpCost;
    for (int i = 2; i < maxChildNum; i++)
    {
        tmpCost = CalculateCost(cntTree, i, dataset);
        EXPECT_GE(1E100, tmpCost)<<"calculate childNum :" << i << "tmpCost is: " << tmpCost << "    minCost: " << minCost << "    minNum: " << minNum << endl;
        if (tmpCost < minCost)
        {
            minCost = tmpCost;
            minNum = i;
        }
    }
}

TEST(TestArray, Array)
{
    long double tmpCost = ArrayNode::GetCost(cntTree, dataset);
    EXPECT_GE(1e100, tmpCost);
}
TEST(TestGappedArray, GappedArray)
{
    long double tmpCost = GappedArray::GetCost(cntTree, dataset);
    EXPECT_GE(1e100, tmpCost);
}


TEST(TestLR, LR)
{
    long double tmpCost = LRNode::GetCost(cntTree, 25, dataset);
    EXPECT_GE(1e100, tmpCost);
}
TEST(TestAdaptiveLR, AdaptiveLR)
{
    long double tmpCost = AdaptiveLR::GetCost(cntTree, 25, dataset);
    EXPECT_GE(1e100, tmpCost);
}

TEST(TestNN, NN)
{
    long double tmpCost = NetworkNode::GetCost(cntTree, 25, dataset);
    EXPECT_GE(1e100, tmpCost);
}
TEST(TestAdaptiveNN, AdaptiveNN)
{
    long double tmpCost = AdaptiveNN::GetCost(cntTree, 25, dataset);
    EXPECT_GE(1e100, tmpCost);
}

TEST(TestBS, BS)
{
    long double tmpCost = BinarySearchNode::GetCost(cntTree, 25, dataset);
    EXPECT_GE(1e100, tmpCost);
}
TEST(TestAdaptiveBin, AdaptiveBin)
{
    long double tmpCost = AdaptiveBin::GetCost(cntTree, 25, dataset);
    EXPECT_GE(1e100, tmpCost);
}

TEST(TestHistogramNode, HistogramNode)
{
    long double tmpCost = HistogramNode::GetCost(cntTree, 25, dataset);
    EXPECT_GE(1e100, tmpCost);
}
TEST(TestAdaptiveHis, AdaptiveHis)
{
    long double tmpCost = AdaptiveHis::GetCost(cntTree, 25, dataset);
    EXPECT_GE(1e100, tmpCost);
}