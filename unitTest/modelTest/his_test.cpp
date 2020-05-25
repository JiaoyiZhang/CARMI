#include "../head.h"

int datasetSize = 1000000;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

int kLeafNodeID = 1;
int kInnerNodeID = 0;
int kNeuronNumber = 8;

int childNum = 25;
int kThreshold = 1000;
int kMaxKeyNum = 200000;

LognormalDataset logData = LognormalDataset(datasetSize, 0.9);


TEST(TestTrain, Train)
{
    logData.GenerateDataset(dataset, insertDataset);
    HistogramModel *model = new HistogramModel(childNum);
    model->Train(dataset);
    vector<double> table;
    model->GetTable(table);
    float value = model->GetValue();
    EXPECT_NE(0, value);
    cout<<"table :"<<endl;
    for(int i=0;i<table.size();i++)
    {
        cout<<table[i]<<"    ";
        if((i+1)%10 == 0)
            cout<<endl;
    }
    cout<<endl;
}

TEST(TestPredict, Predict)
{
    HistogramModel *model = new HistogramModel(childNum);
    model->Train(dataset);
    vector<double> table;
    model->GetTable(table);
    float value = model->GetValue();
    for(int i=0;i<dataset.size();i++)
    {
        int idx = float(dataset[i].first * 99) / (value * childNum);
        double p = table[idx];
        p = p < 0 ? 0 : p;
        p = p > 1 ? 1 : p;
        EXPECT_EQ(p, model->Predict(dataset[i].first))<<"i:"<<i<<"\t key:"<<dataset[i].first<<"\t idx:"<<idx<<endl;
    }
}