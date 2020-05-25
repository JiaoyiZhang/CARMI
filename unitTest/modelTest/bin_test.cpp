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
    BinarySearchModel *model = new BinarySearchModel(childNum);
    model->Train(dataset);
    vector<double> index;
    model->GetIndex(index);
    int num = model->GetChildNum();
    EXPECT_EQ(childNum, num);
    cout<<"index :"<<endl;
    for(int i=0;i<index.size();i++)
        cout<<index[i]<<"    ";
    cout<<endl;
}

TEST(TestPredict, Predict)
{
    BinarySearchModel *model = new BinarySearchModel(childNum);
    model->Train(dataset);
    vector<double> index;
    model->GetIndex(index);
    int num = model->GetChildNum();
    for(int i=0;i<dataset.size();i++)
    {
        int start_idx = 0;
        int end_idx = index.size() - 1;
        int mid;
        while (start_idx < end_idx)
        {
            mid = (start_idx + end_idx) / 2;
            if (index[mid] < dataset[i].first)
                start_idx = mid + 1;
            else
                end_idx = mid;
        }
        double p = float(mid) / index.size();
        p = p < 0 ? 0 : p;
        p = p > 1 ? 1 : p;
        EXPECT_EQ(p, model->Predict(dataset[i].first));
    }
}