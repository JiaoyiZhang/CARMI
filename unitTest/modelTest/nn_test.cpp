#include "../head.h"

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


TEST(TestTrain, Train)
{
    logData.GenerateDataset(dataset, insertDataset);
    Net *model = new Net();
    model->Train(dataset);
    vector<double> W1,W2,b1;
    double b2;
    model->GetW1(W1);
    model->GetW2(W2);
    model->Getb1(b1);
    b2 = model->Getb2();
    EXPECT_EQ(kNeuronNumber, W1.size());
    EXPECT_EQ(kNeuronNumber, W2.size());
    EXPECT_EQ(kNeuronNumber, b1.size());
    cout<<"W1 :"<<endl;
    for(int i=0;i<kNeuronNumber;i++)
        cout<<W1[i]<<"    ";
    cout<<endl;

    cout<<"W2 :"<<endl;
    for(int i=0;i<kNeuronNumber;i++)
        cout<<W2[i]<<"    ";
    cout<<endl;

    cout<<"b1 :"<<endl;
    for(int i=0;i<kNeuronNumber;i++)
        cout<<b1[i]<<"    ";
    cout<<endl;

    cout<<"b2 :"<<b2<<endl;
}

TEST(TestPredict, Predict)
{
    Net *model = new Net();
    model->Train(dataset);
    vector<double> W1,W2,b1;
    double b2;
    model->GetW1(W1);
    model->GetW2(W2);
    model->Getb1(b1);
    b2 = model->Getb2();
    for(int i=0;i<dataset.size();i++)
    {
        double p = b2;
        for (int i = 0; i < W1.size(); i++)
        {
            p += max(double(0), dataset[i].first *W1[i] + b1[i]) * W2[i];
        }
        if (p < 0)
            p = 0;
        else if (p > 1)
            p = 1;
        double diff = p - model->Predict(dataset[i].first);
        if(diff < 0)
            diff = -diff;
        EXPECT_GT(0.000001, diff);
    }
}