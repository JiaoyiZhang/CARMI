#ifndef LONGITUDES_H
#define LONGITUDES_H

#include <algorithm>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
using namespace std;

class LongitudesDataset
{
public:
	LongitudesDataset(double initRatio)
	{
		init = initRatio;
		insertNumber = 100000 * (1 - initRatio);
		if (initRatio == 0)
		{
			num = 0;
			init = 0.85;
			insertNumber = 15000;
		}
		else if (initRatio == 1)
			num = -1;
		else
			num = round(initRatio / (1 - initRatio));
	}

	void GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &trainFindQuery, vector<pair<double, double>> &trainInsertQuery, vector<pair<double, double>> &testInsertQuery);

private:
	int num;
	float init;
	int insertNumber;
};

void LongitudesDataset::GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &trainFindQuery, vector<pair<double, double>> &trainInsertQuery, vector<pair<double, double>> &testInsertQuery)
{
	vector<pair<double, double>>().swap(initDataset);
	vector<pair<double, double>>().swap(trainFindQuery);
	vector<pair<double, double>>().swap(trainInsertQuery);
	vector<pair<double, double>>().swap(testInsertQuery);

	vector<pair<double, double>> insertDataset;

	vector<pair<double, double>> ds;
	ifstream inFile("../src/dataset/longitude.csv", ios::in);
	if (!inFile)
	{
		cout << "打开文件失败！" << endl;
		exit(1);
	}
	string line;
	while (getline(inFile, line))
	{
		if (line.empty())
			continue;
		istringstream sin(line);
		vector<string> fields;
		string field;
		while (getline(sin, field, ','))
			fields.push_back(field);
		string key = fields[0];
		string value = fields[1];
		double k = stod(key);
		double v = stod(value);
		ds.push_back({k, v});
		if (ds.size() == round(67108864.0 / init))
			break;
	}

	std::sort(ds.begin(), ds.end());
	if (num == 0)
	{
		int i = 0;
		for (; i < 0.6 * ds.size(); i++)
			initDataset.push_back(ds[i]);
		for (; i < 0.9 * ds.size(); i += 2)
		{
			initDataset.push_back(ds[i]);
			insertDataset.push_back(ds[i + 1]);
		}
		for (; i < ds.size(); i++)
			initDataset.push_back(ds[i]);
	}
	else if (num == -1)
	{
		for (int i = 0; i < ds.size(); i++)
			initDataset.push_back(ds[i]);
	}
	else
	{
		int cnt = 0;
		for (int i = 0; i < ds.size(); i++)
		{
			cnt++;
			if (cnt <= num)
				initDataset.push_back(ds[i]);
			else
			{
				insertDataset.push_back(ds[i]);
				cnt = 0;
			}
		}
	}
    default_random_engine engine;

    auto find = initDataset;
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    engine = default_random_engine(seed);
    shuffle(find.begin(), find.end(), engine);

    int end = 100000 - insertNumber;
    for (int i = 0; i < end; i++)
        trainFindQuery.push_back(initDataset[i]);

    unsigned seed1 = chrono::system_clock::now().time_since_epoch().count();
    engine = default_random_engine(seed1);
    shuffle(insertDataset.begin(), insertDataset.end(), engine);
    end = insertNumber * 2;
    for (int i = 0; i < end; i += 2)
    {
        trainInsertQuery.push_back(insertDataset[i]);
        testInsertQuery.push_back(insertDataset[i + 1]);
    }

    cout << "longitudes: init size:" << initDataset.size() << "\tFind size:" << trainFindQuery.size() << "\tWrite size:" << testInsertQuery.size() << endl;
}

#endif