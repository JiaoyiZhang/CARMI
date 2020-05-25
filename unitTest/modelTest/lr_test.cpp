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
    LinearRegression *model = new LinearRegression();
    model->Train(dataset);
    EXPECT_GT(10000, model->GetTheta1());
    EXPECT_GT(10000, model->GetTheta2());
    EXPECT_LT(-10000, model->GetTheta1());
    EXPECT_LT(-10000, model->GetTheta2());
    cout<<"Theta1 :"<<model->GetTheta1()<<endl;
    cout<<"Theta2 :"<<model->GetTheta2()<<endl;
}

TEST(TestPredict, Predict)
{
    LinearRegression *model = new LinearRegression();
    model->Train(dataset);
    double theta1 = model->GetTheta1();
    double theta2 = model->GetTheta2();
    for(int i=0;i<dataset.size();i++)
    {
        double p = theta1 * dataset[i].first + theta2;
        p = p < 0 ? 0 : p;
        p = p > 1 ? 1 : p;
        EXPECT_EQ(p, model->Predict(dataset[i].first));
    }
}