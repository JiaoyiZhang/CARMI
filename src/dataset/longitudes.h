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

class LongitudesDataset
{
public:
	LongitudesDataset(double initRatio)
	{
		num = initRatio / (1 - initRatio);
	}

	void GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset);

private:
	int num;
};

void LongitudesDataset::GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset)
{
	initDataset.clear();
	insertDataset.clear();
	vector<pair<double, double>> ds;
	ifstream inFile("../src/dataset/iranLon.csv", ios::in);
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
		ds.push_back({ k, v });
	}

	std::sort(ds.begin(), ds.end());
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
	cout << "Read size:" << initDataset.size() << "\tWrite size:" << insertDataset.size() << endl;
}

#endif