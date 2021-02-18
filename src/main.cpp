
#include <algorithm>
#include <random>
#include <iostream>
#include <algorithm>
#include <fstream>

#include "./experiment/fixed_experiment.h"
#include "./experiment/main_experiment.h"
using namespace std;

int kLeafNodeID = 0;
int kInnerNodeID = 0;

ofstream outRes;

int main()
{
    outRes.open("res_0217.csv", ios::app);

    time_t timep;
    time(&timep);
    char tmpTime[64];
    strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    cout << "\nTest time: " << tmpTime << endl;

    int datasetSize = 67108864;

    mainExperiment(datasetSize);
    fixedExperiment(datasetSize);

    outRes << "----------------------------------------------" << endl;

    return 0;
}